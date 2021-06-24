#pragma once
#include "Engine/Core/Core.h"
#include "Editor/EditorDrawer.h"
#include "Engine/Component/TransformComponent.h"

#if defined(IS_EDITOR)
namespace Insight::Editor
{
	class TransformComponentEditorDrawer : public IEditorDrawer
	{
	public:
		virtual void OnDraw(Object& obj) override;

	private:
		EDITOR_DEC_TYPE(TransformComponent, TransformComponentEditorDrawer);
	};
}
#endif