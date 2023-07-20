#pragma once

#include "Runtime/Defines.h"
#include "Resource/Loaders/IResourceLoader.h"

#include "Core/Defines.h"
#include "Core/Logger.h"

#include <vector>
#include <string_view>

namespace Insight
{
    namespace Runtime
    {
        class IResource;

        /// @brief 
        class IS_RUNTIME ResourceLoaderRegister
        {
            //This is thread safe as we should only ever be reading never writing (apart from on Initialise and Shutdown).
            THREAD_SAFE
        public:
            ResourceLoaderRegister();
            ~ResourceLoaderRegister();

			/// @brief Initialise the register will all possible loaders, default setup.
            static void Initialise();
			/// @brief Remove all loaders.
            static void Shutdown();

			template<typename ResourceLoaderType>
			static void RegisterResourceLoader()
			{
				static_assert(std::is_base_of_v<IResourceLoader, ResourceLoaderType>);
				ResourceLoaderType* resourceLoader = New<ResourceLoaderType>();
				if (!resourceLoader->GetLoadableResourceTypes().empty() && Algorithm::VectorContainsIf(s_resourceLoaders, [&resourceLoader](const IResourceLoader* loader)
				                                                                                       {
				                                                                                       return resourceLoader->GetResourceTypeId() == loader->GetResourceTypeId();
																									   }))
				{
					IS_CORE_ERROR("[ResourceLoaderRegister::RegisterResourceLoader] Trying to add a new loader for ResourceTypeId '{}' when one already exists.",
					              resourceLoader->GetResourceTypeId().GetTypeName());
					Delete(resourceLoader);
				}
				else
				{
					resourceLoader->Initialise();
					s_resourceLoaders.push_back(resourceLoader);
				}
			}

            static const IResourceLoader* GetLoaderFromExtension(std::string_view fileExtension);
            static const IResourceLoader* GetLoaderFromResource(const IResource* resource);

            /// @brief Check that no two loaders can load the same ResourceTypeId.
            static void VerifyLoaders();

        private:
            static std::vector<IResourceLoader*> s_resourceLoaders;
        };
    }
}