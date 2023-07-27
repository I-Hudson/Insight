#include "Resource/IResourceManager.h"
#include "Resource/ResourceDatabase.h"
#include "Resource/ResourcePack.h"
#include "Resource/Loaders/ResourceLoaderRegister.h"

#include "Resource/Loaders/BinaryLoader.h"
#include "Resource/Loaders/TextLoader.h"
#include "Resource/Loaders/ModelLoader.h"
#include "Resource/Loaders/TextureLoader.h"


#include "Runtime/ProjectSystem.h"

#include "Core/Logger.h"

#include "Algorithm/Vector.h"

#include "FileSystem/FileSystem.h"
#include "Threading/TaskSystem.h"

#include "Serialisation/Serialisers/JsonSerialiser.h"
#include "Serialisation/Serialisers/BinarySerialiser.h"
#include "Serialisation/Archive.h"

namespace Insight
{
    namespace Runtime
    {
#define RESOURCE_LOAD_THREAD
        IResourceManager::IResourceManager()
        {
        }

        IResourceManager::~IResourceManager()
        {
        }

        void IResourceManager::Update(float const deltaTime)
        {
            ASSERT(Platform::IsMainThread());

            u32 resourcesToLoad = 0;

            u32 resourcesCurrentlyLoading = 0;
            {
                std::lock_guard resourcesLoadingLock(m_resourcesLoadingMutex);
                // Check for all loaded resources.
                for (size_t i = 0; i < m_resourcesLoading.size(); ++i)
                {
                    if (m_resourcesLoading.at(i)->IsLoaded())
                    {
                        ++resourcesToLoad;
                    }
                }

            //    // Remove all loaded resources from m_resourcesLoading.
            //    for (IResource* const resource : resourceLoaded)
            //    {
            //        Algorithm::VectorRemove(m_resourcesLoading, resource);
            //    }
                resourcesCurrentlyLoading = static_cast<u32>(m_resourcesLoading.size());
            }

            {
                std::lock_guard queueLock(m_queuedResoucesToLoadMutex);
                // Start loading more resources.
                for (int i = resourcesToLoad; i >= 0; --i)
                {
                    if (resourcesCurrentlyLoading >= c_MaxLoadingResources
                        || m_queuedResoucesToLoad.empty())
                    {
                        break;
                    }
                    IResource* resourceToLoad = m_queuedResoucesToLoad.front();
                    m_queuedResoucesToLoad.pop();
                    StartLoading(resourceToLoad, true);
                }
            }
        }

        void IResourceManager::SaveDatabase()
        {
            ASSERT(Platform::IsMainThread());

            if (!m_database)
            {
                return;
            }
            Serialisation::BinarySerialiser binarySerialiser(false);
            Serialisation::JsonSerialiser jsonSerialiser(false);

            m_database->Serialise(&binarySerialiser);
            m_database->Serialise(&jsonSerialiser);
            
            Archive archive(Runtime::ProjectSystem::Instance().GetProjectInfo().GetIntermediatePath() + "/ResourceDatabase.isdatabase", ArchiveModes::Write);
            archive.Write(binarySerialiser.GetSerialisedData());
            archive.Close();

            archive = Archive(Runtime::ProjectSystem::Instance().GetProjectInfo().GetIntermediatePath() + "/ResourceDatabaseJson.isdatabase", ArchiveModes::Write, FileType::Text);
            archive.Write(jsonSerialiser.GetSerialisedData());
            archive.Close();
        }

        void IResourceManager::LoadDatabase()
        {
            ASSERT(Platform::IsMainThread());

            if (!m_database)
            {
                return;
            }
            Archive archive(Runtime::ProjectSystem::Instance().GetProjectInfo().GetIntermediatePath() + "/ResourceDatabase.isdatabase", ArchiveModes::Read);
            archive.Close();

            if (!archive.GetData().empty())
            {
                Serialisation::BinarySerialiser serialiser(true);
                serialiser.Deserialise(archive.GetData());
                m_database->Deserialise(&serialiser);
                m_database->FindMissingResources();
            }
        }

