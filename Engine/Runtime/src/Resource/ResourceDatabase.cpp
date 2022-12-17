#include "Resource/ResourceDatabase.h"
#include "Resource/ResourceManager.h"

namespace Insight
{
    namespace Runtime
    {
        void ResourceDatabase::Initialise()
        {
            ASSERT(Platform::IsMainThread());
        }

        void ResourceDatabase::Shutdown()
        {
            ASSERT(Platform::IsMainThread());
            for (auto& pair : m_resources)
            {
                DeleteResource(pair.second);
            }
            m_resources.clear();
        }

        TObjectPtr<IResource> ResourceDatabase::AddResouce(ResourceId resourceId)
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
            {
                std::lock_guard resourceLock(rawResource->m_mutex);
                rawResource->m_resourceId = resourceId;
                rawResource->m_file_path = resourceId.GetPath();
                rawResource->m_resource_state = EResoruceStates::Not_Loaded;
                rawResource->m_storage_type = ResourceStorageTypes::Disk;
                rawResource->OnLoaded.Bind<&ResourceDatabase::OnResouceLoaded>(this);
                rawResource->OnUnloaded.Bind<&ResourceDatabase::OnResouceUnloaded>(this);
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

        void ResourceDatabase::RemoveResource(ResourceId resourceId)
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

        TObjectPtr<IResource> ResourceDatabase::GetResource(ResourceId resourceId) const
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

        ResourceDatabase::ResouceMap ResourceDatabase::GetResouceMap() const
        {
            std::unordered_map<ResourceId, TObjectPtr<IResource>> resouceMap;
            {
                std::lock_guard lock(m_mutex);
                for (const auto& pair : m_resources)
                {
                    resouceMap[pair.first] = pair.second;
                }
            }
            return resouceMap;
        }

        bool ResourceDatabase::HasResource(ResourceId resourceId) const
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

        void ResourceDatabase::DeleteResource(TObjectOPtr<IResource>& resource)
        {
            ASSERT(resource->IsFailedToLoad()
                || resource->IsNotFound()
                || resource->IsNotLoaded()
                || resource->IsUnloaded());
            {
                std::lock_guard resourceLock(resource->m_mutex);
                resource->OnLoaded.Unbind<&ResourceDatabase::OnResouceLoaded>(this);
                resource->OnUnloaded.Unbind<&ResourceDatabase::OnResouceUnloaded>(this);
            }
            resource.Reset();
        }

        std::vector<ResourceId> ResourceDatabase::GetAllResouceIds() const
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

        void ResourceDatabase::OnResouceLoaded(IResource* resouce)
        {
            std::lock_guard lock(m_mutex);
            ++m_loadedResourceCount;
        }

        void ResourceDatabase::OnResouceUnloaded(IResource* resouce)
        {
            std::lock_guard lock(m_mutex);
            --m_loadedResourceCount;
        }
    }
}