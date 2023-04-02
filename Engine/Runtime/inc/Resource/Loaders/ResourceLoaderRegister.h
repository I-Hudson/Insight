#pragma once

#include "Runtime/Defines.h"

#include <vector>
#include <string_view>

namespace Insight
{
    namespace Runtime
    {
        class IResourceLoader;

        class IS_RUNTIME ResourceLoaderRegister
        {
        public:
            ResourceLoaderRegister();
            ~ResourceLoaderRegister();

            static void Initialise();
            static void Shutdown();

            static const IResourceLoader* GetLoaderFromExtension(std::string_view fileExtension);

        private:
            static std::vector<IResourceLoader*> s_resourceLoaders;
        };
    }
}