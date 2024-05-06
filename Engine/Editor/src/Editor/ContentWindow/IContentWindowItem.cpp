#include "Editor/ContentWindow/IContentWindowItem.h"

#include "Asset/AssetInfo.h"

namespace Insight
{
    namespace Editor
    {
        IContentWindowItem::IContentWindowItem(std::vector<std::string> fileExtensions)
            : m_fileExtensions(std::move(fileExtensions))
        { }

        void IContentWindowItem::Draw(const Runtime::AssetInfo* assetInfo)
        {

        }

        void IContentWindowItem::AddFileExtensions(std::vector<std::string> fileExtensions)
        {
            std::move(fileExtensions.begin(), fileExtensions.end(), std::back_inserter(m_fileExtensions));
        }
    }
}