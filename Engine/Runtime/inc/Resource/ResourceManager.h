#pragma once

#include "Runtime/Defines.h"

#include "Resource/ResourceDatabase.h"

#include <queue>

namespace Insight
{
    namespace Runtime
    {
        class ResourceSystem;

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

            static TObjectPtr<IResource> Load(ResourceId const& resourceId);
            static void Unload(ResourceId const& resourceId);
            static void Unload(TObjectPtr<IResource> Resource);
            static void UnloadAll();

            static bool HasResource(ResourceId const& resourceId);
            static bool HasResource(TObjectPtr<IResource> Resource);

            static ResourceDatabase::ResourceMap GetResourceMap();
            static u32 GetQueuedToLoadCount();
            static u32 GetLoadedResourcesCount();
            static u32 GetLoadingCount();

        private:
            static void Update(float const deltaTime);
            static void StartLoading(IResource* resource);

        private:
            /// @brief Max number of resources which can be loading at a single time is 4. This
            // is to not fill up the task system.
            const static u32 c_MaxLoadingResources = -1;
            static ResourceDatabase* s_database;

            /// @brief Current resources being loaded.
            static std::vector<IResource*> s_resourcesLoading;
            /// @brief Resources queued to be loaded.
            static std::queue<IResource*> s_queuedResoucesToLoad;
            static std::mutex s_queuedResoucesToLoadMutex;

            friend class ResourceSystem;
        };
    }
}