#include "Editor/TypeDrawers/TypeDrawer_Float.h"

#include <imgui/imgui.h>

namespace Insight
{
    namespace Editor
    {
        std::string TypeDrawer_Float::GetTypeName()
        {
            return "float";
        }

        void TypeDrawer_Float::Draw(void* data, const std::string_view label) const
        {
            float* fData = static_cast<float*>(data);
            ImGui::DragFloat(label.data(), fData);
        }
    }
}