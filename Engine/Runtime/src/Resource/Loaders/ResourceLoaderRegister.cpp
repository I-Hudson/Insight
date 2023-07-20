#include "Resource/Loaders/ResourceLoaderRegister.h"
#include "Resource/Loaders/IResourceLoader.h"

#include "Resource/Loaders/ModelLoader.h"
#include "Resource/Loaders/BinaryLoader.h"
#include "Resource/Loaders/TextLoader.h"
#include "Resource/Loaders/TextureLoader.h"

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

            RegisterResourceLoader<ModelLoader>();
            RegisterResourceLoader<BinaryLoader>();
            RegisterResourceLoader<TextLoader>();
            RegisterResourceLoader<TextureLoader>();

            VerifyLoaders();
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
            std::string_view fileExtension = FileSystem::GetFileExtension(filePath);
            return GetLoaderFromExtension(fileExtension);
        }

        void ResourceLoaderRegister::VerifyLoaders()
        {
            std::unordered_set<ResourceTypeId> resourceTypesLoadable;
            for (const IResourceLoader* loader : s_resourceLoaders)
            {
                const std::vector<ResourceTypeId>& loaderResourceTypeIds = loader->GetLoadableResourceTypes();
                for (const ResourceTypeId& typeId : loaderResourceTypeIds)
                {
                    ASSERT_MSG(resourceTypesLoadable.find(typeId) == resourceTypesLoadable.end(), 
                        R"([ResourceLoaderRegister::VerifyLoaders] ResourceTypeId '{}' is already loadable from another loader. 
                            This is not allowed. A ResourceTypeId can only be loaded from a single loader.)");
                    resourceTypesLoadable.insert(typeId);
                }
            }
        }
    }
}