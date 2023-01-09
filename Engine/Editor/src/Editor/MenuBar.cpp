#include "Editor/MenuBar.h"

#include "Editor/EditorWindows/EditorWindowManager.h"
#include "Editor/EditorWindows/EntitiesWindow.h"
#include "Editor/EditorWindows/InputWindow.h"
#include "Editor/EditorWindows/ResourceWindow.h"

#include "Platforms/Platform.h"

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
                        PlatformFileDialog::ShowSave(&item);
                    }
                    if (ImGui::MenuItem("Load Project"))
                    {
                        //m_fileDialog.Show("./", FileDialogOperations::Load);
                        std::string item;
                        PlatformFileDialog::ShowLoad(&item, 
                            { 
                                { "Model (*.model)",        "*.model"},
                                { "Mesh (*.mesh)",          "*.mesh"},
                                { "Texture (*.texture)",    "*.texture"},
                                { "Material (*.material)",  "*.material"},
                            });
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Edit"))
                {
                    //if (ImGui::MenuItem(PreferencesWindow::WINDOW_NAME))
                    {
                       //m_editorWindowManager->AddWindow(PreferencesWindow::WINDOW_NAME);
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Windows"))
                {
                    std::vector<std::string> allRegisteredWindows = m_editorWindowManager->GetAllRegisteredWindowNames();
                    for (std::string const& windowName : allRegisteredWindows)
                    {
                        std::string label = windowName;
                        label += m_editorWindowManager->IsWindowVisable(windowName) ? " x" : "";
                        if (ImGui::MenuItem(label.c_str()))
                        {
                            m_editorWindowManager->AddWindow(windowName);
                        }
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();

                m_fileDialog.Update();
            }
        }
    }
}