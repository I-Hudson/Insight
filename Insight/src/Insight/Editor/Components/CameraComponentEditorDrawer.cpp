#include "ispch.h"
#include "CameraComponentEditorDrawer.h"
#include "Insight/Component/CameraComponent.h"

#if defined(IS_EDITOR)
namespace Insight
{
	namespace Editor
	{
		void CameraComponentEditorDrawer::OnDraw(Object& obj)
		{
			CameraComponent component = static_cast<CameraComponent&>(obj);
		}
	}
}
#endif