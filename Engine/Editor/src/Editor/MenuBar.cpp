#include "Editor/MenuBar.h"

#include "Editor/EditorWindows/EditorWindowManager.h"
#include "Editor/EditorWindows/WorldEntitiesWindow.h"
#include "Editor/EditorWindows/InputWindow.h"
#include "Editor/EditorWindows/ResourceWindow.h"

#include "Editor/ProjectCode/ProjectCodeSystem.h"

#include "Platforms/Platform.h"

#include "Serialisation/Archive.h"
#include "Serialisation/Serialisers/JsonSerialiser.h"
#include "World/WorldSystem.h"

#include "Resource/ResourceManager.h"
#include "Runtime/ProjectSystem.h"

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
                                FileDialogFilter{ L"Project (*.isproject)", L"*.isproject"},
                            });
                        Runtime::ProjectSystem::Instance().CreateProject(item, item);
                        Runtime::ProjectSystem::Instance().OpenProject(item);
                    }
                    if (ImGui::MenuItem("Save Project"))
                    {
                        //Runtime::ProjectSystem::Instance().SaveProject();
                    }
                    if (ImGui::MenuItem("Load Project"))
                    {
                        std::string item;
                        PlatformFileDialog fileDialog;
                        fileDialog.ShowLoad(&item,
                            { 
                                FileDialogFilter{ L"Project (*.isproject)", L"*.isproject"},
                            });
                        Runtime::ProjectSystem::Instance().OpenProject(item);
                    }
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
                                Serialisation::JsonSerialiser serialiser(false);
                                activeWorld->Serialise(&serialiser);

                                std::vector<u8> serialisedData = serialiser.GetSerialisedData();
                                Archive archive(item, ArchiveModes::Write);
                                archive.Write(serialisedData.data(), serialisedData.size());
                                archive.Close();
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
                            TObjectPtr<Runtime::World> activeWorld = Runtime::WorldSystem::Instance().GetActiveWorld();
                            if (activeWorld)
                            {
                                Archive archive(item, ArchiveModes::Read);
                                archive.Close();

                                Serialisation::JsonSerialiser serialiser(true);
                                serialiser.Deserialise(archive.GetData());

                                activeWorld->Deserialise(&serialiser);
                            }
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
                    if (ImGui::MenuItem("Save Resource Database"))
                    {
                        Runtime::ResourceManager::SaveDatabase();
                    }
                    if (ImGui::MenuItem("Load Resource Database"))
                    {
                        Runtime::ResourceManager::LoadDatabase();
                    }
                    if (ImGui::MenuItem("Clear Resource Database"))
                    {
                        Runtime::ResourceManager::ClearDatabase();
                    }
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
                        ProjectCodeSystem::Instance().GenerateProjectFiles();
                    }
                    if (ImGui::MenuItem("Link Project DLL"))
                    {
                        ProjectCodeSystem::Instance().LinkProject();
                    }
                    if (ImGui::MenuItem("Unlink Project DLL"))
                    {
                        ProjectCodeSystem::Instance().UnlinkProject();
                    }
                    ImGui::EndMenu();
                }
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
    }
}