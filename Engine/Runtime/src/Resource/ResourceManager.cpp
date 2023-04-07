#include "Resource/ResourceManager.h"
#include "Resource/ResourceDatabase.h"
#include "Resource/Loaders/ResourceLoaderRegister.h"
#include "Resource/Loaders/IResourceLoader.h"

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

        ResourceDatabase* ResourceManager::s_database;
        std::vector<IResource*> ResourceManager::s_resourcesLoading;
        std::queue<IResource*> ResourceManager::s_queuedResoucesToLoad;
        std::mutex ResourceManager::s_queuedResoucesToLoadMutex;

        void ResourceManager::Update(float const deltaTime)
        {
            ASSERT(Platform::IsMainThread());

            u32 resourcesToLoad = 0;
            std::vector<IResource*> resourceLoaded;

            // Check for all loaded resources.
            for (size_t i = 0; i < s_resourcesLoading.size(); ++i)
            {
                if (s_resourcesLoading.at(i)->IsLoaded())
                {
                    resourceLoaded.push_back(s_resourcesLoading.at(i));
                    ++resourcesToLoad;
                }
            }

            // Remove all loaded resources from s_resourcesLoading.
            for (IResource* const resource : resourceLoaded)
            {
                Algorithm::VectorRemove(s_resourcesLoading, resource);
            }

            std::unique_lock queueLock(s_queuedResoucesToLoadMutex);
            // Start loading more resources.
            for (int i = resourcesToLoad; i >= 0 ; --i)
            {
                if (s_resourcesLoading.size() >= c_MaxLoadingResources
                    || s_queuedResoucesToLoad.empty())
                {
                    break;
                }
                IResource* resourceToLoad = s_queuedResoucesToLoad.front();
                s_queuedResoucesToLoad.pop();
                StartLoading(resourceToLoad, true);
            }
            queueLock.unlock();
        }

        void ResourceManager::SaveDatabase()
        {
            ASSERT(Platform::IsMainThread());

            if (!s_database)
            {
                return;
            }
            Serialisation::BinarySerialiser binarySerialiser(false);
           // Serialisation::JsonSerialiser jsonSerialiser(false);

            s_database->Serialise(&binarySerialiser);
            //s_database->Serialise(&jsonSerialiser);
            
            Archive archive(Runtime::ProjectSystem::Instance().GetProjectInfo().GetIntermediatePath() + "/ResourceDatabase.isdatabase", ArchiveModes::Write);
            archive.Write(binarySerialiser.GetSerialisedData());
            archive.Close();

            //archive = Archive(Runtime::ProjectSystem::Instance().GetProjectInfo().GetIntermediatePath() + "/ResourceDatabaseJson.isdatabase", ArchiveModes::Write);
            //archive.Write(jsonSerialiser.GetSerialisedData());
            //archive.Close();
        }

        void ResourceManager::LoadDatabase()
        {
            ASSERT(Platform::IsMainThread());

            if (!s_database)
            {
                return;
            }
            Archive archive(Runtime::ProjectSystem::Instance().GetProjectInfo().GetIntermediatePath() + "/ResourceDatabase.isdatabase", ArchiveModes::Read);
            archive.Close();

            if (!archive.GetData().empty())
            {
                Serialisation::BinarySerialiser serialiser(true);
                serialiser.Deserialise(archive.GetData());
                s_database->Deserialise(&serialiser);
            }
        }

        void ResourceManager::ClearDatabase()
        {
            ASSERT(Platform::IsMainThread());

            s_database->Shutdown();
        }

        TObjectPtr<IResource> ResourceManager::Create(ResourceId const& resourceId)
        {
            ASSERT(s_database);

            TObjectPtr<IResource> resource;
            if (s_database->HasResource(resourceId))
            {
                resource = s_database->GetResource(resourceId);
            }
            else
            {
                resource = s_database->AddResource(resourceId);
            }
            return resource;
        }

        TObjectPtr<IResource> ResourceManager::GetResource(ResourceId const& resourceId)
        {
            ASSERT(s_database);
            if (s_database->HasResource(resourceId))
            {
                return s_database->GetResource(resourceId);
            }
            return nullptr;
        }

        TObjectPtr<IResource> ResourceManager::GetResourceFromGuid(Core::GUID const& guid)
        {
            ASSERT(s_database);
            return s_database->GetResourceFromGuid(guid);
        }

        TObjectPtr<IResource> ResourceManager::LoadSync(ResourceId const& resourceId)
        {
            ASSERT(s_database);
            ASSERT(Platform::IsMainThread());

            TObjectPtr<IResource> resource;
            if (s_database->HasResource(resourceId))
            {
                resource = s_database->GetResource(resourceId);
            }
            else
            {
                resource = s_database->AddResource(resourceId);
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
                        if (!FileSystem::FileSystem::Exists(resource->GetFilePath()))
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
                        FAIL_ASSERT_MSG("[ResourceManager::Load] Maybe this should be done. Maybe when an resource is being loaded from disk it should handle loading memory resources.");
                    }
                }
            }
            return resource;
        }

        TObjectPtr<IResource> ResourceManager::LoadSync(std::string_view filePath)
        {
            std::string_view fileExtension = FileSystem::FileSystem::GetFileExtension(filePath);
            const IResourceLoader* loader = ResourceLoaderRegister::GetLoaderFromExtension(fileExtension);
            if (!loader)
            {
                IS_CORE_WARN("[ResourceManager::LoadSync] loader is null for extension '{}'.", fileExtension);
                return nullptr;
            }

            ResourceId resourceId(filePath, loader->GetResourceTypeId());
            return LoadSync(resourceId);
        }

        TObjectPtr<IResource> ResourceManager::Load(ResourceId const& resourceId)
        {
            ASSERT(s_database);

            TObjectPtr<IResource> resource;
            if (s_database->HasResource(resourceId))
            {
                resource = s_database->GetResource(resourceId);
            }
            else
            {
                resource = s_database->AddResource(resourceId);
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
                        if (!FileSystem::FileSystem::Exists(resource->GetFilePath()))
                        {
                            // File does not exists. Set the resource state and return nullptr.
                            resource->m_resource_state = EResoruceStates::Not_Found;
                            return resource;
                        }
                        else
                        {
                            resource->m_resource_state = EResoruceStates::Queued;
                            // Try and load the resource as it exists.
                            std::unique_lock queueLock(s_queuedResoucesToLoadMutex);
                            s_queuedResoucesToLoad.push(resource);
                            queueLock.unlock();
                        }
                    }
                    else
                    {
                        FAIL_ASSERT_MSG("[ResourceManager::Load] Maybe this should be done. Maybe when an resource is being loaded from disk it should handle loading memory resources.");
                    }
                }
            }
            return resource;
        }

        TObjectPtr<IResource> ResourceManager::Load(std::string_view filePath)
        {
            std::string_view fileExtension = FileSystem::FileSystem::GetFileExtension(filePath);
            const IResourceLoader* loader = ResourceLoaderRegister::GetLoaderFromExtension(fileExtension);
            if (!loader)
            {
                IS_CORE_WARN("[ResourceManager::LoadSync] loader is null for extension '{}'.", fileExtension);
                return nullptr;
            }

            ResourceId resourceId(filePath, loader->GetResourceTypeId());
            return Load(resourceId);
        }

        void ResourceManager::Unload(ResourceId const& resourceId)
        {
            ASSERT(s_database);

            TObjectPtr<IResource> resource = nullptr;
            if (s_database->HasResource(resourceId))
            {
                resource = s_database->GetResource(resourceId);
            }

            if (resource->IsDependentOnAnotherResource())
            {
                // Resource is dependent on another resource. The owning resource should handle unloading this resource.
                return;
            }

            if (!resource)
            {
                IS_CORE_WARN("[ResourceManager::UnloadResource] The resource '{0}' is not valid (null). The Resource isn't tracked by the ResourceDatabase.", resourceId.GetPath());
                return;
            }

            if (resource->GetResourceState() != EResoruceStates::Loaded)
            {
                IS_CORE_WARN("[ResourceManager::Unload] 'resource' current state is '{0}'. Resource must be loaded to be unloaded."
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

        void ResourceManager::Unload(TObjectPtr<IResource> Resource)
        {
            if (Resource)
            {
                Unload(Resource->m_resourceId);
            }
        }

        void ResourceManager::UnloadAll()
        {
            ASSERT(Platform::IsMainThread());
            ASSERT(s_database);

            std::vector<ResourceId> ResourceIds = s_database->GetAllResourceIds();
            for (const ResourceId& id : ResourceIds)
            {
                Unload(id);
            }
        }

        bool ResourceManager::HasResource(ResourceId const& resourceId)
        {
            ASSERT(s_database);
            return s_database->HasResource(resourceId);
        }

        bool ResourceManager::HasResource(TObjectPtr<IResource> Resource)
        {
            if (Resource)
            {
                return HasResource(Resource->GetResourceId());
            }
            return false;
        }

        ResourceDatabase::ResourceMap ResourceManager::GetResourceMap()
        {
            ASSERT(s_database);
            return s_database->GetResourceMap();
        }

        u32 ResourceManager::GetQueuedToLoadCount()
        {
            u32 count = 0;
            {
                std::lock_guard lock(s_queuedResoucesToLoadMutex);
                count = static_cast<u32>(s_queuedResoucesToLoad.size());
            }
            return count;
        }

        u32 ResourceManager::GetLoadedResourcesCount()
        {
            ASSERT(s_database);
            return s_database->GetLoadedResourceCount();
        }

        u32 ResourceManager::GetLoadingCount()
        {
            ASSERT(s_database);
            return s_database->GetLoadingResourceCount();
        }

        void ResourceManager::StartLoading(IResource* resource, bool threading)
        {
            s_resourcesLoading.push_back(resource);
            resource->m_resource_state = EResoruceStates::Loading;

            const IResourceLoader* loader = ResourceLoaderRegister::GetLoaderFromResource(resource);
            if (!loader)
            {
                IS_CORE_WARN("[ResourceManager::StartLoading] Resource '{}' failed to load as no loader could be found.", resource->GetFileName());
            }

            if (threading)
            {
                Threading::TaskSystem::CreateTask([resource, loader]()
                    {
                        resource->StartLoadTimer();
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
                                    // Something has gone wrong when tring to load the resource.
                                    resource->m_resource_state = EResoruceStates::Failed_To_Load;
                                }
                            }
                            else
                            {
                                IS_CORE_WARN("[ResourceManager::StartLoading] Resource has 'Load' called for type '{}'. This should be replaced by a ResourceLoader.", 
                                    resource->GetResourceTypeId().GetTypeName());
                                resource->Load();
                            }
                        }
                        resource->StopLoadTimer();
                        if (resource->IsLoaded())
                        {
                            resource->OnLoaded(resource);
                        }
                    });
            }
            else
            {
                resource->StartLoadTimer();
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
                            // Something has gone wrong when tring to load the resource.
                            resource->m_resource_state = EResoruceStates::Failed_To_Load;
                        }
                    }
                    else
                    {
                        IS_CORE_WARN("[ResourceManager::StartLoading] Resource has 'Load' called for type '{}'. This should be replaced by a ResourceLoader.",
                            resource->GetResourceTypeId().GetTypeName());
                        resource->Load();
                    }
                }
                resource->StopLoadTimer();
                if (resource->IsLoaded())
                {
                    resource->OnLoaded(resource);
                }
            }
        }

        void ResourceManager::Shutdown()
        {
            ASSERT(Platform::IsMainThread());

            // Pop all queued resources.
            {
                std::lock_guard queueLock(s_queuedResoucesToLoadMutex);
                while (!s_queuedResoucesToLoad.empty())
                {
                    auto resource = s_queuedResoucesToLoad.front();
                    s_queuedResoucesToLoad.pop();
                    resource->m_resource_state = EResoruceStates::Cancelled;
                }
            }

            // Finish loading all resources. This allows us to release them correctly.
            while (!s_resourcesLoading.empty())
            {
                Update(0.16f);
            }
        }

        TObjectPtr<IResource> ResourceManager::CreateDependentResource(ResourceId const& resourceId)
        {
            ASSERT(s_database);
            return s_database->CreateDependentResource(resourceId);
        }

        void ResourceManager::RemoveDependentResource(ResourceId const& resourceId)
        {
            ASSERT(s_database);
            return s_database->RemoveDependentResource(resourceId);
        }
    }
}