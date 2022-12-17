#include "Editor/FileDialog.h"

#include "Resource/Texture2D.h"
#include "Resource/ResourceManager.h"

#include "FileSystem/FileSystem.h"

#include <misc/cpp/imgui_stdlib.h>
#include <imgui_internal.h>

#include <filesystem>

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

        //------------------------------------------
        // FileDialogItem
        //------------------------------------------
        FileDialogItem::FileDialogItem(std::string path)
            : m_path(std::move(path))
        {
            u64 lastSlashChar = m_path.find_last_of('/');
            if (lastSlashChar == std::string::npos)
            {
                lastSlashChar = 0;
            }
            else
            {
                lastSlashChar += 1;
            }
            m_pathName = m_path.substr(lastSlashChar);

            m_icon = Runtime::ResourceManagerExt::Load(Runtime::ResourceId("./Resources/Icons/Default.png", Runtime::Texture2D::GetStaticResourceTypeId()));
            m_isDirectory = FileSystem::FileSystem::IsDirectory(m_path);
            m_clickedTimer.Start();
        }

        void FileDialogItem::Clicked()
        {
            m_clickedTimer.Stop();
            m_timeSinceLastClick = m_clickedTimer.GetElapsedTimeMill();
            m_clickedTimer.Start();
        }

        float FileDialogItem::GetTimeSinceLastClickMs() const
        {
            return static_cast<float>(m_timeSinceLastClick.count());
        }

        const std::string& FileDialogItem::GetPath() const
        {
            return m_path;
        }

        const std::string& FileDialogItem::GetPathName() const
        {
            return m_pathName;
        }

        Runtime::Texture2D* FileDialogItem::GetIcon() const
        {
            return m_icon;
        }

        bool FileDialogItem::IsDirectory() const
        {
            return m_isDirectory;
        }

        //------------------------------------------
        // FileDialogNavigation
        //------------------------------------------
        void FileDialogNavigation::SetPath(const std::string& path)
        {
            for (size_t i = 0; i < c_HistoryLevelSize; ++i)
            {
                m_pathHistory[i] = "";
            }
            m_path = path;
            SplitPathToSections();
        }

        void FileDialogNavigation::Navigate(const std::string& path)
        {
            UpdateHistory();
            m_pathHistory[0] = m_path;
            m_path = path;
            SplitPathToSections();
        }

        void FileDialogNavigation::NavigateFromPathSelection(const u32 index)
        {
            std::string path;
            path.reserve(m_path.size());
            for (size_t i = 0; i < m_pathSections.size(); ++i)
            {
                path += m_pathSections.at(i);
                if (i == index)
                {
                    break;
                }
            }
            if (path.back() == '/')
            {
                path.pop_back();
            }
            Navigate(path);
        }

        void FileDialogNavigation::Backwards()
        {

        }

        void FileDialogNavigation::Forwards()
        {

        }

        const std::string& FileDialogNavigation::GetPath() const
        {
            return m_path;
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
        // FileDialog
        //------------------------------------------
        void FileDialog::Show(const std::string& path, FileDialogOperations operation)
        {
            m_operation = operation;
            m_isVisable = true;

            m_title = "File Dialog " + std::string(FileDialogOperationToString(m_operation));
            m_navigation.SetPath(FileSystem::FileSystem::GetAbsolutePath(path));

            m_inputBox.clear();
            m_selectionMade = false;

            m_isDirty = true;
        }

        bool FileDialog::Update()
        {
            return Update(nullptr);
        }

        bool FileDialog::Update(std::string* result)
        {
            if (!m_isVisable)
            {
                return false;
            }

            ImGui::SetNextWindowSize(ImVec2(640, 420), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSizeConstraints(ImVec2(350, 250), ImVec2(FLT_MAX, FLT_MAX));

            const ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse
                | ImGuiWindowFlags_NoScrollbar
                | ImGuiWindowFlags_NoFocusOnAppearing
                | ImGuiWindowFlags_NoDocking;

            ImGui::Begin(m_title.c_str(), &m_isVisable, windowFlags);
            ImGui::SetWindowFocus();

            DrawTop();
            DrawContents();
            DrawBottom();

            ImGui::End();

            if (m_isDirty)
            {
                m_isDirty = false;
                UpdateItems();
            }

            if (m_selectionMade 
                && result != nullptr)
            {
                std::string possiableResult = m_navigation.GetPath() + "/" + m_inputBox;
                if (FileSystem::FileSystem::Exists(possiableResult))
                {
                    *result = m_inputBox;
                }
                else
                {
                    *result = "";
                }
                return true;
            }

            return false;
        }

        void FileDialog::DrawTop()
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
                        m_navigation.NavigateFromPathSelection(i);
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
            // Compute some useful stuff
            const auto window = ImGui::GetCurrentWindowRead();
            const auto content_width = ImGui::GetContentRegionAvail().x;
            const auto content_height = ImGui::GetContentRegionAvail().y - m_offsetBottom;
            ImGuiContext& g = *GImGui;
            ImGuiStyle& style = ImGui::GetStyle();
            const float font_height = g.FontSize;
            const float label_height = font_height;
            const float text_offset = 3.0f;
            float pen_x_min = 0.0f;
            float pen_x = 0.0f;
            bool new_line = true;
            m_displayItemCount = 0;
            ImRect rect_button;
            ImRect rect_label;

            if (ImGui::BeginChild("##ContentRegion", ImVec2(content_width, content_height), true))
            {
                // Set starting position
                {
                    float offset = ImGui::GetStyle().ItemSpacing.x;
                    pen_x_min = ImGui::GetCursorPosX() + offset;
                    ImGui::SetCursorPosX(pen_x_min);

                }

                // Go through all the items
                for (int i = 0; i < m_items.size(); i++)
                {
                    // Get item to be displayed
                    auto& item = m_items[i];

                    m_displayItemCount++;

                    // Start new line ?
                    if (new_line)
                    {
                        ImGui::BeginGroup();
                        new_line = false;
                    }

                    ImGui::BeginGroup();
                    {
                        // Compute rectangles for elements that make up an item
                        {
                            rect_button = ImRect
                            (
                                ImGui::GetCursorScreenPos().x,
                                ImGui::GetCursorScreenPos().y,
                                ImGui::GetCursorScreenPos().x + m_itemSize.x,
                                ImGui::GetCursorScreenPos().y + m_itemSize.y
                            );

                            rect_label = ImRect
                            (
                                rect_button.Min.x,
                                rect_button.Max.y - label_height - style.FramePadding.y,
                                rect_button.Max.x,
                                rect_button.Max.y
                            );
                        }

                        ImGui::PushID(i);
                        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 0.25f));

                        // THUMBNAIL
                        {
                            if (ImGui::Button("##dummy", m_itemSize))
                            {
                                // Determine type of click
                                item.Clicked();
                                const bool isSingleClick = item.GetTimeSinceLastClickMs() > 500;

                                if (isSingleClick)
                                {
                                    // Updated input box
                                    m_inputBox = item.GetPathName();
                                }
                                else // Double Click
                                {
                                    if (item.IsDirectory())
                                    {
                                        m_isDirty = true;
                                        m_navigation.Navigate(item.GetPath());
                                    }
                                    m_selectionMade = !item.IsDirectory();
                                }
                            }
                        }

                        // Image
                        {
                            // Compute thumbnail size
                            ImVec2 image_size_max = ImVec2(rect_button.Max.x - rect_button.Min.x - style.FramePadding.x * 2.0f, rect_button.Max.y - rect_button.Min.y - style.FramePadding.y - label_height - 5.0f);
                            ImVec2 image_size = item.GetIcon() ? ImVec2(static_cast<float>(item.GetIcon()->GetWidth()), static_cast<float>(item.GetIcon()->GetHeight())) : image_size_max;
                            ImVec2 image_size_delta = ImVec2(0.0f, 0.0f);

                            // Scale the image size to fit the max available size while respecting it's aspect ratio
                            {
                                // Clamp width
                                if (image_size.x != image_size_max.x)
                                {
                                    float scale = image_size_max.x / image_size.x;
                                    image_size.x = image_size_max.x;
                                    image_size.y = image_size.y * scale;
                                }
                                // Clamp height
                                if (image_size.y != image_size_max.y)
                                {
                                    float scale = image_size_max.y / image_size.y;
                                    image_size.x = image_size.x * scale;
                                    image_size.y = image_size_max.y;
                                }

                                image_size_delta.x = image_size_max.x - image_size.x;
                                image_size_delta.y = image_size_max.y - image_size.y;
                            }

                            ImGui::SetCursorScreenPos(ImVec2(rect_button.Min.x + style.FramePadding.x + image_size_delta.x * 0.5f, rect_button.Min.y + style.FramePadding.y + image_size_delta.y * 0.5f));
                            ImGui::Image(item.GetIcon()->GetRHITexture(), image_size);
                        }

                        ImGui::PopStyleColor(2);
                        ImGui::PopID();

                        // LABEL
                        {
                            const char* label_text = item.GetPathName().c_str();
                            const ImVec2 label_size = ImGui::CalcTextSize(label_text, nullptr, true);

                            // Draw text background
                            ImGui::GetWindowDrawList()->AddRectFilled(rect_label.Min, rect_label.Max, IM_COL32(51, 51, 51, 190));
                            //ImGui::GetWindowDrawList()->AddRect(rect_label.Min, rect_label.Max, IM_COL32(255, 0, 0, 255)); // debug

                            // Draw text
                            ImGui::SetCursorScreenPos(ImVec2(rect_label.Min.x + text_offset, rect_label.Min.y + text_offset));
                            if (label_size.x <= m_itemSize.x && label_size.y <= m_itemSize.y)
                            {
                                ImGui::TextUnformatted(label_text);
                            }
                            else
                            {
                                ImGui::RenderTextClipped(rect_label.Min, rect_label.Max, label_text, nullptr, &label_size, ImVec2(0, 0), &rect_label);
                            }
                        }

                        ImGui::EndGroup();
                    }

                    // Decide whether we should switch to the next column or switch row
                    pen_x += m_itemSize.x + ImGui::GetStyle().ItemSpacing.x;
                    if (pen_x >= content_width - m_itemSize.x)
                    {
                        ImGui::EndGroup();
                        pen_x = pen_x_min;
                        ImGui::SetCursorPosX(pen_x);
                        new_line = true;
                    }
                    else
                    {
                        ImGui::SameLine();
                    }
                }

                if (!new_line)
                {
                    ImGui::EndGroup();
                }
            }
            ImGui::EndChild();
        }

        void FileDialog::DrawBottom()
        {
            m_offsetBottom = 35.0f;
            ImGui::SetCursorPosY(ImGui::GetWindowSize().y - m_offsetBottom);

            ImGui::PushItemWidth(ImGui::GetWindowSize().x - 235);
            ImGui::InputText("##InputBox", &m_inputBox);
            ImGui::PopItemWidth();

            ImGui::SameLine();
            ImGui::Text("All(*.*)");

            ImGui::SameLine();
            if (ImGui::Button(FileDialogOperationToString(m_operation)))
            {
                m_selectionMade = true;
                m_isVisable = false;
            }

            ImGui::SameLine();
            if (ImGui::Button("Cancel"))
            {
                m_selectionMade = false;
                m_isVisable = false;
            }
        }

        void FileDialog::UpdateItems()
        {
            if (!FileSystem::FileSystem::IsDirectory(m_navigation.GetPath()))
            {
                return;
            }

            m_items.clear();
            std::filesystem::directory_iterator directoryIter = std::filesystem::directory_iterator(m_navigation.GetPath());
            for (const auto& entry : directoryIter)
            {
                m_items.emplace_back(FileSystem::FileSystem::PathToUnix(entry.path().u8string()));
            }
        }
    }
}