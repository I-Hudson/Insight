#include "Editor/FileDialog.h"

#include "Resource/Texture2D.h"

#include "FileSystem/FileSystem.h"

namespace Insight
{
    namespace Editor
    {
        CONSTEXPR const char* FileDialogOperationToString(FileDialogOperations operation)
        {
            switch (operation)
            {
            case Insight::Editor::FileDialogOperations::Save: return "Save";
            case Insight::Editor::FileDialogOperations::Load: return "Load";
            default:
                break;
            }
            return "";
        }

        void FileDialogNavigation::SetPath(const std::string& path)
        {
            for (size_t i = 0; i < c_HistoryLevelSize; ++i)
            {
                m_pathHistory[i] = "";
            }
            m_path = path;
            SplitPathToSections();
        }

        //------------------------------------------
        // FileDialogNavigation
        //------------------------------------------
        void FileDialogNavigation::Navigate(const std::string& path)
        {
            UpdateHistory();
            std::string newAbsolutePath;
            newAbsolutePath.reserve(m_path.size());
            for (size_t i = 0; i < m_pathSections.size(); ++i)
            {
                newAbsolutePath += m_pathSections.at(i);
                if (path == m_pathSections.at(i))
                {
                    break;
                }
            }
            if (newAbsolutePath.back() == '/')
            {
                newAbsolutePath.pop_back();
            }

            m_pathHistory[0] = m_path;
            m_path = newAbsolutePath;
            SplitPathToSections();
        }

        void FileDialogNavigation::Backwards()
        {

        }

        void FileDialogNavigation::Forwards()
        {

        }

        u32 FileDialogNavigation::GetPathSectionSize() const
        {
            return static_cast<u32>(m_pathSections.size());
        }

        const std::vector<std::string>& FileDialogNavigation::GetPathSections() const
        {
            return m_pathSections;
        }

        const std::string& FileDialogNavigation::GetPathSection(u32 index) const
        {
            return m_pathSections.at(index);
        }

        void FileDialogNavigation::SplitPathToSections()
        {
            m_pathSections.clear();

            std::string path = m_path;
            u64 slashOffset = path.find_first_of('/');
            while (slashOffset != std::string::npos)
            {
                std::string section = path.substr(0, slashOffset + 1);
                m_pathSections.push_back(section);

                path = path.substr(slashOffset + 1);
                slashOffset = path.find_first_of('/');
            }
            m_pathSections.push_back(path);
        }

        void FileDialogNavigation::UpdateHistory()
        {
            for (int i = c_HistoryLevelSize - 1; i > 0; --i)
            {
                std::swap(m_pathHistory[i], m_pathHistory[i - 1]);
            }
        }

        //------------------------------------------
        // FileDialogItem
        //------------------------------------------
        FileDialogItem::FileDialogItem(std::string path)
            : m_path(std::move(path))
        {
            m_icon = static_cast<Runtime::Texture2D*>(Runtime::ResourceManager::Instance().Load("./Resources/Icons/Default.png", Runtime::Texture2D::GetStaticResourceTypeId()));
        }

        std::string FileDialogItem::GetPath() const
        {
            return m_path;
        }

        std::string FileDialogItem::GetPathName() const
        {
            u64 lastSlashChar = m_path.find_last_of('/');
            if (lastSlashChar == std::string::npos)
            {
                lastSlashChar = 0;
            }
            u64 lastDotChar = m_path.find_last_of('.');
            return m_path.substr(lastSlashChar, lastDotChar - lastSlashChar);
        }

        Runtime::Texture2D* FileDialogItem::GetIcon() const
        {
            return m_icon;
        }

        bool FileDialogItem::IsDirectory() const
        {
            return false;
        }

        void FileDialog::Show(const std::string& path, FileDialogOperations operation)
        {
            m_navigation.SetPath(FileSystem::FileSystem::GetAbsolutePath(path));
            m_operation = operation;
            m_isVisable = true;
            m_title = "File Dialog " + std::string(FileDialogOperationToString(m_operation));
        }

        void FileDialog::Update()
        {
            if (!m_isVisable)
            {
                return;
            }

            ImGui::SetNextWindowSize(ImVec2(640, 420), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSizeConstraints(ImVec2(350, 250), ImVec2(FLT_MAX, FLT_MAX));

            const ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse
                | ImGuiWindowFlags_NoScrollbar
                | ImGuiWindowFlags_NoFocusOnAppearing
                | ImGuiWindowFlags_NoDocking;

            ImGui::Begin(m_title.c_str(), &m_isVisable, windowFlags);
            ImGui::SetWindowFocus();

            DrawTopBar();
            DrawContents();
            DrawBottomBar();

            ImGui::End();
        }

        void FileDialog::DrawTopBar()
        {
            // Directory navigation buttons
            {
                // Backwards
                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
                bool result = ImGui::Button("<");
                ImGui::PopStyleVar();
                if (result)
                {
                    //m_isDirty = m_navigation.Backward();
                }
                ImGui::SameLine();

                // Forwards
                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
                result = ImGui::Button(">");
                ImGui::PopStyleVar();
                if (result)
                {
                    //m_isDirty = m_navigation.Backward();
                }

                ImGui::SameLine();
                ImGui::Spacing();
                ImGui::SameLine();

                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
                // Individual directories buttons
                for (uint32_t i = 0; i < m_navigation.GetPathSectionSize(); ++i)
                {
                    ImGui::SameLine();
                    if (ImGui::Button(m_navigation.GetPathSection(i).c_str()))
                    {
                        m_isDirty = true;
                        m_navigation.Navigate(m_navigation.GetPathSection(i));
                    }
                }
                ImGui::PopStyleVar();
            }

            // Search filter
            const float label_width = 37.0f; //ImGui::CalcTextSize("Filter", nullptr, true).x;
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12);
            //m_search_filter.Draw("Filter", ImGui::GetContentRegionAvail().x - label_width);
            ImGui::PopStyleVar();

            ImGui::Separator();
        }

        void FileDialog::DrawContents()
        {
        }

        void FileDialog::DrawBottomBar()
        {
        }

        void FileDialog::UpdateItems()
        {
        }
}
}