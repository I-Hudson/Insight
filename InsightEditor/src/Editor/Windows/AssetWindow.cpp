#include "AssetWindow.h"
#include "Insight/FileSystem/FileSystem.h"
#include "Dirent/dirent.h"

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
            m_isAppearing = true;

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
            ImGui::Begin("Asset Browser");

            bool showError = false;

            if (m_isAppearing)
            {
                m_selectedFn.clear();
                m_selectedPath.clear();
                /* If current path is empty (can happen on Windows if user closes dialog while inside MyComputer.
                 * Since this is a virtual folder, path would be empty) load the drives on Windows else initialize the current path on Unix.
                 */
                if (m_currentPath.empty())
                {
#ifdef IS_PLATFORM_WINDOWS
                    showError |= !(LoadWindowsDrives());
#else
                    InitCurrentPath();
                    showError |= !(ReadAssetDirectory(m_currentPath));
#endif // IS_PLATFORM_WINDOWS
                    
                }
                else
                {
                    showError |= !(ReadAssetDirectory(m_currentPath));
                }
                m_isAppearing = false;
            }

            showError |= RenderNavAndSearchBarRegion();
            showError |= RenderFileListRegion();
            showError |= RenderInputTextAndExtRegion();
            showError |= RenderButtonsAndCheckboxRegion();

            ImGui::End();
        }

        void AssetWindow::ClearFileList()
        {
            //Clear pointer references to subdirs and subfiles
            m_filteredDirs.clear();
            m_filteredFiles.clear();
            m_inputcbFilterFiles.clear();

            //Now clear subdirs and subfiles
            m_subDirs.clear();
            m_subFiles.clear();
            m_filterDirty = true;
            m_selectedIdx = -1;
        }

        void AssetWindow::FilterFiles(const int& filterMode)
        {
            m_filterDirty = false;
            if (m_filterMode | FilterMode_Dirs)
            {
                m_filteredDirs.clear();
                for (size_t i = 0; i < m_subDirs.size(); ++i)
                {
                    if (m_filter.PassFilter(m_subDirs[i].Name.c_str()))
                    {
                        m_filteredDirs.push_back(&m_subDirs[i]);
                    }
                }
            }
            if (m_filterMode | FilterMode_Files)
            {
                m_filteredFiles.clear();
                for (size_t i = 0; i < m_subFiles.size(); ++i)
                {
                    m_filteredFiles.push_back(&m_subFiles[i]);
                }
            }
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
            DIR* dir;
            struct dirent* ent;

            /* If the current directory doesn't exist, and we are opening the dialog for the first time, reset to defaults to avoid looping of showing error modal.
            * An example case is when user closes the dialog in a folder. Then deletes the folder outside. On reopening the dialog the current path (previous) would be invalid.
            */
            dir = opendir(assetDirectoryPath.c_str());
            if (dir == nullptr && m_isAppearing)
            {
                m_directoryList.clear();
#ifdef IS_PLATFORM_WINDOWS
                m_currentPath = assetDirectoryPath;
                m_currentPath = m_currentPath.insert(0, "./");
#else
                InitCurrentPath();
                assetDirectoryPath = current_path;
#endif // IS_PLATFORM_WINDOWS

                dir = opendir(assetDirectoryPath.c_str());
            }

            if (dir != nullptr)
            {
#ifdef IS_PLATFORM_WINDOWS
                // If we are on Windows and current path is relative then get absolute path from dirent structure
                if (m_directoryList.empty() && assetDirectoryPath == "./")
                {
                    const wchar_t* absolute_path = dir->wdirp->patt;
                    std::string current_directory = WStringToString(absolute_path);
                    std::replace(current_directory.begin(), current_directory.end(), '\\', '/');

                    //Remove trailing "*" returned by ** dir->wdirp->patt **
                    current_directory.pop_back();
                    m_currentPath = current_directory;

                    //Create a vector of each directory in the file path for the filepath bar. Not Necessary for linux as starting directory is "/"
                    ParsePathTabs(m_currentPath);
                }
#endif // IS_PLATFORM_WINDOWS

                // store all the files and directories within directory and clear previous entries
                ClearFileList();
                while ((ent = readdir(dir)) != nullptr)
                {
                    bool is_hidden = false;
                    std::string name(ent->d_name);

                    //Ignore current directory
                    if (name == ".")
                    {
                        continue;
                    }

                    //Somehow there is a '..' present in root directory in linux.
#ifndef IS_PLATFORM_WINDOWS
                    if (name == ".." && assetDirectoryPath == "/")
                    {
                        continue;
                    }
#endif // IS_PLATFORM_WINDOWS

                    if (name != "..")
                    {
#ifdef IS_PLATFORM_WINDOWS
                        std::string dir = assetDirectoryPath + std::string(ent->d_name);
                        // IF system file skip it...
                        if (FILE_ATTRIBUTE_SYSTEM & GetFileAttributesA(dir.c_str()))
                        {
                            //continue;
                        }
                        if (FILE_ATTRIBUTE_HIDDEN & GetFileAttributesA(dir.c_str()))
                        {
                            is_hidden = true;
                        }
#else
                        if (name[0] == '.')
                        {
                            is_hidden = true;
                        }
#endif // IS_PLATFORM_WINDOWS
                    }
                    //Store directories and files in separate vectors
                    if (ent->d_type == DT_DIR)
                    {
                        m_subDirs.push_back(Info(name, is_hidden));
                    }
                    else if (ent->d_type == DT_REG)
                    {
                        m_subFiles.push_back(Info(name, is_hidden));
                    }
                }
                closedir(dir);
                std::sort(m_subDirs.begin(), m_subDirs.end(), AlphaSortComparator);
                std::sort(m_subFiles.begin(), m_subFiles.end(), AlphaSortComparator);

                //Initialize Filtered dirs and files
                FilterFiles(m_filterMode);
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
            int num_cols = static_cast<int>(std::max(1.0f, std::ceil(static_cast<float>(m_filteredDirs.size() + m_filteredFiles.size()) / m_colItemsLimit)));

            //Limitation by ImGUI in 1.75. If columns are greater than 64 readjust the limit on items per column and recalculate number of columns
            if (num_cols > 64)
            {
                int exceed_items_amount = (num_cols - 64) * m_colItemsLimit;
                m_colItemsLimit += static_cast<int>(std::ceil(exceed_items_amount / 64.0));
                num_cols = static_cast<int>(std::max(1.0f, std::ceil(static_cast<float>(m_filteredDirs.size() + m_filteredFiles.size()) / m_colItemsLimit)));
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
            for (int i = 0; i < m_filteredDirs.size(); i++)
            {
                if (!m_filteredDirs[i]->IsHidden || m_showHidden)
                {
                    items++;
                    if (ImGui::Selectable(m_filteredDirs[i]->Name.c_str(), m_selectedIdx == i && m_isDir, ImGuiSelectableFlags_AllowDoubleClick))
                    {
                        m_selectedIdx = i;
                        m_isDir = true;

                        m_inputFn = m_filteredDirs[i]->Name;

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
            for (int i = 0; i < m_filteredFiles.size(); i++)
            {
                if (!m_filteredFiles[i]->IsHidden || m_showHidden)
                {
                    items++;
                    if (ImGui::Selectable(m_filteredFiles[i]->Name.c_str(), m_selectedIdx == i && !m_isDir, ImGuiSelectableFlags_AllowDoubleClick))
                    {
                        //int len = filtered_files[i]->name.length();
                        m_selectedIdx = i;
                        m_isDir = false;

                        // If dialog mode is OPEN/SAVE then copy the selected file name to the input text bar
                        m_inputFn = m_filteredFiles[i]->Name;

                        if (ImGui::IsMouseDoubleClicked(0))
                        {
                            m_selectedFn = m_filteredFiles[i]->Name;
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

            name = m_filteredDirs[idx]->Name;

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

        //Windows Exclusive function
#ifdef IS_PLATFORM_WINDOWS
        bool AssetWindow::LoadWindowsDrives()
        {
            DWORD len = GetLogicalDriveStringsA(0, nullptr);
            char* drives = new char[len];
            if (!GetLogicalDriveStringsA(len, drives))
            {
                delete[] drives;
                return false;
            }

            ClearFileList();
            char* temp = drives;
            for (char* drv = nullptr; *temp != '\0'; temp++)
            {
                drv = temp;
                if (DRIVE_REMOVABLE == GetDriveTypeA(drv))
                {
                    m_subDirs.push_back({ "Removable Disk: " + std::string(1,drv[0]), false });
                }
                else if (DRIVE_FIXED == GetDriveTypeA(drv))
                {
                    m_subDirs.push_back({ "Local Disk: " + std::string(1,drv[0]), false });
                }
                //Go to nullptr character
                while (*(++temp));
            }
            delete[] drives;
            return true;
        }
#endif

#ifndef IS_PLATFORM_WINDOWS
        void AssetWindow::InitCurrentPath()
        {
            bool path_max_def = false;

#ifdef PATH_MAX
            path_max_def = true;
#endif // PATH_MAX

            char* buffer = nullptr;

            //If PATH_MAX is defined deal with memory using new/delete. Else fallback to malloc'ed memory from `realpath()`
            if (path_max_def)
            {
                buffer = new char[PATH_MAX];
            }

            char* real_path = realpath("./", buffer);
            if (real_path == nullptr)
            {
                current_path = "/";
                current_dirlist.push_back("/");
            }
            else
            {
                m_ = std::string(real_path);
                current_path += "/";
                ParsePathTabs(m_currentPath);
            }

            if (path_max_def)
            {
                delete[] buffer;
            }
            else
            {
                free(real_path);
            }
        }
#endif
    }
}