        void IResourceManager::ClearDatabase()
        {
            ASSERT(Platform::IsMainThread());

            // Finish loading all resources. This allows us to release them correctly.
            while (!m_resourcesLoading.empty())
            {
                Update(0.16f);
            }

            // Pop all queued resources.
            {
                std::lock_guard queueLock(m_queuedResoucesToLoadMutex);
                while (!m_queuedResoucesToLoad.empty())
                {
                    auto resource = m_queuedResoucesToLoad.front();
                    m_queuedResoucesToLoad.pop();
                    resource->m_resource_state = EResoruceStates::Cancelled;
                }
            }

            m_database->Clear();
        }

        void IResourceManager::Initialise()
        {
            ASSERT(m_database == nullptr);
            m_database = New<ResourceDatabase>();
            m_database->Initialise();

            m_loaderRegistry = New<ResourceLoaderRegister>();
			m_loaderRegistry->RegisterResourceLoader<BinaryLoader>();
			m_loaderRegistry->RegisterResourceLoader<TextureLoader>();
			m_loaderRegistry->RegisterResourceLoader<ModelLoader>();
			m_loaderRegistry->RegisterResourceLoader<TextureLoader>();
        }

        void IResourceManager::Shutdown()
        {
            ASSERT(Platform::IsMainThread());

            ClearDatabase();

            m_database->Shutdown();
            Delete(m_database);
            ASSERT(m_database == nullptr);

			m_loaderRegistry->Shutdown();
            Delete(m_loaderRegistry);
        }

        void IResourceManager::SetDebugDirectories(std::string metaFileDirectory, std::string assetReativeBaseDirectory)
        {
            if (!FileSystem::Exists(metaFileDirectory))
            {
                FileSystem::CreateFolder(metaFileDirectory);
            }
            m_debugMetaFileDirectory = std::move(metaFileDirectory);
            m_assetReativeBaseDirectory = std::move(assetReativeBaseDirectory);
        }

