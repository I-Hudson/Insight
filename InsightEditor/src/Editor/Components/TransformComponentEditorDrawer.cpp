#include "ispch.h"
#include "Editor/Components/TransformComponentEditorDrawer.h"

EDITOR_DEF_TYPE(TransformComponent, Insight::Editor::TransformComponentEditorDrawer);

namespace Insight::Editor
{
	void TransformComponentEditorDrawer::OnDraw(Object& obj)
	{
		TransformComponent& component = dynamic_cast<TransformComponent&>(obj);

		glm::vec3 position = component.GetPostion();
		if (UIHelper::DrawVector("Position", 3, &position[0]))
		{
			component.SetPosition(position);
		}
		UIHelper::Space();

		glm::vec3 rotation = component.GetRotation();
		if (UIHelper::DrawVector("Rotation", 3, &rotation[0]))
		{
			//component.SetRotation(rotation);
		}
		UIHelper::Space();

		glm::vec3 scale = component.GetScale();
		if (UIHelper::DrawVector("Scale", 3, &scale[0]))
		{
			//component.SetScale(scale);
		}
		UIHelper::Space();
	}
}