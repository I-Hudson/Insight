#include "Editor/MenuBar.h"

#include "Editor/EditorWindows/EditorWindowManager.h"
#include "Editor/EditorWindows/EntitiesWindow.h"
#include "Editor/EditorWindows/InputWindow.h"
#include "Editor/EditorWindows/ResourceWindow.h"

#include "Platforms/Platform.h"

#include "Serialisation/Archive.h"
#include "Serialisation/JsonSerialiser.h"
#include "World/WorldSystem.h"

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
                    if (ImGui::MenuItem("Save Project"))
                    {
                        //m_fileDialog.Show("./", FileDialogOperations::Save);
                        std::string item;
                        PlatformFileDialog fileDialog;
                        fileDialog.ShowSave(&item);
                    }
                    if (ImGui::MenuItem("Load Project"))
                    {
                        //m_fileDialog.Show("./", FileDialogOperations::Load);
                        std::string item;
                        PlatformFileDialog fileDialog;
                        fileDialog.ShowLoad(&item,
                            { 
                                { "Model (*.model)",        "*.model"},
                                { "Mesh (*.mesh)",          "*.mesh"},
                                { "Texture (*.texture)",    "*.texture"},
                                { "Material (*.material)",  "*.material"},
                            });
                    }
                    if (ImGui::MenuItem("Save World"))
                    {
                        //m_fileDialog.Show("./", FileDialogOperations::Load);
                        std::string item;
                        PlatformFileDialog fileDialog;
                        if (fileDialog.ShowSave(&item,
                            {
                                { "World", "*.isworld"},
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