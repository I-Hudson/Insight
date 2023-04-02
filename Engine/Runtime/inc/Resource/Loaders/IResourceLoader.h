#pragma once

#include "Runtime/Defines.h"

#include <vector>

namespace Insight
{
    namespace Runtime
    {
        class IResource;

        class IResourceLoader
        {
        public:
            IResourceLoader(std::vector<const char*> acceptedFileExtensions);
            virtual ~IResourceLoader();

            const std::vector<const char*>& GetAcceptedFileExtensions() const;

            virtual void Initialise() = 0;
            virtual void Shutdown() = 0;

            virtual IResource* LoadFromFile(std::string_view filePath) const = 0;

        private:
            std::vector<const char*> m_acceptedFileExtensions;
        };
    }
}