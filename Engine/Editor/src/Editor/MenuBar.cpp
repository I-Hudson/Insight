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

#include "Maths/Vector3.h"
#include "Asset/Assets/Model.h"
#include "ECS/Components/TransformComponent.h"

#include "Editor/EditorStyles.h"
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
                        fileDialog.ShowSave(&item, Runtime::ProjectSystem::Instance().GetProjectInfo().GetProjectFilePath(),
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
                        fileDialog.ShowLoad(&item, Runtime::ProjectSystem::Instance().GetProjectInfo().GetProjectFilePath(),
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
                if (ImGui::BeginMenu("Benchmark"))
                {
                    static int benchmarkIndex = 0;
                    const float space = 2;
                    const i64 gridSize = 10;
                    const float offsetX = (benchmarkIndex % gridSize) * (gridSize * 2);
                    const float offsetZ = (benchmarkIndex / gridSize) * (gridSize * 2);

                    if (ImGui::MenuItem("Skeletal Animations"))
                    {
                        Ref<Runtime::ModelAsset> model = Runtime::AssetRegistry::Instance().LoadAsset("Base/Models/New folder/dancing_stormtrooper/gltf/scene.gltf").As<Runtime::ModelAsset>();
                        if (model)
                        {
                            for (size_t z = 0; z < gridSize; ++z)
                            {
                                for (size_t x = 0; x < gridSize; ++x)
                                {
                                    const float xPos = offsetX + ((0.0f - (gridSize * 0.5f)) + (space * x));
                                    const float zPos = offsetZ + ((0.0f - (gridSize * 0.5f)) + (space * z));
                                    Maths::Vector3 position(xPos, 0.0f, zPos);
                                    ECS::Entity* e = model->CreateEntityHierarchy();
                                    e->GetComponent<ECS::TransformComponent>()->SetPosition(position);
                                }
                            }
                            ++benchmarkIndex;
                        }
                    }
                    else if (ImGui::MenuItem("Static Mesh"))
                    {
                        Ref<Runtime::ModelAsset> model = Runtime::AssetRegistry::Instance().LoadAsset("Base/Models/New folder/dancing_stormtrooper/gltf/scene.gltf").As<Runtime::ModelAsset>();
                        const float space = 2;
                        const i64 gridSize = 10;
                        for (size_t z = 0; z < gridSize; ++z)
                        {
                            for (size_t x = 0; x < gridSize; ++x)
                            {
                                const float xPos = offsetX + ((0.0f - (gridSize * 0.5f)) + (space * x));
                                const float zPos = offsetZ + ((0.0f - (gridSize * 0.5f)) + (space * z));
                                Maths::Vector3 position(xPos, 0.0f, zPos);
                                ECS::Entity* e = model->CreateEntityHierarchyStaticMesh();
                                e->GetComponent<ECS::TransformComponent>()->SetPosition(position);
                            }
                        }
                        ++benchmarkIndex;
                    }
                    ImGui::EndMenu();
                }
                EditorStylesMenu();
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
                if (fileDialog.ShowSave(&item, Runtime::ProjectSystem::Instance().GetProjectInfo().GetContentPath(),
                    {
                        FileDialogFilter { L"World", L"*.isworld"},
                    }))
                {
                    TObjectPtr<Runtime::World> activeWorld = Runtime::WorldSystem::Instance().GetActiveWorld();
                    if (activeWorld)
                    {
                        activeWorld->SaveWorld(item);

                        const Runtime::ProjectInfo& projectInfo = Runtime::ProjectSystem::Instance().GetProjectInfo();
                        std::string relative = FileSystem::GetRelativePath(item, projectInfo.GetContentPath());
                        activeWorld->SaveDebugWorld(projectInfo.GetIntermediatePath() + "/World/" + relative);
                    }
                }
            }
            if (ImGui::MenuItem("Load World"))
            {
                std::string item;
                PlatformFileDialog fileDialog;
                if (fileDialog.ShowLoad(&item, Runtime::ProjectSystem::Instance().GetProjectInfo().GetContentPath(),
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

        void MenuBar::EditorStylesMenu() const
        {
            if (ImGui::BeginMenu("Editor Styles"))
            {
                if (ImGui::MenuItem("Unreal Dark Theme"))
                {
                    UnrealDarkTheme();
                }
                else if (ImGui::MenuItem("Cherry Theme"))
                {
                    CherryTheme();
                }
                else if (ImGui::MenuItem("Corporate Grey Theme"))
                {
                    CorporateGreyTheme();
                }
                else if (ImGui::MenuItem("DarkTheme"))
                {
                    DarkTheme();
                }
                ImGui::EndMenu();
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