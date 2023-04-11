#pragma once

#include "Editor/EditorWindows/IEditorWindow.h"

namespace Insight
{
	namespace Input
	{
		class IInputDevice;
	}

	namespace Editor
	{
		REFLECT_CLASS()
		class InputWindow : public IEditorWindow
		{
		public:
			InputWindow();
			InputWindow(u32 minWidth, u32 minHeight);
			InputWindow(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight);
			~InputWindow();

			virtual void OnDraw() override;

			EDITOR_WINDOW(InputWindow, EditorWindowCategories::Windows);

		private:
			void DrawSingleInputDevuce(Input::IInputDevice const* inputDevice);
		};
	}
}