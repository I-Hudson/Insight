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
        public:
            using ResourceMap = std::unordered_map<ResourceId, TObjectPtr<IResource>>;
            using ResourceOwningMap = std::unordered_map<ResourceId, TObjectOPtr<IResource>>;

            ResourceDatabase() = default;
            ~ResourceDatabase() = default;

            void Initialise();
            void Shutdown();

            TObjectPtr<IResource> AddResource(ResourceId const& resourceId);
            void RemoveResource(TObjectPtr<IResource> resource);
            void RemoveResource(ResourceId const& resourceId);

            TObjectPtr<IResource> GetResource(ResourceId const& resourceId) const;
            ResourceMap GetResourceMap() const;
            
            bool HasResource(ResourceId const& resourceId) const;
            bool HasResource(TObjectPtr<IResource> resource) const;

            u32 GetResourceCount() const;
            u32 GetLoadedResourceCount() const;
            u32 GetLoadingResourceCount() const;

        private:
            void DeleteResource(TObjectOPtr<IResource>& resource);

            std::vector<ResourceId> GetAllResourceIds() const;
            void OnResourceLoaded(IResource* Resource);
            void OnResourceUnloaded(IResource* Resource);

        private:
            ResourceOwningMap m_resources;
            u32 m_loadedResourceCount = 0;
            mutable std::mutex m_mutex;

            friend class ResourceManagerExt;
        };
    }
}