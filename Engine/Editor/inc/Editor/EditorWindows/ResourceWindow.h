#pragma once

#include "Editor/EditorWindows/IEditorWindow.h"
#include "Editor/EditorWindows/Generated/ResourceWindow_reflect_generated.h"

namespace Insight
{
	namespace Runtime
	{
		class IResource;
		class ResourcePack;
	}

	namespace Editor
	{
		REFLECT_CLASS()
		class ResourceWindow : public IEditorWindow
		{
			REFLECT_GENERATED_BODY()

		public:
			ResourceWindow();
			ResourceWindow(u32 minWidth, u32 minHeight);
			ResourceWindow(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight);
			~ResourceWindow();

			virtual void OnDraw() override;
			EDITOR_WINDOW(ResourceWindow, EditorWindowCategories::Windows);

		private:
			void DrawSingleResource(Runtime::IResource* resource);
			void DrawSingleResourcePack(Runtime::ResourcePack* resourcePack);

		};
	}
}