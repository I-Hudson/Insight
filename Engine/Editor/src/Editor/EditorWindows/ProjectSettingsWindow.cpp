#include "Editor/EditorWindows/ProjectSettingsWindow.h"
#include "Editor/EditorGUI.h"

#include "Editor/EditorWindows/ContentWindow.h"

#include "Runtime/ProjectSystem.h"
#include "Runtime/RuntimeSettings.h"

#include "World/WorldSystem.h"
#include "Asset/AssetRegistry.h"

#include "Core/Profiler.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

namespace Insight
{
    namespace Editor
    {
        ProjectSettingsWindow::ProjectSettingsWindow()
            : IEditorWindow()
        { }

        ProjectSettingsWindow::ProjectSettingsWindow(u32 minWidth, u32 minHeight)
            : IEditorWindow(minWidth, minHeight)
        { }

        ProjectSettingsWindow::ProjectSettingsWindow(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight)
            : IEditorWindow(minWidth, minHeight, maxWidth, maxHeight)
        { }

        ProjectSettingsWindow::~ProjectSettingsWindow()
        { }

        void ProjectSettingsWindow::OnDraw()
        {
            IS_PROFILE_FUNCTION();

            const Runtime::ProjectInfo& projectInfo = Runtime::ProjectSystem::Instance().GetProjectInfo();
            if (!projectInfo.IsOpen)
            {
                return;
            }

            ImGui::Text("Name");
            ImGui::SameLine();
            ImGui::Text("%s", projectInfo.ProjectName.c_str());

            ImGui::Text("Path");
            ImGui::SameLine();
            ImGui::Text("%s", projectInfo.ProjectPath.c_str());

            ImGui::Text("Version");
            ImGui::SameLine();
            ImGui::Text("%u", projectInfo.ProjectVersion);

            Reflect::TypeInfo worldTypeInfo = Runtime::World::GetStaticTypeInfo();
            const Runtime::AssetInfo* initialWorld = Runtime::AssetRegistry::Instance().GetAsset(projectInfo.DefaultWorld);

            {
                ImGui::BeginGroup();

                std::string_view worldName = "";
                if (initialWorld)
                {
                    worldName = initialWorld->FileName;
                    Runtime::RuntimeSettings::Instance().InitialWorldPath = initialWorld->GetFullFilePath();

                }

                ImGui::Text("Initial World");
                ImGui::SameLine();
                //ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.2f, 0.2f, 1));
                ImGui::InputText("##PorjectSettingsInitalWorld", (char*)worldName.data(), worldName.size(), ImGuiInputTextFlags_ReadOnly);
                //ImGui::PopStyleColor();

                ImGui::EndGroup();

                std::string worldDropTargetData;
                EditorGUI::ObjectFieldTarget(ContentWindow::c_ContentWindowAssetDragSource,
                    worldDropTargetData);

                if (!worldDropTargetData.empty())
                {
                    Core::GUID newInitialWorldGuid;
                    newInitialWorldGuid.StringToGuid(worldDropTargetData);

                    const Runtime::AssetInfo* newInitialworldAssetInfo = Runtime::AssetRegistry::Instance().GetAsset(newInitialWorldGuid);
                    if (newInitialworldAssetInfo)
                    {
                        RemoveConst(projectInfo.DefaultWorld) = newInitialworldAssetInfo->Guid;
                    }
                }
            }
        }
    }
}