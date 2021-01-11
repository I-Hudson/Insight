#include "AssetWindow.h"
#include "Insight/FileSystem/FileSystem.h"
#include <filesystem>

namespace Insight
{
    namespace Editor
    {
        AssetWindow::AssetWindow(SharedPtr<Module::EditorModule> editorModule)
            : EditorWindow(editorModule)
        { 
            m_filterMode = FilterMode_Files | FilterMode_Dirs;
            m_validateFile = false;
            m_showHidden = false;
            m_isDir = false;
            m_filterDirty = true;
            m_initRead = true;

            m_layout = ColumnLayout::OneColumn;
            m_colItemsLimit = 12;
            m_selectedIdx = -1;
            m_selectedExtIdx = 0;
            m_extBoxWidth = -1.0f;
            m_colWidth = 280.0f;
            
            m_selectedFn = "";
            m_selectedPath = "";
            m_inputFn = '\0';

#ifdef IS_PLATFORM_WINDOWS
            m_currentPath = "./";
#else
            initCurrentPath();
#endif
        }

        void AssetWindow::Update(const float& deltaTime)
        {
            IS_PROFILE_FUNCTION();

            ImGui::Begin("Asset Browser");

            bool showError = false;

            if (m_initRead)
            {
                showError |= !(ReadAssetDirectory(m_currentPath));
                m_initRead = false;
            }

            showError |= RenderNavAndSearchBarRegion();
            showError |= RenderFileListRegion();
            showError |= RenderInputTextAndExtRegion();
            showError |= RenderButtonsAndCheckboxRegion();

            ImGui::End();
        }

        void AssetWindow::ClearFileList()
        {
            //Now clear subdirs and subfiles
            m_subDirs.clear();
            m_subFiles.clear();
            m_filterDirty = true;
            m_selectedIdx = -1;
        }

        void AssetWindow::ParsePathTabs(const std::string& path)
        {
            std::string pathElement = "";
            std::string root = "";

#ifdef IS_PLATFORM_WINDOWS
            m_directoryList.push_back("Computer");
#else
            if (path[0] == '/')
            {
                m_directoryList.push_back("/");
            }
#endif // IS_PLATFORM_WINDOWS

            std::istringstream iss(path);
            while (std::getline(iss, pathElement, '/'))
            {
                if (!pathElement.empty())
                {
                    m_directoryList.push_back(pathElement);
                }
            }
        }

        bool AssetWindow::ReadAssetDirectory(const std::string& assetDirectoryPath)
        {
            /* If the current directory doesn't exist, and we are opening the dialog for the first time, reset to defaults to avoid looping of showing error modal.
            * An example case is when user closes the dialog in a folder. Then deletes the folder outside. On reopening the dialog the current path (previous) would be invalid.
            */
            if (assetDirectoryPath.empty())
            {
                m_directoryList.clear();
#ifdef IS_PLATFORM_WINDOWS
                m_currentPath = assetDirectoryPath;
                m_currentPath = m_currentPath.insert(0, "./");
#else
                InitCurrentPath();
                assetDirectoryPath = current_path;
#endif // IS_PLATFORM_WINDOWS
            }

            std::filesystem::path path = assetDirectoryPath;
            if (!assetDirectoryPath.empty())
            {
#ifdef IS_PLATFORM_WINDOWS
                // If we are on Windows and current path is relative then get absolute path from dirent structure
                if (m_directoryList.empty() && assetDirectoryPath == "./")
                {
                    std::string absolute_path = std::filesystem::absolute(path).string();
                    std::replace(absolute_path.begin(), absolute_path.end(), '\\', '/');
                    m_currentPath = absolute_path;

                    //Create a vector of each directory in the file path for the filepath bar. Not Necessary for linux as starting directory is "/"
                    ParsePathTabs(m_currentPath);
                }
#endif // IS_PLATFORM_WINDOWS

                // store all the files and directories within directory and clear previous entries
                ClearFileList();
                for (auto& f : std::filesystem::directory_iterator(std::filesystem::path(m_currentPath)))
                {
                    //Ignore current directory
                    if (f.is_directory())
                    {
                        m_subDirs.push_back(Info(f.path().filename().string(), false));
                    }
                    else if (f.is_regular_file())
                    {
                        m_subFiles.push_back(Info(f.path().filename().string(), false));
                    }
                }
                std::sort(m_subDirs.begin(), m_subDirs.end(), AlphaSortComparator);
                std::sort(m_subFiles.begin(), m_subFiles.end(), AlphaSortComparator);
            }
            else
            {
                m_errorTitle = "Error!";
                m_errorMsg = "Error opening directory! Make sure the directory exists and you have the proper rights to access the directory.";
                return false;
            }
            return true;
        }

