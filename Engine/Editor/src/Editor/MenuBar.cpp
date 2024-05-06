#include "Editor/MenuBar.h"

#include "Editor/EditorWindows/EditorWindowManager.h"

#include "Editor/HotReload/HotReloadSystem.h"

#include "Platforms/Platform.h"

#include "Serialisation/Archive.h"
#include "Serialisation/Serialisers/JsonSerialiser.h"
#include "World/WorldSystem.h"

#include "Runtime/ProjectSystem.h"

#include "Asset/AssetRegistry.h"

#include "Graphics/RenderDocAPI.h"

#include "Core/Profiler.h"

#include <imgui.h>

namespace Insight
{
    namespace Editor
    {
        MenuBar::MenuBar()
        { }

        MenuBar::~MenuBar()
        { }

        void MenuBar::Initialise(EditorWindowManager* editorWindowManager)
        {
            m_editorWindowManager = editorWindowManager;
        }

        void MenuBar::Draw()
        {
            IS_PROFILE_FUNCTION();

            if (ImGui::BeginMainMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    if (ImGui::MenuItem("Create Project"))
                    {
                        std::string item;
                        PlatformFileDialog fileDialog;
                        fileDialog.ShowSave(&item,
                            {
                                FileDialogFilter{ L"Project", L"*.isproject"},
                            });
                        Runtime::ProjectSystem::Instance().CreateProject(item, item);
                        Runtime::ProjectSystem::Instance().OpenProject(item);
                    }
                    if (ImGui::MenuItem("Save Project"))
                    {
                        Runtime::ProjectSystem::Instance().SaveProject();
                    }
                    if (ImGui::MenuItem("Load Project"))
                    {
                        std::string item;
                        PlatformFileDialog fileDialog;
                        fileDialog.ShowLoad(&item,
                            { 
                                FileDialogFilter{ L"Project", L"*.isproject"},
                            });
                        Runtime::ProjectSystem::Instance().OpenProject(item);
                    }

                    WorldItems();
                    
                    DrawAllRegisteredWindow(EditorWindowCategories::File);
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Edit"))
                {
                    DrawAllRegisteredWindow(EditorWindowCategories::Edit);
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Windows"))
                {
                    DrawAllRegisteredWindow(EditorWindowCategories::Windows);
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Code"))
                {
                    if (ImGui::MenuItem("Generate Project Solution"))
                    {
                        HotReloadSystem::Instance().GenerateProjectSolution();
                    }
                    if (ImGui::MenuItem("Reload Project DLL"))
                    {
                        HotReloadSystem::Instance().Reload();
                    }
                    ImGui::EndMenu();
                }
                DrawProfileMenu();
                ImGui::EndMainMenuBar();

                m_fileDialog.Update();
            }
        }

        void MenuBar::DrawAllRegisteredWindow(EditorWindowCategories category) const
        {
            std::vector<std::string> allRegisteredWindows = m_editorWindowManager->GetAllRegisteredWindowNames(category);
            for (std::string const& windowName : allRegisteredWindows)
            {
                std::string label = windowName;
                label += m_editorWindowManager->IsWindowVisable(windowName) ? " x" : "";
                if (ImGui::MenuItem(label.c_str()))
                {
                    m_editorWindowManager->AddWindow(windowName);
                }
            }
        }

        void MenuBar::WorldItems()
        {
            if (ImGui::MenuItem("Save World"))
            {
                std::string item;
                PlatformFileDialog fileDialog;
                if (fileDialog.ShowSave(&item,
                    {
                        FileDialogFilter { L"World", L"*.isworld"},
                    }))
                {
                    TObjectPtr<Runtime::World> activeWorld = Runtime::WorldSystem::Instance().GetActiveWorld();
                    if (activeWorld)
                    {
                        //activeWorld->SaveWorld(item);

                        const Runtime::ProjectInfo& projectInfo = Runtime::ProjectSystem::Instance().GetProjectInfo();
                        std::string relative = FileSystem::GetRelativePath(item, projectInfo.GetContentPath());
                        //activeWorld->SaveDebugWorld(projectInfo.GetIntermediatePath() + "/World/" + relative);
                    }
                }
            }
            if (ImGui::MenuItem("Load World"))
            {
                std::string item;
                PlatformFileDialog fileDialog;
                if (fileDialog.ShowLoad(&item,
                    {
                        FileDialogFilter{ L"World", L"*.isworld"},
                    }))
                {
                    Runtime::WorldSystem::Instance().RemoveWorld(Runtime::WorldSystem::Instance().GetActiveWorld());
                    Runtime::WorldSystem::Instance().LoadWorld(item);
                }
            }
            if (ImGui::MenuItem("Clear World"))
            {
                TObjectPtr<Runtime::World> activeWorld = Runtime::WorldSystem::Instance().GetActiveWorld();
                if (activeWorld)
                {
                    activeWorld->Destroy();
                }
            }
        }

        void MenuBar::DrawProfileMenu() const
        {
            if (Graphics::RenderDocAPI::Instance().IsInitialised()
                && ImGui::BeginMenu("RenderDoc"))
            {
                if (ImGui::MenuItem("Capture"))
                {
                    Graphics::RenderDocAPI::Instance().Capture();
                }
                if (ImGui::MenuItem("Enable Overlay"))
                {
                    Graphics::RenderDocAPI::Instance().EnableOverlay();
                }
                if (ImGui::MenuItem("Disable Overlay"))
                {
                    Graphics::RenderDocAPI::Instance().DisableOverlay();
                }
                ImGui::EndMenu();
            }
        }
    }
}