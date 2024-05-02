#include "Editor/TypeDrawers/TypeDrawer_Maths.h"

#include "Maths/Vector2.h"
#include "Maths/Vector3.h"
#include "Maths/Vector4.h"

#include <imgui/imgui.h>

namespace Insight
{
	namespace Editor
	{
		std::string TypeDrawer_Vector2::GetTypeName()
		{
			return Reflect::Util::GetTypeName<Maths::Vector2>();
		}
		
		void TypeDrawer_Vector2::Draw(void* data, const std::string_view label) const
		{
			Maths::Vector2* vec2 = static_cast<Maths::Vector2*>(data);
			ImGui::DragFloat2(label.data(), vec2->data);
		}

		std::string TypeDrawer_Vector3::GetTypeName()
		{
			return Reflect::Util::GetTypeName<Maths::Vector3>();
		}

		void TypeDrawer_Vector3::Draw(void* data, const std::string_view label) const
		{
			Maths::Vector3* vec3 = static_cast<Maths::Vector3*>(data);
			ImGui::DragFloat3(label.data(), vec3->data);
		}

		std::string TypeDrawer_Vector4::GetTypeName()
		{
			return Reflect::Util::GetTypeName<Maths::Vector4>();
		}
		
		void TypeDrawer_Vector4::Draw(void* data, const std::string_view label) const
		{
			Maths::Vector4* vec4 = static_cast<Maths::Vector4*>(data);
			ImGui::DragFloat4(label.data(), vec4->data);
		}
	}
}