#include "Editor/TypeDrawers/TypeDrawer_Float.h"

#include <imgui.h>

namespace Insight
{
    namespace Editor
    {
        std::string TypeDrawer_Float::GetTypeName()
        {
            return "float";
        }

        void TypeDrawer_Float::Draw(void* data, const Reflect::MemberInfo& memberInfo, const std::string_view label) const
        {
            Reflect::PropertyMeta clampMin = memberInfo.GetMeta("ClampMin");
            Reflect::PropertyMeta clampMax = memberInfo.GetMeta("ClampMax");

            float* fData = static_cast<float*>(data);
            ImGui::DragFloat(label.data(), fData, 1.0f, 
                clampMin.IsValid() ? clampMin.GetValue<float>() : 0.0f,
                clampMax.IsValid() ? clampMax.GetValue<float>() : 0.0f);
        }
    }
}