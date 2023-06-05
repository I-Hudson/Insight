#include "Editor/TypeDrawers/TypeDrawer_GLMMat4.h"

#include <glm/glm.hpp>
#include <imgui.h>

namespace Insight
{
    namespace Editor
    {
        std::string TypeDrawer_GLMMat4::GetTypeName()
        {
            std::string typeName = typeid(glm::mat4).name();
            typeName = RemoveString(typeName, "class");
            typeName = RemoveString(typeName, "struct");
            return typeName;
        }

        void TypeDrawer_GLMMat4::Draw(void* data) const
        {
#define ARRAY_TO_GLM(row, array) mat4[row][0] = array[0]; mat4[row][1] = array[1]; mat4[row][2] = array[2]; mat4[row][3] = array[3];

            glm::mat4* mat4Ptr = static_cast<glm::mat4*>(data);
            glm::mat4& mat4 = *mat4Ptr;

            float x[4] = { mat4[0][0], mat4[0][1], mat4[0][2], mat4[0][3] };
            ImGui::DragFloat4("X", x);

            float y[4] = { mat4[1][0], mat4[1][1], mat4[1][2], mat4[1][3] };
            ImGui::DragFloat4("Y", y);

            float z[4] = { mat4[2][0], mat4[2][1], mat4[2][2], mat4[3][3] };
            ImGui::DragFloat4("Z", z);

            float w[4] = { mat4[3][0], mat4[3][1], mat4[3][2], mat4[3][3] };
            ImGui::DragFloat4("W", w);

            ARRAY_TO_GLM(0, x);
            ARRAY_TO_GLM(1, y);
            ARRAY_TO_GLM(2, z);
            ARRAY_TO_GLM(3, w);
#undef ARRAY_TO_GLM
        }
    }
}