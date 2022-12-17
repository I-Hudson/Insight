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

            static TObjectPtr<IResource> Load(ResourceId resourceId);
            static void Unload(ResourceId resourceId);
            static void Unload(TObjectPtr<IResource> resouce);
            static void UnloadAll();

            static ResourceDatabase::ResouceMap GetResouceMap();

        private:
            static ResourceDatabase* m_database;

            friend class ResourceSystem;
        };
    }
}