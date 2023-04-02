#include "Resource/Loaders/IResourceLoader.h"

namespace Insight
{
    namespace Runtime
    {
        ///===============================================================================
        /// IResourceLoader
        ///===============================================================================
        IResourceLoader::IResourceLoader(std::vector<const char*> acceptedFileExtensions)
            : m_acceptedFileExtensions(std::move(acceptedFileExtensions))
        { }

        IResourceLoader::~IResourceLoader()
        {
        }

        const std::vector<const char*>& IResourceLoader::GetAcceptedFileExtensions() const
        {
            return m_acceptedFileExtensions;
        }
    }
}