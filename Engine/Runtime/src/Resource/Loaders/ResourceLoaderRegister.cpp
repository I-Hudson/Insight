#include "Resource/Loaders/ResourceLoaderRegister.h"
#include "Resource/Loaders/IResourceLoader.h"

#include "Algorithm/Vector.h"

namespace Insight
{
    namespace Runtime
    {
        std::vector<IResourceLoader*> ResourceLoaderRegister::s_resourceLoaders;

        ///===============================================================================
        /// ResourceLoaderRegister
        ///===============================================================================
        ResourceLoaderRegister::ResourceLoaderRegister()
        {
        }

        ResourceLoaderRegister::~ResourceLoaderRegister()
        {
        }

        void ResourceLoaderRegister::Initialise()
        {
        }

        void ResourceLoaderRegister::Shutdown()
        {
        }

        const IResourceLoader* ResourceLoaderRegister::GetLoaderFromExtension(std::string_view fileExtension)
        {
            for (size_t i = 0; i < s_resourceLoaders.size(); ++i)
            {
                const IResourceLoader* loader = s_resourceLoaders.at(i);
                if (Algorithm::VectorContains(loader->GetAcceptedFileExtensions(), fileExtension))
                {
                    return loader;
                }
            }
            return nullptr;
        }
    }
}