        void IResourceManager::LoadResourcesInFolder(std::string_view path, bool recursive)
        {
            std::string absFolderPath = FileSystem::GetAbsolutePath(path);
            if (recursive)
            {
                for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(absFolderPath))
                {
                    std::string path = entry.path().string();
                    FileSystem::PathToUnix(path);
                    LoadSync(path, false);
                }
            }
            else
            {
                for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(absFolderPath))
                {
                    std::string path = entry.path().string();
                    FileSystem::PathToUnix(path);
                    LoadSync(path, false);
                }
            }
        }

        ResourcePack* IResourceManager::CreateResourcePack(std::string_view filePath)
        {
            ASSERT(m_database);
            return m_database->CreateResourcePack(filePath);
        }

        TObjectPtr<IResource> IResourceManager::Create(ResourceId const& resourceId)
        {
            ASSERT(m_database);

            TObjectPtr<IResource> resource;
            if (m_database->HasResource(resourceId))
            {
                resource = m_database->GetResource(resourceId);
            }
            else
            {
                resource = m_database->AddResource(resourceId, true);
            }
            return resource;
        }

        TObjectPtr<IResource> IResourceManager::GetResource(ResourceId const& resourceId) const
        {
            ASSERT(m_database);
            if (m_database->HasResource(resourceId))
            {
                return m_database->GetResource(resourceId);
            }
            return nullptr;
        }

        TObjectPtr<IResource> IResourceManager::GetResourceFromGuid(Core::GUID const& guid) const
        {
            ASSERT(m_database);
            return m_database->GetResourceFromGuid(guid);
        }

        TObjectPtr<IResource> IResourceManager::LoadSync(ResourceId resourceId)
        {
            return LoadSync(std::move(resourceId), false);
        }

        TObjectPtr<IResource> IResourceManager::LoadSync(ResourceId resourceId, bool convertToEngineFormat)
        {
            ASSERT(m_database);

            if (!resourceId)
            {
                std::string_view extension = FileSystem::GetFileExtension(resourceId.GetPath());
                resourceId = ResourceId(resourceId.GetPath(), ResourceRegister::GetResourceTypeIdFromExtension(extension));
            }

            if (!resourceId)
            {
                IS_CORE_ERROR("[IResourceManager::LoadSync] Invalid 'ResourceId' was given and no resource Id could be found the file '{}'.", resourceId.GetPath());
                return nullptr;
            }

            const IResourceLoader* loader = ResourceLoaderRegister::GetLoaderFromExtension(FileSystem::GetFileExtension(resourceId.GetPath()));
            if (!loader)
            {
                IS_CORE_WARN("[IResourceManager::StartLoading] Resource '{}' failed to load as no loader could be found.", resourceId.GetPath());
                return nullptr;
            }
            else if (loader->GetResourceTypeId() != resourceId)
            {
                return  nullptr;
            }

            if (convertToEngineFormat)
            {
                resourceId = ConvertResource(std::move(resourceId));
            }

            TObjectPtr<IResource> resource;
            if (m_database->HasResource(resourceId))
            {
                resource = m_database->GetResource(resourceId);
            }
            else
            {
                resource = m_database->AddResource(resourceId);
            }

            if (resource)
            {
                if (resource->IsLoaded())
                {
                    // Item is already loaded, just return it.
                    return resource;
                }

                if (resource->IsNotLoaded() || resource->IsUnloaded())
                {
                    if (resource->GetResourceStorageType() == ResourceStorageTypes::Disk)
                    {
                        if (!FileSystem::Exists(resource->GetFilePath()))
                        {
                            // File does not exists. Set the resource state and return nullptr.
                            resource->m_resource_state = EResoruceStates::Not_Found;
                            return resource;
                        }
                        else
                        {
                            resource->m_resource_state = EResoruceStates::Queued;
                            StartLoading(resource.Get(), false);
                        }
                    }
                    else
                    {
                        FAIL_ASSERT_MSG("[IResourceManager::Load] Maybe this should be done. Maybe when an resource is being loaded from disk it should handle loading memory resources.");
                    }
                }
            }
            return resource;
        }

        TObjectPtr<IResource> IResourceManager::LoadSync(std::string_view filepath, bool convertToEngineFormat)
        {
            std::string_view fileExtension = FileSystem::GetFileExtension(filepath);
            const Runtime::IResourceLoader* loader = Runtime::ResourceLoaderRegister::GetLoaderFromExtension(fileExtension);
            if (!loader)
            {
                return nullptr;
            }
            return LoadSync(ResourceId(filepath, loader->GetResourceTypeId()), convertToEngineFormat);
        }

        TObjectPtr<IResource> IResourceManager::Load(ResourceId resourceId)
        {
            return Load(std::move(resourceId), false);
        }

        TObjectPtr<IResource> IResourceManager::Load(ResourceId resourceId, bool convertToEngineFormat)
        {
            ASSERT(m_database);

            if (convertToEngineFormat)
            {
                resourceId = ConvertResource(std::move(resourceId));
            }

            TObjectPtr<IResource> resource;
            if (m_database->HasResource(resourceId))
            {
                resource = m_database->GetResource(resourceId);
            }
            else
            {
                resource = m_database->AddResource(resourceId);
            }

            if (resource)
            {
                if (resource->IsLoaded())
                {
                    // Item is already loaded, just return it.
                    return resource;
                }

                if (resource->IsNotLoaded() || resource->IsUnloaded())
                {
                    if (resource->GetResourceStorageType() == ResourceStorageTypes::Disk)
                    {
                        if (!FileSystem::Exists(resource->GetFilePath()))
                        {
                            // File does not exists. Set the resource state and return nullptr.
                            resource->m_resource_state = EResoruceStates::Not_Found;
                            return resource;
                        }
                        else
                        {
                            resource->m_resource_state = EResoruceStates::Queued;
                            // Try and load the resource as it exists.
                            std::unique_lock queueLock(m_queuedResoucesToLoadMutex);
                            m_queuedResoucesToLoad.push(resource);
                            queueLock.unlock();
                        }
                    }
                    else
                    {
                        FAIL_ASSERT_MSG("[IResourceManager::Load] Maybe this should be done. Maybe when an resource is being loaded from disk it should handle loading memory resources.");
                    }
                }
            }
            return resource;
        }

        TObjectPtr<IResource> IResourceManager::LoadSync(const Core::GUID& guid)
        {
            return Load(guid, false, false);
        }

        TObjectPtr<IResource> IResourceManager::LoadSync(const Core::GUID& guid, bool convertToEngineFormat)
        {
            return Load(guid, convertToEngineFormat, false);
        }

        TObjectPtr<IResource> IResourceManager::Load(const Core::GUID& guid)
        {
            return Load(guid, true, false);
        }

        TObjectPtr<IResource> IResourceManager::Load(const Core::GUID& guid, bool loadAsyncs, bool convertToEngineFormat)
        {
            ASSERT(m_database);

            if (!guid.IsValid())
            {
                IS_CORE_WARN("[IResourceManager::Load] Invalid Guid.");
                return nullptr;
            }

            if (convertToEngineFormat)
            {
                FAIL_ASSERT();
                //resourceId = ConvertResource(std::move(resourceId));
            }

            TObjectPtr<IResource> resource;
            if (m_database->HasResource(guid))
            {
                resource = m_database->GetResourceFromGuid(guid);
            }
            else
            {
                FAIL_ASSERT();
            }

            if (resource)
            {
                if (resource->IsLoaded())
                {
                    // Item is already loaded, just return it.
                    return resource;
                }

                if (resource->IsNotLoaded() || resource->IsUnloaded())
                {
                    if (resource->GetResourceStorageType() == ResourceStorageTypes::Disk)
                    {
                        if (!FileSystem::Exists(resource->GetFilePath()))
                        {
                            // File does not exists. Set the resource state and return nullptr.
                            resource->m_resource_state = EResoruceStates::Not_Found;
                            return resource;
                        }
                        else
                        {
                            resource->m_resource_state = EResoruceStates::Queued;
                            StartLoading(resource.Get(), false);
                        }
                    }
                    else
                    {
                        FAIL_ASSERT_MSG("[IResourceManager::Load] Maybe this should be done. Maybe when an resource is being loaded from disk it should handle loading memory resources.");
                    }
                }
            }
            return resource;
        }

        ResourcePack* IResourceManager::LoadResourcePack(std::string_view filepath)
        {
            ASSERT(m_database);
            return m_database->LoadResourcePack(filepath);
        }

        void IResourceManager::UnloadResourcePack(ResourcePack* resourcePack)
        {
            ASSERT(m_database);
            m_database->UnloadResourcePack(resourcePack);
        }

        void IResourceManager::Unload(std::string_view filePath)
        {
            std::string_view fileExtension = FileSystem::GetFileExtension(filePath);
            const Runtime::IResourceLoader* loader = Runtime::ResourceLoaderRegister::GetLoaderFromExtension(fileExtension);
            if (!loader)
            {
                return;
            }
            return Unload(ResourceId(filePath, loader->GetResourceTypeId()));
        }

        void IResourceManager::Unload(ResourceId const& resourceId)
        {
            ASSERT(m_database);

            TObjectPtr<IResource> resource = nullptr;
            if (m_database->HasResource(resourceId))
            {
                resource = m_database->GetResource(resourceId);
            }

            if (resource->IsDependentOnAnotherResource())
            {
                // Resource is dependent on another resource. The owning resource should handle unloading this resource.
                return;
            }

            if (!resource)
            {
                IS_CORE_WARN("[IResourceManager::UnloadResource] The resource '{0}' is not valid (null). The Resource isn't tracked by the ResourceDatabase.", resourceId.GetPath());
                return;
            }

            if (resource->GetResourceState() != EResoruceStates::Loaded)
            {
                IS_CORE_WARN("[IResourceManager::Unload] 'resource' current state is '{0}'. Resource must be loaded to be unloaded."
                    , ERsourceStatesToString(resource->GetResourceState()));
                return;
            }

            // Unload the resource,
            resource->m_resource_state = EResoruceStates::Unloading;
            resource->StartUnloadTimer();
            {
                //std::lock_guard resourceLock(resource->m_mutex);
                resource->UnLoad();
            }
            resource->StopUnloadTimer();
            resource->m_resource_state = EResoruceStates::Unloaded;
            resource->OnUnloaded(resource);
        }

        void IResourceManager::Unload(TObjectPtr<IResource> Resource)
        {
            if (Resource)
            {
                Unload(Resource->m_resourceId);
            }
        }

        void IResourceManager::UnloadAll()
        {
            ASSERT(Platform::IsMainThread());
            ASSERT(m_database);

            std::vector<ResourceId> ResourceIds = m_database->GetAllResourceIds();
            for (const ResourceId& id : ResourceIds)
            {
                Unload(id);
            }
        }

        bool IResourceManager::HasResource(ResourceId const& resourceId) const
        {
            ASSERT(m_database);
            return m_database->HasResource(resourceId);
        }

        bool IResourceManager::HasResource(TObjectPtr<IResource> Resource) const
        {
            if (Resource)
            {
                return HasResource(Resource->GetResourceId());
            }
            return false;
        }

        ResourceDatabase::ResourceMap IResourceManager::GetResourceMap() const
        {
            ASSERT(m_database);
            return m_database->GetResourceMap();
        }

        std::vector<ResourcePack*> IResourceManager::GetResourcePacks() const
        {
            ASSERT(m_database);
            return m_database->GetResourcePacks();
        }

        u32 IResourceManager::GetQueuedToLoadCount() const
        {
            u32 count = 0;
            {
                std::lock_guard lock(m_queuedResoucesToLoadMutex);
                count = static_cast<u32>(m_queuedResoucesToLoad.size());
            }
            return count;
        }

        u32 IResourceManager::GetLoadedResourcesCount() const
        {
            ASSERT(m_database);
            return m_database->GetLoadedResourceCount();
        }

        u32 IResourceManager::GetLoadingCount() const
        {
            ASSERT(m_database);
            return m_database->GetLoadingResourceCount();
        }

        void IResourceManager::StartLoading(IResource* resource, bool threading)
        {
            {
                std::lock_guard resourcesLoadingLock(m_resourcesLoadingMutex);
                m_resourcesLoading.push_back(resource);
            }
            resource->m_resource_state = EResoruceStates::Loading;

            const IResourceLoader* loader = ResourceLoaderRegister::GetLoaderFromResource(resource);
            if (!loader)
            {
                IS_CORE_WARN("[IResourceManager::StartLoading] Resource '{}' failed to load as no loader could be found.", resource->GetFileName());
            }

            std::function<void(IResource* resource, const IResourceLoader* loader, const bool threaded)> resourceLoadFunc = 
                [this](IResource* resource, const IResourceLoader* loader, const bool threaded)
            {
                bool resourceLoaded = false;
                resource->StartLoadTimer();
                {
                    if (resource->GetResourcePackInfo().IsWithinPack)
                    {
                        // Resource is within a pack. Check if that resource is currently serialised, if so then load from the pack
                        // otherwise try and load it from loose files.
                        ResourcePack* resourcePack = m_database->GetResourcePackFromResourceId(resource->GetResourceId());

                        if (resourcePack)
                        {
                            ResourcePack::PackedResource entry = resourcePack->GetEntry(resource->GetResourceId());
                            if (entry.IsSerialised)
                            {
                                resourcePack->LoadResource(entry);
                                resourceLoaded = true;
                            }
                            else
                            {
                                IS_CORE_WARN("[IResourceManager::StartLoading] Trying to load resource '{}' from a resource pack '{}'. Resource is not serialised in pack falling back to trying to load from disk."
                                , resource->GetFilePath(), resourcePack->GetFilePath());
                            }
                        }
                    }

                    if (!resourceLoaded && resource->IsEngineFormat())
                    {
                        Archive archive(resource->GetFilePath(), ArchiveModes::Read, FileType::Binary);
                        archive.Close();
                        if (!archive.IsEmpty())
                        {
                            IResource::ResourceSerialiserType serialiser(true);
                            serialiser.Deserialise(archive.GetData());
                            resource->Deserialise(&serialiser);                   
                            resource->m_resource_state = EResoruceStates::Loaded;
                        }
                        else
                        {
                            // Something has gone wrong when tring to load the resource.
                            resource->m_resource_state = EResoruceStates::Failed_To_Load;
                        }
                    }
                    else if (!resourceLoaded)
                    {
                        //std::lock_guard resourceLock(resource->m_mutex); // FIXME Maybe don't do this?
                        if (loader)
                        {
                            if (loader->Load(resource))
                            {
                                resource->m_resource_state = EResoruceStates::Loaded;
                            }
                            else
                            {
                                // Something has gone wrong when trying to load the resource.
                                resource->m_resource_state = EResoruceStates::Failed_To_Load;
                            }
                        }
                    }
                }
                resource->StopLoadTimer();

                if (resource->IsLoaded())
                {
                    if (threaded == true)
                    {
                        ASSERT(0 == 0);
                    }
                    m_database->SaveMetaFileData(resource, true);
                    resource->OnLoaded(resource);
                }

                std::lock_guard resourceLoadingGuard(m_resourcesLoadingMutex);
                Algorithm::VectorRemove(m_resourcesLoading, resource);
            };


            if (threading)
            {
                Threading::TaskSystem::CreateTask([resource, loader, resourceLoadFunc]()
                    {
                        resourceLoadFunc(resource, loader, true);
                    });
            }
            else
            {
                resourceLoadFunc(resource, loader, false);
                {
                    std::lock_guard resourcesLoadingLock(m_resourcesLoadingMutex);
                    Algorithm::VectorRemove(m_resourcesLoading, resource);
                }
            }
        }

        ResourceId IResourceManager::ConvertResource(ResourceId resourceId)
        {
            /// Hack: 
            /// Load the resource as normal. 
            /// Then serialise is to disk with the correct extension.
            /// Then unload and delete the original resource.

            /// Check if the source resource is loaded. If it is then don't unload it 
            /// as it has been loaded by something else so might be in use.
            bool resourceAllreadyLoaded = HasResource(resourceId);

            IResource* sourceResource = LoadSync(resourceId).Get();
            ASSERT(sourceResource);
            ResourceId engineFormatResourceId = sourceResource->ConvertToEngineFormat();

            if (!resourceAllreadyLoaded)
            {
                RemoveResource(resourceId);
            }
            return engineFormatResourceId;
        }

        void IResourceManager::RemoveResource(ResourceId resourceId)
        {
            if (!resourceId)
            {
                return;
            }

            Unload(resourceId);
            m_database->RemoveResource(resourceId);
        }

        TObjectPtr<IResource> IResourceManager::CreateDependentResource(ResourceId const& resourceId)
        {
            ASSERT(m_database);
            return m_database->CreateDependentResource(resourceId);
        }

        void IResourceManager::RemoveDependentResource(ResourceId const& resourceId)
        {
            ASSERT(m_database);
            return m_database->RemoveDependentResource(resourceId);
        }

        std::string IResourceManager::GetMetaPath(const IResource* resource) const
        {
            ASSERT(m_database);
            return m_database->GetMetaFileForResource(resource);
        }
    }
}