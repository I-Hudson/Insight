#include "Resource/Loaders/IResourceLoader.h"

namespace Insight
{
    namespace Runtime
    {
        ///===============================================================================
        /// IResourceLoader
        ///===============================================================================
        IResourceLoader::IResourceLoader(std::vector<const char*> acceptedFileExtensions, std::vector<ResourceTypeId> loadableResourceTypeIds)
            : m_acceptedFileExtensions(std::move(acceptedFileExtensions))
            , m_loadableResourceTypeIds(std::move(loadableResourceTypeIds))
        {
            ASSERT(m_acceptedFileExtensions.size() > 0);
            ASSERT(m_loadableResourceTypeIds.size() > 0);
        }

        IResourceLoader::~IResourceLoader()
        {
        }

        const std::vector<const char*>& IResourceLoader::GetAcceptedFileExtensions() const
        {
            return m_acceptedFileExtensions;
        }

        bool IResourceLoader::HasSingleLoadableResourceTypeId() const
        {
            return m_loadableResourceTypeIds.size() == 1;
        }

        u32 IResourceLoader::GetResourceTypeIdSize() const
        {
            return static_cast<u32>(m_loadableResourceTypeIds.size());
        }

        ResourceTypeId IResourceLoader::GetResourceTypeId() const
        {
            if (m_loadableResourceTypeIds.size() > 0)
            {
                return m_loadableResourceTypeIds.at(0);
            }
        }

        ResourceTypeId IResourceLoader::GetResourceTypeId(u32 index) const
        {
            if (index < m_loadableResourceTypeIds.size())
            {
                return m_loadableResourceTypeIds.at(index);
            }
           FAIL_ASSERT_MSG("[IResourceLoader::GetResourceTypeId] Out of bounds container size '{}' index requested '{}'.", m_loadableResourceTypeIds.size(), index);
           return {};
        }
    }
}