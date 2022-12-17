#pragma once

#include "Core/Memory.h"

#include "Runtime/Defines.h"
#include "Resource/Resource.h"

#include <mutex>

namespace Insight
{
    namespace Runtime
    {
        class ResourceManagerExt;

        /// @brief Database to store all currently known resources.
        class ResourceDatabase
        {
            using ResouceMap = std::unordered_map<ResourceId, TObjectPtr<IResource>>;
            using ResouceOwningMap = std::unordered_map<ResourceId, TObjectOPtr<IResource>>;
        public:
            ResourceDatabase() = default;
            ~ResourceDatabase() = default;

            void Initialise();
            void Shutdown();

            TObjectPtr<IResource> AddResouce(ResourceId resourceId);
            void RemoveResource(TObjectPtr<IResource> resource);
            void RemoveResource(ResourceId resourceId);

            TObjectPtr<IResource> GetResource(ResourceId resourceId) const;
            ResouceMap GetResouceMap() const;
            
            bool HasResource(ResourceId resourceId) const;
            bool HasResource(TObjectPtr<IResource> resource) const;

            u32 GetResourceCount() const;
            u32 GetLoadedResourceCount() const;

        private:
            void DeleteResource(TObjectOPtr<IResource>& resource);

            std::vector<ResourceId> GetAllResouceIds() const;
            void OnResouceLoaded(IResource* resouce);
            void OnResouceUnloaded(IResource* resouce);

        private:
            ResouceOwningMap m_resources;
            u32 m_loadedResourceCount = 0;
            mutable std::mutex m_mutex;

            friend class ResourceManagerExt;
        };
    }
}