#pragma once

#include "Editor/EditorWindows/IEditorWindow.h"

#include "Generated/ShaderWindow_reflect_generated.h"

namespace Insight
{
	namespace Editor
	{
		REFLECT_CLASS()
			class ShaderWindow : public IEditorWindow
		{
			REFLECT_GENERATED_BODY()
		public:
			ShaderWindow();
			ShaderWindow(u32 minWidth, u32 minHeight);
			ShaderWindow(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight);
			~ShaderWindow();

			virtual void OnDraw() override;

			EDITOR_WINDOW(ShaderWindow, EditorWindowCategories::Windows);
		};
	}
}