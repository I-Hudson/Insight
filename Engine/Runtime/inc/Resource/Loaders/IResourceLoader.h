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

        class IResourceLoader
        {
        public:
            IResourceLoader() = delete;
            IResourceLoader(std::vector<const char*> acceptedFileExtensions);
            virtual ~IResourceLoader();

            const std::vector<const char*>& GetAcceptedFileExtensions() const;

            virtual void Initialise() { }
            virtual void Shutdown() { }

            virtual void Load(IResource* resource) const = 0;
            virtual ResourceTypeId GetResourceTypeId() const = 0;

        private:
            std::vector<const char*> m_acceptedFileExtensions;
        };
    }
}