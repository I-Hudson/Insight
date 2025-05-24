#include "Editor/Asset/Inspector/AssetPackageAssetInspector.h"

#include "Editor/EditorWindows/ContentWindow.h"

#include "Asset/AssetRegistry.h"
#include "Asset/AssetPackage/IAssetPackage.h"

#include "Editor/EditorGUI.h"
#include "Editor/Editor.h"

#include <IconsFontAwesome5.h>

#include <imgui.h>

namespace Insight
{
	namespace Editor
	{
		AssetPackageAssetInspector::AssetPackageAssetInspector()
			: IAssetInspector({ Runtime::IAssetPackage::c_FileExtension })
		{ }

		AssetPackageAssetInspector::~AssetPackageAssetInspector()
		{ }

		void AssetPackageAssetInspector::Draw(const Runtime::AssetInfo* assetInfo)
		{
			std::vector<IObject*> assetObjects = Runtime::AssetRegistry::Instance().GetObjectsFromAsset(assetInfo->Guid);
			if (assetObjects.empty())
			{
				return;
			}

			Runtime::IAssetPackage* assetPackage = static_cast<Runtime::IAssetPackage*>(assetObjects.at(0));
			if (!assetPackage)
			{
				return;
			}

			ImGui::Text("Asset package name: %s", assetPackage->GetName().data());
			const std::vector<Runtime::AssetInfo*>& packageAssetInfos = assetPackage->GetAllAssetInfos();

			if (ImGui::BeginTable("##AssetPackageAssetInspectorTable", 2), ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInner)
			{
				ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_None);
				ImGui::TableSetupColumn("Path", ImGuiTableColumnFlags_None);
				ImGui::TableHeadersRow();

				ImGuiListClipper clipper;
				clipper.Begin(static_cast<int>(packageAssetInfos.size()));

				while (clipper.Step())
				{
					for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
					{
						const Runtime::AssetInfo* info = packageAssetInfos.at(row);

						ImGui::BeginGroup();
						{
							ImGui::TableNextColumn();
							ImGui::Text("%s", info ? info->FileName.c_str() : "");
						}
						{
							ImGui::TableNextColumn();
							ImGui::Text("%s", info ? info->FilePath.c_str() : "");
						}
						ImGui::EndGroup();

						std::string assetDragGuid;
						if (EditorGUI::ObjectFieldTarget(ContentWindow::c_ContentWindowAssetDragSource, assetDragGuid))
						{
							Core::GUID assetGuid;
							assetGuid.StringToGuid(assetDragGuid);
							const Runtime::AssetInfo* dragGuid = Runtime::AssetRegistry::Instance().GetAssetInfo(assetGuid);
							if (dragGuid)
							{
								assetPackage->ReplaceAsset(RemoveConst(info), RemoveConst(dragGuid));
								Runtime::IAssetPackage* projectAssetPackage = Runtime::AssetRegistry::Instance().GetAssetPackageFromName(Editor::Editor::c_ProjectAssetPackageName);
								if (projectAssetPackage)
								{
									projectAssetPackage->AddAsset(RemoveConst(info));
								}
							}
						}
					}
				}
				ImGui::EndTable();
			}

			ImGui::BeginGroup();

			ImGuiListClipper clipper;
			clipper.Begin(static_cast<int>(packageAssetInfos.size()));

			while (clipper.Step())
			{
				for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
				{
					Runtime::AssetInfo*& info = RemoveConst(packageAssetInfos.at(row));

					const std::string textLabel = "Asset Name: " + (info ? info->FileName : "");
					const ImVec4 bgColour(0.5f, 0.5f, 1.0f, 1.0f);

					ImVec2 textSize = ImGui::CalcTextSize(textLabel.c_str());
					ImVec2 rectMin(ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y);
					ImVec2 rectMax(rectMin.x + textSize.x, rectMin.y + textSize.y);

					ImGui::GetWindowDrawList()->AddRectFilled(rectMin, rectMax, IM_COL32(bgColour.x * 255, bgColour.y * 255, bgColour.z * 255, bgColour.w * 255));
					ImGui::Text(textLabel.c_str());
				}
			}
			ImGui::EndGroup();
			std::string assetDragGuid;
			if (EditorGUI::ObjectFieldTarget(ContentWindow::c_ContentWindowAssetDragSource, assetDragGuid))
			{
				Core::GUID assetGuid;
				assetGuid.StringToGuid(assetDragGuid);
				const Runtime::AssetInfo* dargAssetInfo = Runtime::AssetRegistry::Instance().GetAssetInfo(assetGuid);
				if (dargAssetInfo)
				{
					assetPackage->AddAsset(RemoveConst(dargAssetInfo));
				}
			}

			if (ImGui::Button("Build Package"))
			{
				PlatformFileDialog dialog;
				std::string buildPath;
				if (dialog.ShowSave(&buildPath, Runtime::ProjectSystem::Instance().GetProjectInfo().GetProjectPath(), { FileDialogFilter(L"AssetPackage", L".isassetpackage") }))
				{
					assetPackage->BuildPackage(buildPath);
				}
			}
		}
	}
}