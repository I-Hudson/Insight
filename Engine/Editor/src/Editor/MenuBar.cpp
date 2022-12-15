#include "Editor/MenuBar.h"

#include "Editor/EditorWindows/EditorWindowManager.h"
#include "Editor/EditorWindows/EntitiesWindow.h"
#include "Editor/EditorWindows/InputWindow.h"
#include "Editor/EditorWindows/ResourceWindow.h"

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
                        m_fileDialog.Show("./", FileDialogOperations::Save);
                    }
                    if (ImGui::MenuItem("Load Project"))
                    {
                        m_fileDialog.Show("./", FileDialogOperations::Load);
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
                    if (ImGui::MenuItem(EntitiesWindow::WINDOW_NAME))
                    {
                        m_editorWindowManager->AddWindow(EntitiesWindow::WINDOW_NAME);
                    }
                    if (ImGui::MenuItem(InputWindow::WINDOW_NAME))
                    {
                        m_editorWindowManager->AddWindow(InputWindow::WINDOW_NAME);
                    }
                    if (ImGui::MenuItem(ResourceWindow::WINDOW_NAME))
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