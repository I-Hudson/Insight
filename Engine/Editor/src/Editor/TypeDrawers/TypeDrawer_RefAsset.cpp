#include "Editor/TypeDrawers/TypeDrawer_RefAsset.h"

#include "Core/ReferencePtr.h"
#include "Asset/Asset.h"

#include <Reflect/Core/Util.h>

namespace Insight
{
    namespace Editor
    {
        std::string TypeDrawer_RefAsset::GetTypeName()
        {
            return Reflect::Util::GetValueTypeName<Ref<Runtime::Asset>>();
        }

        void TypeDrawer_RefAsset::Draw(void* data, const Reflect::MemberInfo& memberInfo, const std::string_view label) const
        {

        }
    }
}