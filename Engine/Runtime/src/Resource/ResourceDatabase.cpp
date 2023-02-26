#include "Resource/ResourceDatabase.h"
#include "Resource/ResourceDatabase.inl"
#include "Resource/ResourceManager.h"

#include "Resource/Material.h"
#include "Resource/Mesh.h"
#include "Resource/Model.h"
#include "Resource/Texture2D.h"

namespace Insight
{
    namespace Runtime
    {
        IS_SERIALISABLE_CPP(ResourceDatabase)

        ResourceDatabase::ResourceDatabase(ResourceDatabase const& other)
        {
        }

        void ResourceDatabase::Initialise()
        {
            ASSERT(Platform::IsMainThread());
            ResourceTypeIdToResource::RegisterResource(Material::GetStaticResourceTypeId(),  []() { return NewTracked(Material); });
            ResourceTypeIdToResource::RegisterResource(Mesh::GetStaticResourceTypeId(),      []() { return NewTracked(Mesh); });
            ResourceTypeIdToResource::RegisterResource(Model::GetStaticResourceTypeId(),     []() { return NewTracked(Model); });
            ResourceTypeIdToResource::RegisterResource(Texture2D::GetStaticResourceTypeId(), []() { return NewTracked(Texture2D); });
        }

        void ResourceDatabase::Shutdown()
        {
            ASSERT(Platform::IsMainThread());

            for (auto& pair : m_resources)
            {
                if (pair.second->IsLoaded())
                {
                    pair.second->UnLoad();
                }
                DeleteResource(pair.second);
            }

            m_resources.clear();
            m_dependentResources.clear();
        }

        TObjectPtr<IResource> ResourceDatabase::AddResource(ResourceId const& resourceId)
        {
            TObjectPtr<IResource> resource;
            if (HasResource(resourceId))
            {
                {
                    std::lock_guard lock(m_mutex);
                    resource = m_resources.find(resourceId)->second;
                }
                return resource;
            }

            IResource* rawResource = ResourceTypeIdToResource::CreateResource(resourceId.GetTypeId());
            ASSERT(rawResource);
            {
                std::lock_guard resourceLock(rawResource->m_mutex);
                rawResource->m_resourceId = resourceId;
                rawResource->m_file_path = resourceId.GetPath();
                rawResource->m_resource_state = EResoruceStates::Not_Loaded;
                rawResource->m_storage_type = ResourceStorageTypes::Disk;
                rawResource->OnLoaded.Bind<&ResourceDatabase::OnResourceLoaded>(this);
                rawResource->OnUnloaded.Bind<&ResourceDatabase::OnResourceUnloaded>(this);
            }
            TObjectOPtr<IResource> ownerResource = TObjectOPtr<IResource>(rawResource);
            {
                std::lock_guard lock(m_mutex);
                resource = m_resources[resourceId] = std::move(ownerResource);
            }


            return resource;
        }

        void ResourceDatabase::RemoveResource(TObjectPtr<IResource> resource)
        {
            if (HasResource(resource))
            {
                TObjectOPtr<IResource> resourceOPtr;
                ResourceId resourceId;
                {
                    std::lock_guard lock(m_mutex);
                    for (auto& pair : m_resources)
                    {
                        if (pair.second == resource)
                        {
                            resourceOPtr = std::move(pair.second);
                            resourceId = pair.first;
                            break;
                        }
                    }
                    m_resources.erase(resourceId);
                }
                DeleteResource(resourceOPtr);
            }
        }

        void ResourceDatabase::RemoveResource(ResourceId const& resourceId)
        {
            if (HasResource(resourceId))
            {
                TObjectOPtr<IResource> resourceOPtr;
                {
                    std::lock_guard lock(m_mutex);
                    auto iter = m_resources.find(resourceId);
                    resourceOPtr = std::move(iter->second);
                    m_resources.erase(iter);
                }
                DeleteResource(resourceOPtr);
            }
        }

        TObjectPtr<IResource> ResourceDatabase::GetResource(ResourceId const& resourceId) const
        {
            TObjectPtr<IResource> resource;
            {
                std::lock_guard lock(m_mutex);
                auto iter = m_resources.find(resourceId);
                ASSERT(iter != m_resources.end());
                resource = iter->second;
            }
            return resource;
        }

