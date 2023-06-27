#pragma once

#include "Runtime/Defines.h"

#include "Resource/ResourceDatabase.h"

#include <queue>

namespace Insight
{
    namespace Runtime
    {
        class ResourceSystem;
        class AssimpLoader;

        /// @brief Static class used to interact with resources.
        class IS_RUNTIME ResourceManager
        {
        public:

            static void SaveDatabase();
            static void LoadDatabase();
            static void ClearDatabase();

            /// @brief Create a resource reference, but don't load it.
            /// @param resourceId 
            /// @return TObjectPtr<IResource>
            static TObjectPtr<IResource> Create(ResourceId const& resourceId);

            static TObjectPtr<IResource> GetResource(ResourceId const& resourceId);
            static TObjectPtr<IResource> GetResourceFromGuid(Core::GUID const& guid);

            static TObjectPtr<IResource> LoadSync(ResourceId resourceId);
            static TObjectPtr<IResource> LoadSync(ResourceId resourceId, bool convertToEngineFormat);

            /// @brief Legacy function to be removed in later versions.
            /// @param filepath 
            /// @param convertToEngineFormat 
            /// @return TObjectPtr<IResource>
            static TObjectPtr<IResource> LoadSync(std::string_view filepath, bool convertToEngineFormat);

            static TObjectPtr<IResource> Load(ResourceId resourceId);
            static TObjectPtr<IResource> Load(ResourceId resourceId, bool convertToEngineFormat);

            static void Unload(ResourceId const& resourceId);
            static void Unload(TObjectPtr<IResource> Resource);
            static void UnloadAll();

            static bool HasResource(ResourceId const& resourceId);
            static bool HasResource(TObjectPtr<IResource> Resource);

            static ResourceDatabase::ResourceMap GetResourceMap();
            static u32 GetQueuedToLoadCount();
            static u32 GetLoadedResourcesCount();
            static u32 GetLoadingCount();

            static TObjectPtr<IResource> CreateDependentResource(ResourceId const& resourceId);
            static void RemoveDependentResource(ResourceId const& resourceId);

            static std::string GetMetaPath(const IResource* resource);

        private:
            static void Update(float const deltaTime);
            static void StartLoading(IResource* resource, bool threading);
            static void Shutdown();

            /// @brief Convert a resource from disk to an engine format resource.
            /// @param resourceId 
            /// @return ResourceId
            static ResourceId ConvertResource(ResourceId resourceId);

            static void RemoveResource(ResourceId resourceId);

        private:
            /// @brief Max number of resources which can be loading at a single time is 4. This
            // is to not fill up the task system.
            const static u32 c_MaxLoadingResources = -1;
            static ResourceDatabase* s_database;

            /// @brief Current resources being loaded.
            static std::vector<IResource*> s_resourcesLoading;
            static std::mutex s_resourcesLoadingMutex;

            /// @brief Resources queued to be loaded.
            static std::queue<IResource*> s_queuedResoucesToLoad;
            static std::mutex s_queuedResoucesToLoadMutex;

            friend class ResourceSystem;
            friend class AssimpLoader;
        };
    }
}