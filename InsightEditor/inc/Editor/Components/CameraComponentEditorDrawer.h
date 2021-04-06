#pragma once
#include "Engine/Core/Core.h"
#include "Editor/EditorDrawer.h"
#include "Engine/Component/CameraComponent.h"

#if defined(IS_EDITOR)
	namespace Editor
	{
		class CameraComponentEditorDrawer : public IEditorDrawer
		{
		public:
			virtual void OnDraw(Object& obj) override;

		private:
			EDITOR_DEC_TYPE(CameraComponent, CameraComponentEditorDrawer);
		};
	}
#endif