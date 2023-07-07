#include "Resource/ResourceDatabase.h"
#include "Resource/ResourceDatabase.inl"
#include "Resource/ResourceManager.h"

#include "Resource/Material.h"
#include "Resource/Mesh.h"
#include "Resource/Model.h"
#include "Resource/Texture2D.h"
#include "Resource/ResourcePack.h"

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
            ResourceRegister::RegisterResource<Material>();
            ResourceRegister::RegisterResource<Mesh>();
            ResourceRegister::RegisterResource<Model>();
            ResourceRegister::RegisterResource<Texture2D>();

            Core::EventSystem::Instance().AddEventListener(this, Core::EventType::Project_Save, [this](const Core::Event& eve)
                {
                    ASSERT(Platform::IsMainThread());
                    std::lock_guard lock(m_mutex);
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

            for (auto& pack : m_resourcePacks)
            {
                pack->UnloadAllResources();
                pack->Close();
                ::Delete(pack);
            }

            for (auto& pair : m_resources)
            {
                if (pair.second->IsLoaded())
                {
                    pair.second->UnLoad();
                }
                DeleteResource(pair.second);
            }

            m_resourcePacks.clear();
            m_resources.clear();
            m_dependentResources.clear();

            Core::EventSystem::Instance().RemoveEventListener(this, Core::EventType::Project_Save);
        }

        ResourcePack* ResourceDatabase::CreateResourcePack(std::string_view filePath)
        {
            std::string packFilePath = FileSystem::ReplaceExtension(filePath, ResourcePack::c_Extension);
            
            if (HasResourcePack(packFilePath))
            {
                IS_CORE_WARN("[ResourceDatabase::CreateResourcePack] Resource pack already at location '{filePath}'.");
                return GetResourcePack(packFilePath);
            }

            ResourcePack* resourcePack = ::New<ResourcePack>(packFilePath);
            m_resourcePacks.push_back(resourcePack);
            return resourcePack;
        }

        ResourcePack* ResourceDatabase::LoadResourcePack(std::string_view filepath)
        {
            if (HasResourcePack(filepath))
            {
                return GetResourcePack(filepath);
            }

            Archive archive(filepath, ArchiveModes::Read);

            if (archive.IsEmpty())
            {
                IS_CORE_WARN("[ResourceDatabase::LoadResourcePack] Unable to load resource pack at '{}'.", filepath.data());
                return nullptr;
            }

            ResourcePack* resourcePack = ::New<ResourcePack>(filepath);
            m_resourcePacks.push_back(resourcePack);

            Serialisation::BinarySerialiser serialiser(true);
            serialiser.Deserialise(archive.GetData());

            resourcePack->Deserialise(&serialiser);
            resourcePack->Open();

            return resourcePack;
        }

        void ResourceDatabase::UnloadResourcePack(ResourcePack* resourcePack)
        {
            resourcePack->Close();
            FAIL_ASSERT();
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
                std::lock_guard lock(m_mutex);
                for (auto& pair : m_resources)
                {
                    if (pair.second->GetGuid() == guid) 
                    {
                        resource = pair.second;
                        break;
                    }
                }
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
                std::lock_guard lock(m_mutex);
                for (const auto& pair : m_resources)
                {
                    ResourceMap[pair.first] = pair.second;
                }
            }
            return ResourceMap;
        }

        std::vector<ResourcePack*> ResourceDatabase::GetResourcePacks() const
        {
            return m_resourcePacks;
        }

        bool ResourceDatabase::HasResource(ResourceId const& resourceId) const
        {
            bool result = false;
            {
                std::lock_guard lock(m_mutex);
                result |= m_resources.find(resourceId) != m_resources.end();
                result |= m_dependentResources.find(resourceId) != m_dependentResources.end();
            }
            return result;
        }

        bool ResourceDatabase::HasResource(TObjectPtr<IResource> resource) const
        {
            return HasResource(resource->GetResourceId());
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

        std::string ResourceDatabase::GetMetaFileForResource(const IResource* resource) const
        {
            std::string metaFilePath = resource->GetFilePath();
            metaFilePath += c_MetaFileExtension;
            return metaFilePath;
        }

        TObjectPtr<IResource> ResourceDatabase::AddResource(ResourceId const& resourceId, bool force)
        {
            if (!force && !FileSystem::Exists(resourceId.GetPath()))
            {
                IS_CORE_WARN("[ResourceDatabase::AddResource] Path '{}' doesn't exist.", resourceId.GetPath());
                return nullptr;
            }

            TObjectPtr<IResource> resource;
            if (HasResource(resourceId))
            {
                {
                    std::lock_guard lock(m_mutex);
                    resource = m_resources.find(resourceId)->second;
                }
                return resource;
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
                std::lock_guard lock(m_mutex);
                resource = m_resources[resourceId] = std::move(ownerResource);
            }

            LoadMetaFileData(resource);

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
            std::string path = resourceId.GetPath();
            if (path == "F:/Users/Documents/SourceControl/Github/C++Porjects/Insight/Resources/models/Main.1_Sponza/NewSponza_Main_glTF_002.gltf/arch_stones_01-0")
            {
                IS_CORE_INFO("");
            }
            ASSERT(m_dependentResources.find(resourceId) == m_dependentResources.end());

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
            std::unique_lock lock(m_mutex);

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

            lock.unlock();

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

        bool ResourceDatabase::HasResourcePack(std::string_view filePath) const
        {
            std::lock_guard packLock(m_resourcePacksMutex);
            for (const ResourcePack* const& pack : m_resourcePacks)
            {
                if (pack->GetFilePath() == filePath) 
                {
                    return true;
                }
            }
            return false;
        }

        ResourcePack* ResourceDatabase::GetResourcePack(std::string_view filePath) const
        {
            std::lock_guard packLock(m_resourcePacksMutex);
            for (ResourcePack* pack : m_resourcePacks)
            {
                if (pack->GetFilePath() == filePath)
                {
                    return pack;
                }
            }
            return nullptr;
        }

        ResourcePack* ResourceDatabase::GetResourcePackFromResourceId(ResourceId resourceId) const
        {
            std::lock_guard packLock(m_resourcePacksMutex);
            for (ResourcePack* pack : m_resourcePacks)
            {
                if (pack->HasResourceId(resourceId))
                {
                    return pack;
                }
            }
            return nullptr;
        }
    }
}