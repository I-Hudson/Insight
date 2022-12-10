#include  "EditorWindows/ResourceWindow.h"
#include "Resource/Resource.h"

#include <imgui.h>

namespace Insight
{
	namespace Editor
	{
		ResourceWindow::ResourceWindow()
			: IEditorWindow()
		{ }

		ResourceWindow::ResourceWindow(u32 minWidth, u32 minHeight)
			: IEditorWindow(minWidth, minHeight)
		{ }

		ResourceWindow::ResourceWindow(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight)
			: IEditorWindow(minWidth, minHeight, maxWidth, maxHeight)
		{ }

		ResourceWindow::~ResourceWindow()
		{ }

		void ResourceWindow::OnDraw()
		{
			ImGui::Text("Total resources loaded: %i", Runtime::ResourceManager::Instance().GetLoadedResourcesCount());
			ImGui::Text("Total resources loading: %i", Runtime::ResourceManager::Instance().GetLoadingCount());
		}
	}
}