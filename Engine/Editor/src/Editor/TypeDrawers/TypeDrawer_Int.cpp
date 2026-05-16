#include "Editor/TypeDrawers/TypeDrawer_Int.h"

#include <imgui.h>

namespace Insight
{
    namespace Editor
    {
        std::string TypeDrawer_Int::GetTypeName()
        {
            return Reflect::Util::GetTypeName<int>();
        }

        void TypeDrawer_Int::Draw(void* data, const Reflect::MemberInfo& memberInfo, const std::string_view label) const
        {
            Reflect::PropertyMeta clampMin = memberInfo.GetMeta("ClampMin");
            Reflect::PropertyMeta clampMax = memberInfo.GetMeta("ClampMax");

            int* fData = static_cast<int*>(data);
            ImGui::DragInt(label.data(), fData, 1.0f, 
                clampMin.IsValid() ? clampMin.GetValue<int>() : 0,
                clampMax.IsValid() ? clampMax.GetValue<int>() : 0);

            if (clampMin.IsValid())
            {
                *fData = std::max(*fData, clampMin.GetValue<int>());
            }
            if (clampMax.IsValid())
            {
                *fData = std::min(*fData, clampMax.GetValue<int>());
            }
        }
    }
}