#include "ispch.h"
#include "CameraComponentEditorDrawer.h"
#include "Insight/Component/CameraComponent.h"

#include <glm/gtx/euler_angles.hpp>

#if defined(IS_EDITOR)
namespace Insight
{
	namespace Editor
	{
		void CameraComponentEditorDrawer::OnDraw(Object& obj)
		{
			CameraComponent component = static_cast<CameraComponent&>(obj);

			glm::mat4 view = component.GetViewMatrix();
			float position[4] = { view[3].x, view[3].y, view[3].z, 1.0f };
			if (UIHelper::DrawVector("Position", 3, position))
			{
				view[3].x = position[0];
				view[3].z = position[1];
				view[3].z = position[2];
				component.SetViewMatrix(view);
			}
		}
	}
}
#endif