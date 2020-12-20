#include "ispch.h"
#include "CameraComponentEditorDrawer.h"
#include "Insight/Component/CameraComponent.h"

#include <glm/gtx/euler_angles.hpp>

#if defined(IS_EDITOR)
namespace Insight
{
	namespace Editor
	{
		void CameraComponentEditorDrawer::OnDraw(SharedPtr<Object> obj)
		{
			SharedPtr<CameraComponent> component = DynamicPointerCast<CameraComponent>(obj);

			glm::mat4 view = component->GetViewMatrix();
			float position[3] = { view[3].x, view[3].y, view[3].z };
			if (UIHelper::DrawVector("Position", 3, position))
			{
				view[3].x = position[0];
				view[3].y = position[1];
				view[3].z = position[2];
				component->SetViewMatrix(view);
			}

			float fov = component->GetFov();
			if (UIHelper::DrawFloat("FOV", &fov))
			{
				component->SetFov(fov);
			}
		}
	}
}
#endif