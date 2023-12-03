#include "Resource/ResourceDatabase.h"
#include "Resource/ResourceDatabase.inl"
#include "Resource/ResourceManager.h"
#include "Resource/Loaders/ResourceLoaderRegister.h"

#include "Asset/AssetRegistry.h"

#include "Runtime/ProjectSystem.h"
#include "Algorithm/Vector.h"

#include "Runtime/RuntimeEvents.h"

#include "FileSystem/FileSystem.h"
#include "Core/Logger.h"
#include "Serialisation/Archive.h"

#include "Event/EventSystem.h"

#include <fstream>

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
            Core::EventSystem::Instance().AddEventListener(this, Core::EventType::Project_Save, [this](const Core::Event& eve)
                {
                    ASSERT(Platform::IsMainThread());

                    std::lock_guard resourceLock(m_resourcesMutex);
                    for (auto& [id, resource] : m_resources)
                    {
                        if (resource->IsEngineFormat()
                            && resource->GetResourceStorageType() == ResourceStorageTypes::Disk)
                        {
                            IResource::ResourceSerialiserType serialiser(false);
                            resource->Serialise(&serialiser);
                            if (!serialiser.IsEmpty())
                            {
                                Archive archive(resource->GetFilePath(), ArchiveModes::Write);
                                archive.Write(serialiser.GetSerialisedData());
                                archive.Close();
                            }
                        }
                    }
                });
        }

        void ResourceDatabase::Shutdown()
        {
            ASSERT(Platform::IsMainThread());

            Clear();

            Core::EventSystem::Instance().RemoveEventListener(this, Core::EventType::Project_Save);
        }

        void ResourceDatabase::Clear()
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

            std::lock_guard resourceLock(m_resourcesMutex);
            m_resources.clear();
            std::lock_guard dependentResourceLock(m_dependentResourcesMutex);
            m_dependentResources.clear();
        }

        TObjectPtr<IResource> ResourceDatabase::AddResource(ResourceId const& resourceId)
        {
            return AddResource(resourceId, false);
        }

        void ResourceDatabase::RemoveResource(TObjectPtr<IResource> resource)
        {
            RemoveResource(resource->GetResourceId());
        }

        void ResourceDatabase::RemoveResource(ResourceId const& resourceId)
        {
            if (HasResource(resourceId))
            {
                TObjectOPtr<IResource> resourceOPtr;
                {
                    std::lock_guard resourceLock(m_resourcesMutex);
                    auto iter = m_resources.find(resourceId);
                    resourceOPtr = std::move(iter->second);
                    m_resources.erase(iter);
                }
                DeleteResource(resourceOPtr);
            }
        }

        void ResourceDatabase::UpdateGuidToResource(TObjectPtr<IResource>& resource)
        {
            std::lock_guard guidToResourceLock(m_guidToResourceMutex);
            m_guidToResources[resource->GetGuid()] = resource;
            for (auto& referenceLink : resource->GetReferenceLinks())
            {
                if (referenceLink.GetReferenceLinkType() == ResourceReferenceLinkType::Dependent_Owner)
                {
                    //IS_CORE_INFO("");
                }
            }
        }

        TObjectPtr<IResource> ResourceDatabase::GetResource(ResourceId const& resourceId) const
        {
            TObjectPtr<IResource> resource;
            {
                std::lock_guard resourceLock(m_resourcesMutex);
                std::lock_guard dependentResourceLock(m_dependentResourcesMutex);

                if (auto iter = m_resources.find(resourceId);
                    iter != m_resources.end())
                {
                    resource = iter->second;
                }
                else if (auto iter = m_dependentResources.find(resourceId);
                    iter != m_dependentResources.end())
                {
                    resource = iter->second;
                }
            }
            return resource;
        }

        TObjectPtr<IResource> ResourceDatabase::GetResourceFromGuid(Core::GUID const& guid) const
        {
            TObjectPtr<IResource> resource;
            {
                std::lock_guard resourceLock(m_resourcesMutex);
                for (auto& pair : m_resources)
                {
                    if (pair.second->GetGuid() == guid) 
                    {
                        resource = pair.second;
                        break;
                    }
                }

                std::lock_guard dependentResourceLock(m_dependentResourcesMutex);
                for (auto& pair : m_dependentResources)
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
                std::lock_guard resourceLock(m_resourcesMutex);
                for (const auto& pair : m_resources)
                {
                    ResourceMap[pair.first] = pair.second;
                }
            }
            return ResourceMap;
        }

        bool ResourceDatabase::HasResource(ResourceId const& resourceId) const
        {
            bool result = false;
            {
                std::lock_guard resourceLock(m_resourcesMutex);
                std::lock_guard dependentResourceLock(m_dependentResourcesMutex);
                result |= m_resources.find(resourceId) != m_resources.end();
                result |= m_dependentResources.find(resourceId) != m_dependentResources.end();
            }
            return result;
        }

        bool ResourceDatabase::HasResource(TObjectPtr<IResource> resource) const
        {
            return HasResource(resource->GetResourceId());
        }

        bool ResourceDatabase::HasResource(const Core::GUID& guid) const
        {
            bool result = false;
            {
                std::lock_guard resourceLock(m_resourcesMutex);
                for (const auto& [resourceId, resource] : m_resources)
                {
                    if (resource->GetGuid() == guid) {
                        return true;
                    }
                }
                std::lock_guard dependentResourceLock(m_dependentResourcesMutex);
                for (const auto& [resourceId, resource] : m_dependentResources)
                {
                    if (resource->GetGuid() == guid) {
                        return true;
                    }
                }
            }
            return result;
        }

        u32 ResourceDatabase::GetResourceCount() const
        {
            u32 result;
            {
                std::lock_guard resourceLock(m_resourcesMutex);
                result = static_cast<u32>(m_resources.size());
            }
            return result;
        }

        u32 ResourceDatabase::GetLoadedResourceCount() const
        {
            u32 result;
            {
                result = m_loadedResourceCount;
            }
            return result;
        }

        u32 ResourceDatabase::GetLoadingResourceCount() const
        {
            u32 count = 0;
            {
                std::lock_guard resourceLock(m_resourcesMutex);
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

        std::string ResourceDatabase::GetMetaFileForResource(const IResource* resource) const
        {
            std::string metaFilePath = resource->GetFilePath();
            metaFilePath += c_MetaFileExtension;
            return metaFilePath;
        }

        TObjectPtr<IResource> ResourceDatabase::AddResource(ResourceId const& resourceId, bool force)
        {
            if (!force && !AssetRegistry::Instance().GetAsset(resourceId.GetPath()))
            {
                IS_CORE_WARN("[ResourceDatabase::AddResource] Path '{}' doesn't exist.", resourceId.GetPath());
                return nullptr;
            }

            TObjectPtr<IResource> resource;
            if (HasResource(resourceId))
            {
                {
                    std::lock_guard lock(m_resourcesMutex);
                    resource = m_resources.find(resourceId)->second;
                }
                return resource;
            }

            const AssetInfo* assetInfo = AssetRegistry::Instance().GetAsset(resourceId.GetPath());
            if (!assetInfo)
            {
                FAIL_ASSERT();
                return nullptr;
            }

            IResource* rawResource = ResourceRegister::CreateResource(resourceId.GetTypeId(), resourceId.GetPath());
            ASSERT(rawResource);
            {
                std::lock_guard resourceLock(rawResource->m_mutex);
                rawResource->m_resourceId = resourceId;
                rawResource->m_resource_state = EResoruceStates::Not_Loaded;
                rawResource->m_storage_type = ResourceStorageTypes::Disk;
                rawResource->OnLoaded.Bind<&ResourceDatabase::OnResourceLoaded>(this);
                rawResource->OnUnloaded.Bind<&ResourceDatabase::OnResourceUnloaded>(this);
            }
            TObjectOPtr<IResource> ownerResource = TObjectOPtr<IResource>(rawResource);
            {
                std::lock_guard resourceLock(m_resourcesMutex);
                resource = m_resources[resourceId] = std::move(ownerResource);
            }

            LoadMetaFileData(resource);
            UpdateGuidToResource(resource);

            const AssetInfo* info = AssetRegistry::Instance().GetAsset(resourceId.GetPath());
            if (info)
            {
                AssetRegistry::Instance().RegisterObjectToAsset(info, resource);
            }

            return resource;
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
                std::lock_guard resourceLock(m_resourcesMutex);
                for (const auto& pair : m_resources)
                {
                    result.push_back(pair.first);
                }
            }
            return result;
        }

        void ResourceDatabase::OnResourceLoaded(IResource* Resource)
        {
            ++m_loadedResourceCount;
        }

        void ResourceDatabase::OnResourceUnloaded(IResource* Resource)
        {
            --m_loadedResourceCount;
        }

        TObjectPtr<IResource> ResourceDatabase::CreateDependentResource(ResourceId const& resourceId)
        {
            std::string path = resourceId.GetPath();
            if (path == "F:/Users/Documents/SourceControl/Github/C++Porjects/Insight/Resources/models/Main.1_Sponza/NewSponza_Main_glTF_002.gltf/arch_stones_01-0")
            {
                //IS_CORE_INFO("");
            }

            {
                std::lock_guard dependentResourceLock(m_dependentResourcesMutex);
                ASSERT(m_dependentResources.find(resourceId) == m_dependentResources.end());
            }

            TObjectPtr<IResource> resource = nullptr;
            IResource* rawResource = ResourceRegister::CreateResource(resourceId.GetTypeId(), resourceId.GetPath());
            ASSERT(rawResource);
            {
                std::lock_guard resourceLock(rawResource->m_mutex);
                rawResource->m_resourceId = resourceId;
                rawResource->m_resource_state = EResoruceStates::Not_Loaded;
                rawResource->m_storage_type = ResourceStorageTypes::Memory;
            }
            TObjectOPtr<IResource> ownerResource = TObjectOPtr<IResource>(rawResource);
            {
                std::lock_guard dependentResourceLock(m_dependentResourcesMutex);
                resource = m_dependentResources[resourceId] = std::move(ownerResource);
            }

            return resource;
        }

        void ResourceDatabase::RemoveDependentResource(ResourceId const& resourceId)
        {
            std::lock_guard dependentResourceLock(m_dependentResourcesMutex);

            auto iter = m_dependentResources.find(resourceId);
            if (iter != m_dependentResources.end())
            {
                ASSERT(!iter->second->GetFilePath().empty());
                iter->second.Reset();
                m_dependentResources.erase(iter);
            }
        }

        void ResourceDatabase::LoadMetaFileData(IResource* resource)
        {
            if (resource->IsEngineFormat())
            {
                return;
            }

            std::string metaFilePath = resource->GetFilePath();
            metaFilePath += c_MetaFileExtension;
            Archive metaFile(metaFilePath, ArchiveModes::Read, FileType::Text);
            metaFile.Close();
            if (!metaFile.IsEmpty())
            {
                Serialisation::JsonSerialiser serialiser(true);
                if (serialiser.Deserialise(metaFile.GetData()))
                {
                    resource->Deserialise(&serialiser);
                }
                else
                {
                    // Something went wrong when deserialising, so just save the meta file again.
                    SaveMetaFileData(resource, true);
                    LoadMetaFileData(resource);
                }
            }
            else
            {
                SaveMetaFileData(resource, true);
                LoadMetaFileData(resource);
            }
        }

        void ResourceDatabase::SaveMetaFileData(IResource* resource, bool overwrite)
        {
            ASSERT(!resource->IsEngineFormat());

            std::string metaFilePath = resource->GetFilePath();
            metaFilePath += c_MetaFileExtension;

            if (!overwrite && FileSystem::Exists(metaFilePath))
            {
                return;
            }

            Serialisation::JsonSerialiser serialiser(false);
            resource->Serialise(&serialiser);

            Archive metaFile(metaFilePath, ArchiveModes::Write, FileType::Text);
            metaFile.Write(serialiser.GetSerialisedData());
            metaFile.Close();
        }

        void ResourceDatabase::FindMissingResources()
        {
            std::vector<ResourceId> resourcesFound;
            // We really don't want to be doing this. This is a last ditch attempt when starting the engine 
            // to find any files that might have moved through File Explorer. Really files should be moved through the 
            // editor to allow for correct tracking and updating of data.
            if (!m_missingResources.empty())
            {
                IS_CORE_WARN("[ResourceDatabase::FindMissingResources] There are missing resources. Trying to find them.");
            
                if (ProjectSystem::Instance().IsProjectOpen())
                {
                    std::string contentPath = ProjectSystem::Instance().GetProjectInfo().GetContentPath();
                    for (const auto& pathIter : std::filesystem::recursive_directory_iterator(contentPath))
                    {
                        if (!pathIter.is_regular_file())
                        {
                            continue;
                        }

                        std::string fileName = pathIter.path().filename().string();

                        // Try and see if the current file iterating over has the same name as a missing file.
                        auto mapIter = m_missingResources.begin();
                        for (; mapIter != m_missingResources.end(); ++mapIter)
                        {
                            std::string iterPath = mapIter->first.GetPath();
                            std::string iterFileName = std::filesystem::path(iterPath).filename().string();
                            if (iterFileName == fileName)
                            {
                                break;
                            }
                        }
                        if (mapIter == m_missingResources.end())
                        {
                            continue;
                        }

                        const ResourceId& oldResourceId = mapIter->first;
                        std::string filePath = pathIter.path().string();
                        std::string filePathMeta = filePath;
                        filePathMeta += c_MetaFileExtension;

                        if (FileSystem::Exists(filePathMeta))
                        {
                            Archive metaFile(filePathMeta, ArchiveModes::Read);
                            metaFile.Close();
                            MetaFileSerialiser serialiser(true);

                            if (serialiser.Deserialise(metaFile.GetData()))
                            {
                                // Create a temp resources of the missing type and deserialise the meta data.
                                IResource* metaFileResource = ResourceRegister::CreateResource(oldResourceId.GetTypeId(), filePath);
                                metaFileResource->Deserialise(&serialiser);
                                Core::GUID metaFileGuid = metaFileResource->GetGuid();
                                Delete(metaFileResource);

                                // Compare the known guid from the database file and the serialised one we found.
                                // If both guids match then we have found our file.
                                bool foundResource = mapIter->second == metaFileGuid;
                                if (foundResource)
                                {
                                    ResourceId resourceIdToAdd(filePath, oldResourceId.GetTypeId());
                                    resourcesFound.push_back(resourceIdToAdd);
                                }
                                else
                                {
                                    IS_CORE_ERROR("[ResourceDatabase::FindMissingResources] Was unable to find resource '{}'.", mapIter->first.GetPath());
                                }
                            }
                            else
                            {
                                IS_CORE_ERROR("[ResourceDatabase::FindMissingResources] Unable to deserialise meta file.");
                            }
                        }
                        else
                        {
                            IS_CORE_ERROR("[ResourceDatabase::FindMissingResources] Missing '{}' file.", c_MetaFileExtension);
                        }

                    }
                }
            }

            if (!resourcesFound.empty())
            {
                IS_CORE_INFO("[ResourceDatabase::FindMissingResources] Resources found:");
                for (const ResourceId& resourceId : resourcesFound)
                {
                    IS_CORE_INFO("\t'{}'", resourceId.GetPath());
                    AddResource(resourceId);
                }
            }
        }
    }
}