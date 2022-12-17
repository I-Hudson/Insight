#pragma once

#include "Runtime/Defines.h"

#include "Resource/ResourceDatabase.h"

namespace Insight
{
    namespace Runtime
    {
        class ResourceSystem;

        /// @brief Static class used to interact with resources.
        class IS_RUNTIME ResourceManagerExt
        {
        public:

            static TObjectPtr<IResource> Load(ResourceId const& resourceId);
            static void Unload(ResourceId const& resourceId);
            static void Unload(TObjectPtr<IResource> resouce);
            static void UnloadAll();

            static bool HasResource(ResourceId const& resourceId);
            static bool HasResource(TObjectPtr<IResource> resouce);

            static ResourceDatabase::ResourceMap GetResourceMap();
            static u32 GetLoadedResourcesCount();
            static u32 GetLoadingCount();

        private:
            static ResourceDatabase* m_database;

            friend class ResourceSystem;
        };
    }
}