        bool AssetWindow::AlphaSortComparator(const Info& a, const Info& b)
        {
            const char* str1 = a.Name.c_str();
            const char* str2 = b.Name.c_str();
            int ca, cb;
            do
            {
                ca = (unsigned char)*str1++;
                cb = (unsigned char)*str2++;
                ca = std::tolower(std::toupper(ca));
                cb = std::tolower(std::toupper(cb));
            } while (ca == cb && ca != '\0');
            if (ca < cb)
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        bool AssetWindow::RenderNavAndSearchBarRegion()
        {
            return false;
        }

        bool AssetWindow::RenderFileListRegion()
        {
            ImGuiStyle& style = ImGui::GetStyle();
            ImVec2 pw_size = ImGui::GetWindowSize();
            bool show_error = false;
            float list_item_height = ImGui::CalcTextSize("").y + style.ItemSpacing.y;
            float input_bar_ypos = pw_size.y - ImGui::GetFrameHeightWithSpacing() * 2.5f - style.WindowPadding.y;
            float window_height = input_bar_ypos - ImGui::GetCursorPosY() - style.ItemSpacing.y;
            float window_content_height = window_height - style.WindowPadding.y * 2.0f;
            float min_content_size = pw_size.x - style.WindowPadding.x * 4.0f;

            if (window_content_height <= 0.0f)
                return show_error;

            //Reinitialize the limit on number of selectables in one column based on height
            m_colItemsLimit = static_cast<int>(std::max(1.0f, window_content_height / list_item_height));
            int num_cols = static_cast<int>(std::max(1.0f, std::ceil(static_cast<float>(m_subDirs.size() + m_subFiles.size()) / m_colItemsLimit)));

            //Limitation by ImGUI in 1.75. If columns are greater than 64 readjust the limit on items per column and recalculate number of columns
            if (num_cols > 64)
            {
                int exceed_items_amount = (num_cols - 64) * m_colItemsLimit;
                m_colItemsLimit += static_cast<int>(std::ceil(exceed_items_amount / 64.0));
                num_cols = static_cast<int>(std::max(1.0f, std::ceil(static_cast<float>(m_subDirs.size() + m_subFiles.size()) / m_colItemsLimit)));
            }

            float content_width = num_cols * m_colWidth;
            if (content_width < min_content_size)
                content_width = 0;

            ImGui::SetNextWindowContentSize(ImVec2(content_width, 0));
            ImGui::BeginChild("##ScrollingRegion", ImVec2(0, window_height), true, ImGuiWindowFlags_HorizontalScrollbar);
            ImGui::Columns(num_cols);

            //Output directories in yellow
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.882f, 0.745f, 0.078f, 1.0f));
            int items = 0;
            for (int i = 0; i < m_subDirs.size(); i++)
            {
                if (!m_subDirs[i].IsHidden || m_showHidden)
                {
                    items++;
                    if (ImGui::Selectable(m_subDirs[i].Name.c_str(), m_selectedIdx == i && m_isDir, ImGuiSelectableFlags_AllowDoubleClick))
                    {
                        m_selectedIdx = i;
                        m_isDir = true;

                        m_inputFn = m_subDirs[i].Name;

                        if (ImGui::IsMouseDoubleClicked(0))
                        {
                            show_error |= !(OnDirClick(i));
                            break;
                        }
                    }
                    if ((items) % m_colItemsLimit == 0)
                    {
                        ImGui::NextColumn();
                    }
                }
            }
            ImGui::PopStyleColor(1);

            //Output files
            for (int i = 0; i < m_subFiles.size(); i++)
            {
                if (!m_subFiles[i].IsHidden || m_showHidden)
                {
                    items++;
                    if (ImGui::Selectable(m_subFiles[i].Name.c_str(), m_selectedIdx == i && !m_isDir, ImGuiSelectableFlags_AllowDoubleClick))
                    {
                        //int len = filtered_files[i]->name.length();
                        m_selectedIdx = i;
                        m_isDir = false;

                        m_inputFn = m_subFiles[i].Name;

                        if (ImGui::IsMouseDoubleClicked(0))
                        {
                            m_selectedFn = m_subFiles[i].Name;
                            m_validateFile = true;
                        }
                    }
                    if ((items) % m_colItemsLimit == 0)
                    {
                        ImGui::NextColumn();
                    }
                }
            }
            ImGui::Columns(1);
            ImGui::EndChild();

            return show_error;
        }

        bool AssetWindow::RenderInputTextAndExtRegion()
        {
            return false;
        }

        bool AssetWindow::RenderButtonsAndCheckboxRegion()
        {
            return false;
        }

        bool AssetWindow::OnDirClick(const int& idx)
        {
            std::string name;
            std::string new_path(m_currentPath);
            bool drives_shown = false;

#ifdef IS_PLATFORM_WINDOWS
            drives_shown = (m_directoryList.size() == 1 && m_directoryList.back() == "Computer");
#endif // OSWIN

            name = m_subDirs[idx].Name;

            if (name == "..")
            {
                new_path.pop_back(); // Remove trailing '/'
                new_path = new_path.substr(0, new_path.find_last_of('/') + 1); // Also include a trailing '/'
            }
            else
            {
                //Remember we displayed drives on Windows as *Local/Removable Disk: X* hence we need last char only
                if (drives_shown)
                {
                    name = std::string(1, name.back()) + ":";
                }
                new_path += name + "/";
            }

            if (ReadAssetDirectory(new_path))
            {
                if (name == "..")
                {
                    m_directoryList.pop_back();
                }
                else
                {
                    m_directoryList.push_back(name);
                }

                m_currentPath = new_path;
                return true;
            }
            else
            {
                return false;
            }
        }
    }
}