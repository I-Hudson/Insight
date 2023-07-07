#include "Editor/EditorWindows/ResourceWindow.h"
#include "Resource/ResourceManager.h"
#include "Resource/Resource.h"
#include "Resource/ResourcePack.h"

#include "FileSystem/FileSystem.h"

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
			ImGui::Text("Resources queued to loading: %i.", Runtime::ResourceManager::GetQueuedToLoadCount());
			ImGui::Text("Total resources loading: %i.", Runtime::ResourceManager::GetLoadingCount());
			ImGui::Text("Total resources loaded: %i.", Runtime::ResourceManager::GetLoadedResourcesCount());

			ImGui::Text("Resources:");
			Runtime::ResourceDatabase::ResourceMap resources = Runtime::ResourceManager::GetResourceMap();
			for (const auto& [resourceId, resource] : resources)
			{
				DrawSingleResource(resource);
			}

			ImGui::Text("Resource Packs:");
			std::vector<Runtime::ResourcePack*> resourcePacks = Runtime::ResourceManager::GetResourcePacks();
			for (Runtime::ResourcePack* const& pack : resourcePacks)
			{
				DrawSingleResourcePack(pack);
			}
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

			std::string fileName = resource->GetFileName();
			std::string resourceState = Runtime::ERsourceStatesToString(resource->GetResourceState());
			float loadTime = resource->GetLoadTimer().GetElapsedTimeMillFloat();
			ImGui::PushStyleColor(ImGuiCol_Header, resourceStateColours[static_cast<int>(resource->GetResourceState())]);
			if (ImGui::TreeNodeEx(static_cast<const void*>(resource), ImGuiTreeNodeFlags_Framed, "%s", fileName.c_str()))
			{
				ImGui::Text("Guid: %s", resource->GetGuid().ToString().c_str());
				ImGui::Text("Resource Type: %s", resource->GetResourceTypeId().GetTypeName().c_str());
				ImGui::Text("State: %s, LoadTime: %f.", resourceState.c_str(), loadTime);

				if (ImGui::Button("Load"))
				{
					Runtime::ResourceManager::LoadSync(Runtime::ResourceId(resource->GetFilePath(), resource->GetResourceTypeId()));
				}
				if (ImGui::Button("Unload"))
				{
					Runtime::ResourceManager::Unload(resource->GetResourceId());
				}
				ImGui::TreePop();
			}
			ImGui::PopStyleColor();
		}

		void ResourceWindow::DrawSingleResourcePack(Runtime::ResourcePack* resourcePack)
		{
			static constexpr ImVec4 stateColours[] =
			{
				ImVec4(0, 1, 0, 1),			// Open
				ImVec4(0, 0, 1, 1),			// Closed
			};

			std::string_view filePath = resourcePack->GetFilePath();
			std::string fileName = FileSystem::GetFileName(filePath);
			bool packLoaded = resourcePack->IsLoaded();

			bool popColourStyle = true;
			ImGui::PushStyleColor(ImGuiCol_Header, stateColours[static_cast<int>(packLoaded == true ? 0 : 1)]);

			if (ImGui::TreeNodeEx(static_cast<const void*>(resourcePack), ImGuiTreeNodeFlags_Framed, "%s", fileName.data()))
			{
				ImGui::PopStyleColor();
				popColourStyle = false;

				ImGui::Text("File Path: %s", filePath.data());
				ImGui::Text("Is Loaded: %s", packLoaded ? "Loaded" : "Unloaded");
				ImGui::Text("On Disk: %s", FileSystem::Exists(filePath) ? "True" : "False");

				if (ImGui::TreeNodeEx("Resources", ImGuiTreeNodeFlags_Framed))
				{
					std::vector<Runtime::IResource*> resources = resourcePack->GetAllResources();
					for (Runtime::IResource* resource : resources)
					{
						if (ImGui::TreeNodeEx((void*)resource, ImGuiTreeNodeFlags_Framed, "%s", resource->GetFileName().c_str()))
						{
							ImGui::Text("Guid: %s", resource->GetGuid().ToString().c_str());
							ImGui::Text("Resource Type: %s", resource->GetResourceTypeId().GetTypeName().c_str());
							ImGui::TreePop();
						}
					}
					ImGui::TreePop();
				}

				if (ImGui::Button("Save"))
				{
					resourcePack->Save();
				}

				ImGui::TreePop();
			}

			if (popColourStyle)
			{
				ImGui::PopStyleColor();
			}
		}
	}
}