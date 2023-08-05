#include "Editor/EditorWindows/AssetAuditWindow.h"

#include "Asset/AssetRegistry.h"
#include "Asset/AssetPackage.h"

#include "FileSystem/FileSystem.h"

#include "Resource/Loaders/ResourceLoaderRegister.h"

#include "Core/Profiler.h"

#include <imgui.h>

namespace Insight
{
    namespace Editor
    {
        AssetAuditWindow::AssetAuditWindow()
            : IEditorWindow()
        { }
        AssetAuditWindow::AssetAuditWindow(u32 minWidth, u32 minHeight)
            : IEditorWindow(minWidth, minHeight)
        { }
        AssetAuditWindow::AssetAuditWindow(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight)
            : IEditorWindow(minWidth, minHeight, maxWidth, maxHeight)
        { }
        AssetAuditWindow::~AssetAuditWindow()
        { }

        void AssetAuditWindow::OnDraw()
        {
            IS_PROFILE_FUNCTION();

            std::vector<Runtime::AssetPackage*> assetPackages = Runtime::AssetRegistry::Instance().GetAllAssetPackages();

            for (int i = 0; i < assetPackages.size(); ++i)
            {
                Runtime::AssetPackage* package = assetPackages.at(i);
                if (ImGui::Button(package->GetName().data()))
                {
                    m_selectedAssetPackage = package;
                }

                if (i < assetPackages.size() - 1)
                {
                    ImGui::SameLine();
                }
            }

            const int tableFlags = ImGuiTableFlags_Resizable
                | ImGuiTableFlags_RowBg
                | ImGuiTableFlags_BordersInner
                ;

            if (m_selectedAssetPackage 
                && ImGui::BeginTable("Asset Audit", 3, tableFlags))
            {
                {
                    ImGui::TableNextColumn();
                    ImGui::Text("Folder Path");
                }
                {
                    ImGui::TableNextColumn();
                    ImGui::Text("File Name");
                }
                {
                    ImGui::TableNextColumn();
                    ImGui::Text("Resource Type Id");
                }

                std::vector<const Runtime::AssetInfo*> assetInfos = m_selectedAssetPackage->GetAllAssetInfos();
                for (const Runtime::AssetInfo* info : assetInfos)
                {
                    {
                        ImGui::TableNextColumn();
                        ImGui::Text("%s", info->FilePath.c_str());
                    }
                    {
                        ImGui::TableNextColumn();
                        ImGui::Text("%s", info->FileName.c_str());
                    }
                    {
                        ImGui::TableNextColumn();

                        std::string_view fileExtension = FileSystem::GetExtension(info->FileName);
                        const Runtime::IResourceLoader* loader = Runtime::ResourceLoaderRegister::GetLoaderFromExtension(fileExtension);
                        if (loader != nullptr)
                        {
                            Runtime::ResourceTypeId typeId = loader->GetResourceTypeId();
                            ImGui::Text("%s", typeId.GetTypeName().c_str());
                        }
                        else
                        {
                            ImGui::Text("Unknown");
                        }
                    }
                }
                ImGui::EndTable();
            }
            else
            {
                ImGui::Text("No package has been selected.");
            }
        }
    }
}