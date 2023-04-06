#pragma once

#include "Runtime/Defines.h"
#include "Core/Defines.h"

#include <vector>
#include <string_view>

namespace Insight
{
    namespace Runtime
    {
        class IResourceLoader;
        class IResource;

        /// @brief 
        class IS_RUNTIME ResourceLoaderRegister
        {
            //This is thread safe as we should only ever be reading never writing (apart from on Initialise and Shutdown).
            THREAD_SAFE
        public:
            ResourceLoaderRegister();
            ~ResourceLoaderRegister();

            static void Initialise();
            static void Shutdown();

            static const IResourceLoader* GetLoaderFromExtension(std::string_view fileExtension);
            static const IResourceLoader* GetLoaderFromResource(const IResource* resource);

        private:
            static void RegisterResourceLoader(IResourceLoader* resourceLoader);
            /// @brief Check that no two loaders can load the same ResourceTypeId.
            static void VerifyLoaders();

        private:
            static std::vector<IResourceLoader*> s_resourceLoaders;
        };
    }
}