        TObjectPtr<IResource> ResourceDatabase::GetResourceFromGuid(Core::GUID const& guid) const
        {
            TObjectPtr<IResource> resource;
            {
                std::lock_guard lock(m_mutex);
                for (auto& pair : m_resources)
                {
                    if (pair.second->GetGuid() == guid) 
                    {
                        resource = pair.second;
                        break;
                    }
                }
            }
            return resource;
        }

        ResourceDatabase::ResourceMap ResourceDatabase::GetResourceMap() const
        {
            std::unordered_map<ResourceId, TObjectPtr<IResource>> ResourceMap;
            {
                std::lock_guard lock(m_mutex);
                for (const auto& pair : m_resources)
                {
                    ResourceMap[pair.first] = pair.second;
                }
            }
            return ResourceMap;
        }

        bool ResourceDatabase::HasResource(ResourceId const& resourceId) const
        {
            bool result;
            {
                std::lock_guard lock(m_mutex);
                result = m_resources.find(resourceId) != m_resources.end();
            }
            return result;
        }

        bool ResourceDatabase::HasResource(TObjectPtr<IResource> resource) const
        {
            bool result = false;
            {
                std::lock_guard lock(m_mutex);
                for (const auto& pair : m_resources)
                {
                    if (pair.second == resource)
                    {
                        result = true;
                        break;
                    }
                }
            }
            return result;
        }

        u32 ResourceDatabase::GetResourceCount() const
        {
            u32 result;
            {
                std::lock_guard lock(m_mutex);
                result = static_cast<u32>(m_resources.size());
            }
            return result;
        }

        u32 ResourceDatabase::GetLoadedResourceCount() const
        {
            u32 result;
            {
                std::lock_guard lock(m_mutex);
                result = m_loadedResourceCount;
            }
            return result;
        }

        u32 ResourceDatabase::GetLoadingResourceCount() const
        {
            u32 count = 0;
            {
                std::lock_guard lock(m_mutex);
                for (const auto& pair : m_resources)
                {
                    if (pair.second->GetResourceState() == EResoruceStates::Loading)
                    {
                        ++count;
                    }
                }
            }
            return count;
        }

        void ResourceDatabase::DeleteResource(TObjectOPtr<IResource>& resource)
        {
            ASSERT(resource->IsFailedToLoad()
                || resource->IsNotFound()
                || resource->IsNotLoaded()
                || resource->IsUnloaded());
            {
                std::lock_guard resourceLock(resource->m_mutex);
                resource->OnLoaded.Unbind<&ResourceDatabase::OnResourceLoaded>(this);
                resource->OnUnloaded.Unbind<&ResourceDatabase::OnResourceUnloaded>(this);
            }
            resource.Reset();
        }

        std::vector<ResourceId> ResourceDatabase::GetAllResourceIds() const
        {
            std::vector<ResourceId> result;
            {
                std::lock_guard lock(m_mutex);
                for (const auto& pair : m_resources)
                {
                    result.push_back(pair.first);
                }
            }
            return result;
        }

        void ResourceDatabase::OnResourceLoaded(IResource* Resource)
        {
            std::lock_guard lock(m_mutex);
            ++m_loadedResourceCount;
        }

        void ResourceDatabase::OnResourceUnloaded(IResource* Resource)
        {
            std::lock_guard lock(m_mutex);
            --m_loadedResourceCount;
        }

        TObjectPtr<IResource> ResourceDatabase::CreateDependentResource(ResourceId const& resourceId)
        {
            ASSERT(m_dependentResources.find(resourceId) == m_dependentResources.end());

            TObjectPtr<IResource> resource = nullptr;
            IResource* rawResource = ResourceTypeIdToResource::CreateResource(resourceId.GetTypeId());
            ASSERT(rawResource);
            {
                std::lock_guard resourceLock(rawResource->m_mutex);
                rawResource->m_resourceId = resourceId;
                rawResource->m_file_path = resourceId.GetPath();
                rawResource->m_resource_state = EResoruceStates::Not_Loaded;
                rawResource->m_storage_type = ResourceStorageTypes::Memory;
            }
            TObjectOPtr<IResource> ownerResource = TObjectOPtr<IResource>(rawResource);
            {
                std::lock_guard lock(m_mutex);
                resource = m_dependentResources[resourceId] = std::move(ownerResource);
            }

            return resource;
        }

        void ResourceDatabase::RemoveDependentResource(ResourceId const& resourceId)
        {
            std::lock_guard lock(m_mutex);
            auto iter = m_dependentResources.find(resourceId);
            if (iter != m_dependentResources.end())
            {
                m_dependentResources.erase(iter);
            }
        }
    }
}