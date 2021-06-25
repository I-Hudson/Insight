#include "Editor/Components/TransformComponentEditorDrawer.h"
#include "Engine/Core/Maths/Math.h"
#include "glm/gtc/type_ptr.hpp"

EDITOR_DEF_TYPE(TransformComponent, Insight::Editor::TransformComponentEditorDrawer);

namespace Insight::Editor
{
	void TransformComponentEditorDrawer::OnDraw(Object& obj)
	{
		TransformComponent& component = dynamic_cast<TransformComponent&>(obj);

		glm::vec3 position = component.GetPostion();
		if (UIHelper::DrawVector("Position", 3, glm::value_ptr(position)))
		{
			component.SetPosition(position);
		}
		UIHelper::Space();

		glm::vec3 rotation = Maths::RadiansToDegreesVector(component.GetRotation());
		if (UIHelper::DrawVector("Rotation", 3, glm::value_ptr(rotation)))
		{
			component.SetRotation(Maths::DegreesToRadiansVector(rotation));
		}
		UIHelper::Space();

		glm::vec3 scale = component.GetScale();
		if (UIHelper::DrawVector("Scale", 3, glm::value_ptr(scale)))
		{
			component.SetScale(scale);
		}
		UIHelper::Space();
	}
}