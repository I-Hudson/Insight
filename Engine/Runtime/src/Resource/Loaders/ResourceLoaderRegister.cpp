#include "Resource/Loaders/ResourceLoaderRegister.h"
#include "Resource/Loaders/IResourceLoader.h"
#include "Resource/Loaders/AssimpLoader.h"

#include "Resource/Resource.h"

#include "FileSystem/FileSystem.h"

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
            ASSERT(Platform::IsMainThread());
            RegisterResourceLoader(New<AssimpLoader>());
        }

        void ResourceLoaderRegister::Shutdown()
        {
            ASSERT(Platform::IsMainThread());
            for (size_t i = 0; i < s_resourceLoaders.size(); ++i)
            {
                s_resourceLoaders.at(i)->Shutdown();
                Delete(s_resourceLoaders.at(i));
            }
            s_resourceLoaders.resize(0);
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
            IS_CORE_WARN("[ResourceLoaderRegister::GetLoaderFromExtension] No loader was found for extension of '{}'.", fileExtension);
            return nullptr;
        }

        const IResourceLoader* ResourceLoaderRegister::GetLoaderFromResource(const IResource* resource)
        {
            if (!resource)
            {
                return nullptr;
            }

            std::string filePath = resource->GetFilePath();
            std::string_view fileExtension = FileSystem::FileSystem::GetFileExtension(filePath);
            return GetLoaderFromExtension(fileExtension);
        }

        void ResourceLoaderRegister::RegisterResourceLoader(IResourceLoader* resourceLoader)
        {
            if (Algorithm::VectorContainsIf(s_resourceLoaders, [resourceLoader](const IResourceLoader* loader)
                {
                    return resourceLoader->GetResourceTypeId() == loader->GetResourceTypeId();
                }))
            {
                IS_CORE_ERROR("[ResourceLoaderRegister::RegisterResourceLoader] Trying to add a new loader for ResourceTypeId '{}' when one already exists.", 
                    resourceLoader->GetResourceTypeId().GetTypeName());
                Delete(resourceLoader);
            }
            resourceLoader->Initialise();
            s_resourceLoaders.push_back(resourceLoader);
        }
    }
}