#include "Editor/Asset/Inspector/AssetPackageAssetInspector.h"

#include "Editor/EditorWindows/ContentWindow.h"

#include "Asset/AssetRegistry.h"
#include "Asset/AssetPackage.h"

#include "Editor/EditorGUI.h"

#include <IconsFontAwesome5.h>

#include <imgui.h>

namespace Insight
{
	namespace Editor
	{
		AssetPackageAssetInspector::AssetPackageAssetInspector()
			: IAssetInspector({ Runtime::AssetPackage::c_FileExtension })
		{ }

		AssetPackageAssetInspector::~AssetPackageAssetInspector()
		{ }

		void AssetPackageAssetInspector::Draw(const Runtime::AssetInfo* assetInfo)
		{
			IObject* assetObject = Runtime::AssetRegistry::Instance().GetObjectFromAsset(assetInfo->Guid);
			if (!assetObject)
			{
				return;
			}

			Runtime::AssetPackage* assetPackage = static_cast<Runtime::AssetPackage*>(assetObject);
			if (!assetPackage)
			{
				return;
			}

			ImGui::Text("Asset package name: %s", assetPackage->GetName().data());
			std::vector<const Runtime::AssetInfo*> packageAssetInfos = assetPackage->GetAllAssetInfos();

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
						const Runtime::AssetInfo*& info = packageAssetInfos.at(row);

						ImGui::BeginGroup();
						{
							ImGui::TableNextColumn();
							ImGui::Text("%s", info->FileName.c_str());
						}
						{
							ImGui::TableNextColumn();
							ImGui::Text("%s", info->FilePath.c_str());
						}
						ImGui::EndGroup();

						std::string assetDragGuid;
						if (EditorGUI::ObjectFieldTarget(ContentWindow::c_ContentWindowAssetDragSource, assetDragGuid))
						{
							Core::GUID assetGuid;
							assetGuid.StringToGuid(assetDragGuid);
							const Runtime::AssetInfo* dragGuid = Runtime::AssetRegistry::Instance().GetAsset(assetGuid);
							if (dragGuid)
							{

							}
						}
					}
				}
				ImGui::EndTable();
			}

			if (ImGui::Button("+"))
			{
				assetPackage->AddAsset("");
			}
			if (packageAssetInfos.size() > 0 && ImGui::Button("-"))
			{
				assetPackage->RemoveAsset(packageAssetInfos.at(packageAssetInfos.size() - 1)->GetFullFilePath());
			}

			ImGuiListClipper clipper;
			clipper.Begin(static_cast<int>(packageAssetInfos.size()));

			while (clipper.Step())
			{
				for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
				{
					const Runtime::AssetInfo* info = packageAssetInfos.at(row);

					const std::string textLabel = "Asset Name: " + info->FileName;
					const ImVec4 bgColour(0.5f, 0.5f, 1.0f, 1.0f);

					ImVec2 textSize = ImGui::CalcTextSize(textLabel.c_str());
					ImVec2 rectMin(ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y);
					ImVec2 rectMax(rectMin.x + textSize.x, rectMin.y + textSize.y);

					ImGui::GetWindowDrawList()->AddRectFilled(rectMin, rectMax, IM_COL32(bgColour.x * 255, bgColour.y * 255, bgColour.z * 255, bgColour.w * 255));
					ImGui::Text(textLabel.c_str());

					std::string assetDragGuid;
					if (EditorGUI::ObjectFieldTarget(ContentWindow::c_ContentWindowAssetDragSource, assetDragGuid))
					{
						Core::GUID assetGuid;
						assetGuid.StringToGuid(assetDragGuid);
						const Runtime::AssetInfo* dragGuid = Runtime::AssetRegistry::Instance().GetAsset(assetGuid);
						if (dragGuid)
						{

						}
					}
				}
			}
		}
	}
}