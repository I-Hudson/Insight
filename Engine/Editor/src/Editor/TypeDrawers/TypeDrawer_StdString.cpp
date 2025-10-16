#include "Editor/TypeDrawers/TypeDrawer_StdString.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

namespace Insight
{
    namespace Editor
    {
        std::string TypeDrawer_StdString::GetTypeName()
        {
            return Reflect::Util::GetTypeName<std::string>();
        }

        void TypeDrawer_StdString::Draw(void* data, const Reflect::MemberInfo& memberInfo, const std::string_view label) const
        {
            std::string* fData = static_cast<std::string*>(data);
            ImGui::InputText(label.data(), fData);
        }
    }
}