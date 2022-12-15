#pragma once

#include "Editor/EditorWindows/IEditorWindow.h"

namespace Insight
{
	namespace Runtime
	{
		class IResource;
	}
	namespace Editor
	{
		class ResourceWindow : public IEditorWindow
		{
		public:
			ResourceWindow();
			ResourceWindow(u32 minWidth, u32 minHeight);
			ResourceWindow(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight);
			~ResourceWindow();

			virtual void OnDraw() override;
			EDITOR_WINDOW(ResourceWindow);

		private:
			void DrawSingleResource(Runtime::IResource* resource);

		};
	}
}