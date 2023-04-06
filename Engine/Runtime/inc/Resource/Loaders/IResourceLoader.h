#pragma once

#include "Runtime/Defines.h"
#include "Resource/ResourceTypeId.h"

#include <vector>
#include <string_view>

namespace Insight
{
    namespace Runtime
    {
        class IResource;

        class IS_RUNTIME IResourceLoader
        {
        public:
            IResourceLoader() = delete;
            IResourceLoader(std::vector<const char*> acceptedFileExtensions, std::vector<ResourceTypeId> loadableResourceTypeIds);
            virtual ~IResourceLoader();

            const std::vector<const char*>& GetAcceptedFileExtensions() const;
            /// @brief Return true is this loader can load a single type of resource, otherwise return false.
            /// Example: Model loader would only be able to load a model file while Texture load could load Texture2D,
            /// or Cubemap.
            /// @return bool 
            bool HasSingleLoadableResourceTypeId() const;
            u32 GetResourceTypeIdSize() const;
            const std::vector<ResourceTypeId>& GetLoadableResourceTypes() const;

            /// @brief Return the first ResourceTypeId.
            /// @return ResourceTypeId
            ResourceTypeId GetResourceTypeId() const;
            ResourceTypeId GetResourceTypeId(u32 index) const;

            virtual void Initialise() { }
            virtual void Shutdown() { }

            virtual bool Load(IResource* resource) const = 0;

        private:
            std::vector<const char*> m_acceptedFileExtensions;
            std::vector<ResourceTypeId> m_loadableResourceTypeIds;
        };
    }
}