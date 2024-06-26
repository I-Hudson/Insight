#include "Editor/EditorWindows/ResourceWindow.h"


#include "FileSystem/FileSystem.h"

#include "Core/Profiler.h"

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
			IS_PROFILE_FUNCTION();

			//ImGui::Text("Resources queued to loading: %i.", Runtime::ResourceManager::Instance().GetQueuedToLoadCount());
			//ImGui::Text("Total resources loading: %i.", Runtime::ResourceManager::Instance().GetLoadingCount());
			//ImGui::Text("Total resources loaded: %i.", Runtime::ResourceManager::Instance().GetLoadedResourcesCount());
			//
			//ImGui::Text("Resources:");
			//Runtime::ResourceDatabase::ResourceMap resources = Runtime::ResourceManager::Instance().GetResourceMap();
			//for (const auto& [resourceId, resource] : resources)
			//{
			//	DrawSingleResource(resource);
			//}
		}

		void ResourceWindow::DrawSingleResource(Runtime::IResource* resource)
		{
			static constexpr ImVec4 resourceStateColours[] =
			{
				ImVec4(1, 1, 0, 1),			// Not found
				ImVec4(0, 0.5f, 0.5f, 1),	// Queued
				ImVec4(0, 1, 0, 1),			// Loaded
				ImVec4(0, 0.5f, 0, 1),		// Loading
				ImVec4(1, 0, 1, 1),			// Not loaded
				ImVec4(1, 0, 0, 1),			// Failed to load
				ImVec4(0, 0, 1, 1),			// Unloaded
				ImVec4(0, 0, 0.5f, 1),		// Unloading
			};

			//std::string fileName = resource->GetFileName();
			//std::string resourceState = Runtime::ERsourceStatesToString(resource->GetResourceState());
			//float loadTime = resource->GetLoadTimer().GetElapsedTimeMillFloat();
			//ImGui::PushStyleColor(ImGuiCol_Header, resourceStateColours[static_cast<int>(resource->GetResourceState())]);
			//if (ImGui::TreeNodeEx(static_cast<const void*>(resource), ImGuiTreeNodeFlags_Framed, "%s", fileName.c_str()))
			//{
			//	ImGui::Text("Guid: %s", resource->GetGuid().ToString().c_str());
			//	ImGui::Text("Resource Type: %s", resource->GetResourceTypeId().GetTypeName().c_str());
			//	ImGui::Text("State: %s, LoadTime: %f.", resourceState.c_str(), loadTime);
			//
			//	if (ImGui::Button("Load"))
			//	{
			//		Runtime::ResourceManager::Instance().LoadSync(Runtime::ResourceId(resource->GetFilePath(), resource->GetResourceTypeId()));
			//	}
			//	if (ImGui::Button("Unload"))
			//	{
			//		Runtime::ResourceManager::Instance().Unload(resource->GetResourceId());
			//	}
			//	ImGui::TreePop();
			//}
			//ImGui::PopStyleColor();
		}
	}
}