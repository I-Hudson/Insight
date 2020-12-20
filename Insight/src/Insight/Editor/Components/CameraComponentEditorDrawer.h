#pragma once
#include "Insight/Core/Core.h"
#include "Insight/Editor/EditorDrawer.h"
#include "Insight/Component/CameraComponent.h"

#if defined(IS_EDITOR)
namespace Insight
{
	namespace Editor
	{
		class CameraComponentEditorDrawer : public TEditorDrawer<CameraComponent, CameraComponentEditorDrawer>
		{
		public:
			virtual void OnDraw(SharedPtr<Object> obj) override;
		};
	}
}
#endif