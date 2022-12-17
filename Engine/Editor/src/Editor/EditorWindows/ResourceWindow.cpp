#include "Editor/EditorWindows/ResourceWindow.h"
#include "Resource/Resource.h"
#include "Resource/ResourceManager.h"

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
			ImGui::Text("Total resources loaded: %i", Runtime::ResourceManagerExt::GetLoadedResourcesCount());
			ImGui::Text("Total resources loading: %i", Runtime::ResourceManagerExt::GetLoadingCount());

			ImGui::Text("Resoruces:");
			Runtime::ResourceDatabase::ResourceMap resources = Runtime::ResourceManagerExt::GetResourceMap();
			for (const auto& pair : resources)
			{
				DrawSingleResource(pair.second);
			}
		}

		void ResourceWindow::DrawSingleResource(Runtime::IResource* resource)
		{
			static constexpr ImVec4 resourceStateColours[] =
			{
				ImVec4(1, 1, 0, 1),	    // Not found
				ImVec4(0, 1, 0, 1),	    // Loaded
				ImVec4(0, 0.5f, 0, 1),	// Loading
				ImVec4(1, 0, 1, 1),	    // Not loaded
				ImVec4(1, 0, 0, 1),	    // Failed to load
				ImVec4(0, 0, 1, 1),	    // Unloaded
				ImVec4(0, 0, 0.5f, 1),	// Unloading
			};

			std::string fileName = resource->GetFileName();
			std::string resourceState = Runtime::ERsourceStatesToString(resource->GetResourceState());
			float loadTime = resource->GetLoadTimer().GetElapsedTimeMillFloat();
			ImGui::PushStyleColor(ImGuiCol_Header, resourceStateColours[static_cast<int>(resource->GetResourceState())]);
			if (ImGui::TreeNodeEx(static_cast<const void*>(resource), ImGuiTreeNodeFlags_Framed, "%s", fileName.c_str()))
			{
				ImGui::Text("State: %s, LoadTime: %f", resourceState.c_str(), loadTime);
				if (ImGui::Button("Load"))
				{
					Runtime::ResourceManagerExt::Load(Runtime::ResourceId(resource->GetFilePath(), resource->GetResourceTypeId()));
				}
				if (ImGui::Button("Unload"))
				{
					Runtime::ResourceManagerExt::Unload(resource->GetResourceId());
				}
				ImGui::TreePop();
			}
			ImGui::PopStyleColor();
		}
	}
}