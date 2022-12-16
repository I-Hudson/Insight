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
                    std::string label = EntitiesWindow::WINDOW_NAME;
                    label += m_editorWindowManager->IsWindowVisable(EntitiesWindow::WINDOW_NAME) ? " x" : "";
                    if (ImGui::MenuItem(label.c_str()))
                    {
                        m_editorWindowManager->AddWindow(EntitiesWindow::WINDOW_NAME);
                    }
                    
                    label = InputWindow::WINDOW_NAME;
                    label += m_editorWindowManager->IsWindowVisable(InputWindow::WINDOW_NAME) ? " x" : "";
                    if (ImGui::MenuItem(label.c_str()))
                    {
                        m_editorWindowManager->AddWindow(InputWindow::WINDOW_NAME);
                    }

                    label = ResourceWindow::WINDOW_NAME;
                    label += m_editorWindowManager->IsWindowVisable(ResourceWindow::WINDOW_NAME) ? " x" : "";
                    if (ImGui::MenuItem(label.c_str()))
                    {
                        m_editorWindowManager->AddWindow(ResourceWindow::WINDOW_NAME);
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();

                m_fileDialog.Update();
            }
        }
    }
}