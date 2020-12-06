#pragma once
#include "Insight/Core.h"
#include "Insight/Editor/EditorDrawer.h"

#if defined(IS_EDITOR)
namespace Insight
{
	namespace Editor
	{
		class CameraComponentEditorDrawer : public EditorDrawer
		{
		public:
			virtual void OnDraw(Object& obj) override;
		};
	}
}
#endif