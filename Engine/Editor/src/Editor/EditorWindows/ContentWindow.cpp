#include "Editor/EditorWindows/ContentWindow.h"
#include "Editor/EditorWindows/EditorWindowManager.h"

#include "Editor/Asset/AssetInspectorWindow.h"

#include "Editor/EditorGUI.h"
#include "Editor/EditorAssetRegistry.h"

#include "Asset/AssetRegistry.h"
#include "Asset/Assets/Texture.h"
#include "Asset/Assets/Model.h"

#include "Runtime/ProjectSystem.h"

#include "FileSystem/FileSystem.h"

#include "World/WorldSystem.h"

#include "Core/EnginePaths.h"
#include "Core/Profiler.h"
#include "Core/Logger.h"

#include "Event/EventSystem.h"

#include <filesystem>
#include <imgui.h>
#include <imgui_internal.h>

namespace Insight::Editor
{
    //==============================
    // IContentThumbnail
    //==============================
    void IContentThumbnail::Draw(const Runtime::Asset* asset, ContentWindow& contentWindow)
    {
        const Runtime::AssetInfo* assetInfo = asset->GetAssetInfo();
        ImGui::PushID(assetInfo->FileName.c_str());
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 0.25f));

        DrawButton(asset, contentWindow);
        DrawImage(asset, contentWindow);
        DrawContent(asset, contentWindow);

        ImGui::PopStyleColor(2);
        ImGui::PopID();
    }

    void IContentThumbnail::DrawButton(const Runtime::Asset* asset, ContentWindow& contentWindow)
    {
        ItemSelected = false;
        if (ImGui::Button("##dummy", ItemSize))
        {
            contentWindow.m_lastClickTimer.Stop();
            float elapsedTime = contentWindow.m_lastClickTimer.GetElapsedTimeMillFloat();
            contentWindow.m_lastClickTimer.Start();
            const bool isSingleClick = elapsedTime > 0.5f;

            if (isSingleClick)
            {
                contentWindow.m_currentItemSelected = asset->GetAssetInfo()->GetFullFilePath();
                ItemSelected = true;
            }
            else
            {
                AssetInspectorWindow* assetInspectorWindow = static_cast<AssetInspectorWindow*>(EditorWindowManager::Instance().GetActiveWindow(AssetInspectorWindow::WINDOW_NAME));
                if (assetInspectorWindow)
                {
                    assetInspectorWindow->SetSelectedAssetInfo(asset->GetAssetInfo());
                }
            }
        }
    }

    void IContentThumbnail::DrawImage(const Runtime::Asset* asset, ContentWindow& contentWindow)
    {
        const Runtime::AssetInfo* assetInfo = asset->GetAssetInfo();
        // Image
        {
            IS_PROFILE_SCOPE("Image");
            // Compute thumbnail size
            Graphics::RHI_Texture* texture = nullptr;
            if (texture == nullptr)
            {
                Ref<Runtime::Asset> thumbnailTexture = contentWindow.PathToThumbnail(assetInfo->GetFullFilePath());
                if (thumbnailTexture)
                {
                    texture = contentWindow.PathToThumbnail(assetInfo->GetFullFilePath())->GetRHITexture();
                }
            }

            const ImGuiContext* context = ImGui::GetCurrentContext();
            ImGuiStyle style = ImGui::GetStyle();
            const float fontHeight = context->FontSize;

            ImVec2 image_size_max = ImVec2(RectSize.Max.x - RectSize.Min.x - style.FramePadding.x * 2.0f, RectSize.Max.y - RectSize.Min.y - style.FramePadding.y - fontHeight - 5.0f);
            ImVec2 image_size = texture ? ImVec2(static_cast<float>(texture->GetWidth()), static_cast<float>(texture->GetHeight())) : image_size_max;
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

            // Position the image within the square border
            ImGui::SetCursorScreenPos(ImVec2(RectSize.Min.x + style.FramePadding.x + image_size_delta.x * 0.5f, RectSize.Min.y + style.FramePadding.y + image_size_delta.y * 0.5f));

            // Draw the image
            ImGui::Image(texture, image_size);
        }
    }

    //==============================
    // ContentThumbnailModelAsset
    //==============================
    void ContentThumbnailModelAsset::DrawContent(const Runtime::Asset* asset, ContentWindow& contentWindow) const
    {
    }
    Reflect::TypeInfo ContentThumbnailModelAsset::GetAssetTypeInfo() const
    {
        return Runtime::ModelAsset::GetStaticTypeInfo();
    }

    //==============================
    // ContentWindow
    //==============================
    ContentWindow::ContentWindow()
        : IEditorWindow()
    { }

    ContentWindow::ContentWindow(u32 minWidth, u32 minHeight)
        : IEditorWindow(minWidth, minHeight)
    { }

    ContentWindow::ContentWindow(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight)
        : IEditorWindow(minWidth, minHeight, maxWidth, maxHeight)
    { }

    ContentWindow::~ContentWindow()
    { }

    void ContentWindow::OnDraw()
    {
        IS_PROFILE_FUNCTION();
        bool s = true;
        ImGui::ShowDebugLogWindow(&s);
        // Top bar
        TopBar();
        // Centre thumbnails
        CentreArea();
        // Bottom bar
        BottomBar();

        if (m_showImportWindow)
        {
            ImportResource();
        }

        if (m_showCreateResourceWindow)
        {
            CreateNewResourceWindow();
        }
    }

    void ContentWindow::Initialise()
    {
        Core::EventSystem::Instance().AddEventListener(this, Core::EventType::Project_Open, 
            [this](Core::Event& e)
            {
                m_currentDirectory = Runtime::ProjectSystem::Instance().GetProjectInfo().GetContentPath();
                SplitDirectory();
            });

        m_contentThumbnail.push_back(::New<ContentThumbnailModelAsset>());

        m_fileExtensionToTexture[".fbx"] = 
            Runtime::AssetRegistry::Instance().LoadAsset(EnginePaths::GetResourcePath() + "/Editor/Icons/FBX_File_Icon.png").As<Runtime::TextureAsset>();

        m_fileExtensionToTexture[".obj"] =
            Runtime::AssetRegistry::Instance().LoadAsset(EnginePaths::GetResourcePath() + "/Editor/Icons/OBJ_File_Icon.png").As<Runtime::TextureAsset>();
        
        m_fileExtensionToTexture[".gltf"] =
            Runtime::AssetRegistry::Instance().LoadAsset(EnginePaths::GetResourcePath() + "/Editor/Icons/GLTF_File_Icon.png").As<Runtime::TextureAsset>();
        
        m_fileExtensionToTexture[".isworld"] =
            Runtime::AssetRegistry::Instance().LoadAsset(EnginePaths::GetResourcePath() + "/Editor/Icons/World_File_Icon.png").As<Runtime::TextureAsset>();

        m_fileExtensionToTexture[".mp3"] =
            Runtime::AssetRegistry::Instance().LoadAsset(EnginePaths::GetResourcePath() + "/Editor/Icons/MP3_File_Icon.png").As<Runtime::TextureAsset>();
        m_fileExtensionToTexture[".wav"] =
            Runtime::AssetRegistry::Instance().LoadAsset(EnginePaths::GetResourcePath() + "/Editor/Icons/WAV_File_Icon.png").As<Runtime::TextureAsset>();


        m_thumbnailToTexture[ContentWindowThumbnailType::Folder] =
            Runtime::AssetRegistry::Instance().LoadAsset(EnginePaths::GetResourcePath() + "/Editor/Icons/Folder.png").As<Runtime::TextureAsset>();
        m_thumbnailToTexture[ContentWindowThumbnailType::File] =
            Runtime::AssetRegistry::Instance().LoadAsset(EnginePaths::GetResourcePath() + "/Editor/Icons/File.png").As<Runtime::TextureAsset>();


        if (Runtime::ProjectSystem::Instance().IsProjectOpen())
        {
            m_currentDirectory = Runtime::ProjectSystem::Instance().GetProjectInfo().GetContentPath();
            SplitDirectory();
        }
    }

    void ContentWindow::Shutdown()
    {
        for (size_t i = 0; i < m_contentThumbnail.size(); ++i)
        {
            ::Delete(m_contentThumbnail[i]);
        }
        m_contentThumbnail.clear();
        Core::EventSystem::Instance().RemoveEventListener(this, Core::EventType::Project_Open);
    }

    void ContentWindow::TopBar()
    {
        IS_PROFILE_FUNCTION();

        if (ImGui::Button("Import"))
        {
            // Import a new asset.
            PlatformFileDialog importDialog;
            if (importDialog.ShowLoad(&m_importFilePath))
            {
                m_showImportWindow = true;
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("Create Resource"))
        {
            m_showCreateResourceWindow = true;
        }

        bool contentFolderFound = false;
        std::string currentPath;
        for (size_t i = 0; i < m_currentDirectoryParents.size(); ++i)
        {
            if (!contentFolderFound)
            {
                currentPath += m_currentDirectoryParents[i] + "/";
                std::filesystem::path contentRelativePath = "";
                {
                    IS_PROFILE_SCOPE("relative");
                    contentRelativePath = FileSystem::GetRelativePath(currentPath, Runtime::ProjectSystem::Instance().GetProjectInfo().GetContentPath());
                    //std::filesystem::relative(currentPath, Runtime::ProjectSystem::Instance().GetProjectInfo().GetContentPath());
                }
                if (contentRelativePath.empty())
                {
                    contentFolderFound = true;
                }
            }

            if (!contentFolderFound)
            {
                continue;
            }

            std::string buttonTitle = m_currentDirectoryParents[i] + "/";
            if (ImGui::Button(buttonTitle.c_str()))
            {
                SetDirectoryFromParent((u32)i);
                return;
            }
            ImGui::SameLine();
        }
        ImGui::Separator();
    }

    void ContentWindow::CentreArea()
    {
        IS_PROFILE_FUNCTION();

        const ImVec2 itemSize = ImVec2(128.0f, 128.0f);
        const ImGuiTableFlags tableFlags = ImGuiTableFlags_Resizable;

        if (ImGui::BeginTable("Content Browser", 2, tableFlags))
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            // DrawTreeView();

            ImGui::TableNextColumn();
            //float collumnWidth = ImGui::GetColumnWidth(1);
            //if (collumnWidth < itemSize.x)
            //{
            //    ImGui::TableSetColumnWidth(1, itemSize.x);
            //}

            // Compute some useful stuff
            const auto window = ImGui::GetCurrentWindowRead();
            const auto content_width = ImGui::GetContentRegionAvail().x;
            const auto content_height = ImGui::GetContentRegionAvail().y - 20.0f;
            ImGuiContext& g = *GImGui;
            ImGuiStyle& style = ImGui::GetStyle();
            const float font_height = g.FontSize;
            const float label_height = font_height;
            const float text_offset = 3.0f;
            float pen_x_min = 0.0f;
            float pen_x = 0.0f;
            bool new_line = true;
            static u32 displayed_item_count = 0;
            ImRect rect_button;
            ImRect rect_label;

            // Remove border
            ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);

            const float contentBackgroundColour[4] = { 0.0f, 0.0f, 0.0f, 50.0f };
            // Make background slightly darker
            ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(
                static_cast<int>(contentBackgroundColour[0]),
                static_cast<int>(contentBackgroundColour[1]),
                static_cast<int>(contentBackgroundColour[2]),
                static_cast<int>(contentBackgroundColour[3])));

            bool itemSelectedThisFrame = false;

            if (ImGui::BeginChild("##ContentWindow"))
            {
                // Set starting position
                {
                    float offset = ImGui::GetStyle().ItemSpacing.x;
                    pen_x_min = ImGui::GetCursorPosX() + offset;
                    ImGui::SetCursorPosX(pen_x_min);
                }

                if (!m_currentDirectory.empty())
                {
                    std::error_code directoryIteratorErrorCode;
                    for (auto iter : std::filesystem::directory_iterator(m_currentDirectory, directoryIteratorErrorCode))
                    {
                        IS_PROFILE_SCOPE("Entry");

                        std::string path = iter.path().string();
                        FileSystem::PathToUnix(path);
                        std::string fileName = iter.path().filename().string();
                        std::string fileExtension = iter.path().extension().string();

                        //Runtime::IResource* contentResource = nullptr;
                        Ref<Runtime::Asset> contentResource = nullptr;
                        const Runtime::AssetInfo* assetInfo;      
                        {
                            IS_PROFILE_SCOPE("GetAssetInfo");
                            assetInfo = Runtime::AssetRegistry::Instance().GetAssetInfo(path);
                        }

                        if (iter.is_regular_file())
                        {
                            if (assetInfo == nullptr
                                || FileSystem::GetExtension(assetInfo->GetFullFilePath()) == Runtime::AssetMetaData::c_FileExtension)
                            {
                                continue;
                            }
                            else
                            {
                                //contentResource = Runtime::ResourceManager::Instance().LoadSync(path, false);
                                contentResource = Runtime::AssetRegistry::Instance().LoadAsset(path);
                            }
                        }

                        ++displayed_item_count;
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
                                IS_PROFILE_SCOPE("Button");
                                rect_button = ImRect
                                (
                                    ImGui::GetCursorScreenPos().x,
                                    ImGui::GetCursorScreenPos().y,
                                    ImGui::GetCursorScreenPos().x + itemSize.x,
                                    ImGui::GetCursorScreenPos().y + itemSize.y
                                );

                                rect_label = ImRect
                                (
                                    rect_button.Min.x,
                                    rect_button.Max.y - label_height - style.FramePadding.y,
                                    rect_button.Max.x,
                                    rect_button.Max.y
                                );
                            }

                            // Drop shadow effect
                            if (true)
                            {
                                static const float shadow_thickness = 2.0f;
                                ImVec4 color = ImGui::GetStyle().Colors[ImGuiCol_BorderShadow];
                                ImGui::GetWindowDrawList()->AddRectFilled(rect_button.Min, ImVec2(rect_label.Max.x + shadow_thickness, rect_label.Max.y + shadow_thickness), IM_COL32(color.x * 255, color.y * 255, color.z * 255, color.w * 255));
                            }

                            // THUMBNAIL
                            {
                                if (auto thumbNailIter = Algorithm::VectorFindIf(m_contentThumbnail, [&contentResource](const IContentThumbnail* contentThumbnail)
                                    {
                                        if (contentResource)
                                        {
                                            const Reflect::Type assetType = contentResource->GetTypeInfo().GetType();
                                            const Reflect::Type thumbnailType = contentThumbnail->GetAssetTypeInfo().GetType();
                                            return assetType == thumbnailType;
                                        }
                                        return false;
                                    }); thumbNailIter != m_contentThumbnail.end())
                                {
                                    IContentThumbnail* contentThumbnail = (*thumbNailIter);
                                    contentThumbnail->ItemSize = itemSize;
                                    contentThumbnail->RectSize = rect_button;
                                    contentThumbnail->Draw(contentResource.Ptr(), *this);

                                    itemSelectedThisFrame = contentThumbnail->ItemSelected;
                                }
                                else
                                {
                                    IS_PROFILE_SCOPE("THUMBNAIL");
                                    ImGui::PushID(fileName.c_str());
                                    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
                                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 0.25f));

                                    if (ImGui::Button("##dummy", itemSize))
                                    {
                                        m_lastClickTimer.Stop();
                                        float elapsedTime = m_lastClickTimer.GetElapsedTimeMillFloat();
                                        m_lastClickTimer.Start();
                                        const bool isSingleClick = elapsedTime > 0.5f;

                                        if (isSingleClick)
                                        {
                                            m_currentItemSelected = path;
                                            itemSelectedThisFrame = true;
                                        }
                                        else
                                        {
                                            if (iter.is_directory())
                                            {
                                                m_currentDirectory = path;
                                                FileSystem::PathToUnix(m_currentDirectory);
                                                SplitDirectory();
                                            }
                                            else
                                            {

                                                AssetInspectorWindow* assetInspectorWindow = static_cast<AssetInspectorWindow*>(EditorWindowManager::Instance().GetActiveWindow(AssetInspectorWindow::WINDOW_NAME));
                                                if (assetInspectorWindow)
                                                {
                                                    assetInspectorWindow->SetSelectedAssetInfo(assetInfo);
                                                }

                                                std::vector<IObject*> objects = Runtime::AssetRegistry::Instance().GetObjectsFromAsset(assetInfo->Guid);
                                                if (!objects.empty())
                                                {

                                                }
                                            }
                                        }
                                    }

                                    // Image
                                    {
                                        IS_PROFILE_SCOPE("Image");
                                        // Compute thumbnail size
                                        Graphics::RHI_Texture* texture = nullptr;
                                        if (assetInfo != nullptr)
                                        {
                                            Ref<Runtime::Asset> asset = Runtime::AssetRegistry::Instance().LoadAsset(path);
                                            if (Ref<Runtime::TextureAsset> textureAsset = asset.As<Runtime::TextureAsset>())
                                            {
                                                texture = textureAsset->GetRHITexture();
                                            }
                                        }

                                        if (texture == nullptr)
                                        {
                                            Ref<Runtime::Asset> thumbnailTexture = PathToThumbnail(path);
                                            if (thumbnailTexture)
                                            {
                                                texture = PathToThumbnail(path)->GetRHITexture();
                                            }
                                        }
                                        ImVec2 image_size_max = ImVec2(rect_button.Max.x - rect_button.Min.x - style.FramePadding.x * 2.0f, rect_button.Max.y - rect_button.Min.y - style.FramePadding.y - label_height - 5.0f);
                                        ImVec2 image_size = texture ? ImVec2(static_cast<float>(texture->GetWidth()), static_cast<float>(texture->GetHeight())) : image_size_max;
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

                                        // Position the image within the square border
                                        ImGui::SetCursorScreenPos(ImVec2(rect_button.Min.x + style.FramePadding.x + image_size_delta.x * 0.5f, rect_button.Min.y + style.FramePadding.y + image_size_delta.y * 0.5f));

                                        // Draw the image
                                        ImGui::Image(texture, image_size);
                                    }

                                    ImGui::PopStyleColor(2);
                                    ImGui::PopID();
                                }
                            }

                            // LABEL
                            {
                                const char* label_text = fileName.c_str();
                                const ImVec2 label_size = ImGui::CalcTextSize(label_text, nullptr, true);

                                // Draw text background
                                ImGui::GetWindowDrawList()->AddRectFilled(rect_label.Min, rect_label.Max, IM_COL32(51, 51, 51, 190));

                                // Draw text
                                ImGui::SetCursorScreenPos(ImVec2(rect_label.Min.x + text_offset, rect_label.Min.y + text_offset));
                                if (label_size.x <= itemSize.x && label_size.y <= itemSize.y)
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

                        if (contentResource)
                        {
                            EditorGUI::ObjectFieldSource(c_ContentWindowAssetDragSource
                                , contentResource->GetGuid().ToString().data()
                                , Runtime::Asset::GetStaticTypeInfo().GetType());
                        }
                        else if (assetInfo)
                        {
                            EditorGUI::ObjectFieldSource(c_ContentWindowAssetDragSource, assetInfo->Guid.ToString().c_str());
                        }

                        // Item functionality
                        {
                            // Manually detect some useful states
                            if (m_currentItemSelected.empty()
                                && ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly)
                                && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
                            {
                                //m_is_hovering_item = true;
                                //m_hovered_item_path = item.GetPath();
                                IS_LOG_CORE_INFO("Item right click: {}", assetInfo->FileName);
                            }

                            //ItemClick(&item);
                            //ItemContextMenu(&item);
                            //ItemDrag(&item);
                        }

                        // Decide whether we should switch to the next column or switch row
                        pen_x += itemSize.x + ImGui::GetStyle().ItemSpacing.x;
                        if (pen_x >= content_width - itemSize.x)
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

                    if (directoryIteratorErrorCode)
                    {
                        m_currentDirectory = Runtime::ProjectSystem::Instance().GetProjectInfo().GetContentPath();
                        IS_LOG_CORE_ERROR("[ContentWindow::CentreArea] Directory iterator error code '{}'.", directoryIteratorErrorCode.message());
                    }

                    if (!new_line)
                    {
                        ImGui::EndGroup();
                    }
                }
            }
            ImGui::EndChild();

            ImGui::PopStyleColor();
            ImGui::PopStyleVar();

            ImGui::EndTable();

            if (m_showGeneralMenu)
            {
                DrawGeneralMenu();
            }

            if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)
                && IsCursorWithinWindow()
                && !itemSelectedThisFrame)
            {
                m_showGeneralMenu = false;
            }
            else if (ImGui::IsMouseReleased(ImGuiMouseButton_Right)
                && IsCursorWithinWindow()
                && !itemSelectedThisFrame)
            {
                m_showGeneralMenu = true;
            }
        }
    }

    void ContentWindow::BottomBar()
    {
    }

    void ContentWindow::ImportResource()
    {
        /*
        constexpr ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse
            | ImGuiWindowFlags_NoDocking
            //| ImGuiWindowFlags_NoResize
            //| ImGuiWindowFlags_NoMove
            //| ImGuiWindowFlags_AlwaysAutoResize
            ;

        const ImGuiIO& io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowFocus();

        if (ImGui::Begin("Resource type", nullptr, windowFlags))
        {
            std::string_view fileExtension = FileSystem::GetFileExtension(m_importFilePath);
            const Runtime::IResourceLoader* loader = Runtime::ResourceLoaderRegister::GetLoaderFromExtension(fileExtension);
            if (loader)
            {
                const u32 resourceTypeSize = loader->GetResourceTypeIdSize();

                std::vector<std::string> resourceTypeIds;
                resourceTypeIds.reserve(resourceTypeSize);
                for (u32 i = 0; i < resourceTypeSize; ++i)
                {
                    resourceTypeIds.push_back(loader->GetResourceTypeId(i).GetTypeName());
                }

                ImGui::ListBox("Resource Type", &m_resourceTypeToLoadIndex, [](void* data, int idx, const char** outText) -> bool
                    {
                    if (!data)
                    {
                        return false;
                    }
                    const std::vector<std::string>* vec = reinterpret_cast<std::vector<std::string>*>(data);
                    const std::string& str = vec->at(idx);
                    *outText = str.c_str();
                    return true;
                    }, &resourceTypeIds, static_cast<int>(resourceTypeIds.size()));
            }

            ImGui::Checkbox("Convert To Engine Format", &m_convertResourceToEngineFormat);

            if (ImGui::Button("Import"))
            {
                m_showImportWindow = false;

                if (loader)
                {
                    Runtime::ResourceTypeId resourceTypeIdToLoad = loader->GetResourceTypeId(static_cast<u32>(m_resourceTypeToLoadIndex));
                    Runtime::ResourceManager::Instance().LoadSync(Runtime::ResourceId(m_importFilePath, resourceTypeIdToLoad), m_convertResourceToEngineFormat);
                }
                else
                {
                    Runtime::ResourceManager::Instance().LoadSync(Runtime::ResourceId(m_importFilePath, Runtime::ResourceTypeId()), m_convertResourceToEngineFormat);
                }
                m_importFilePath = "";
                m_resourceTypeToLoadIndex = 0;
            }
        }
        ImGui::End();
        */
    }

    void ContentWindow::CreateNewResourceWindow()
    {
        /*
        constexpr ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse
            | ImGuiWindowFlags_NoDocking
            | ImGuiWindowFlags_NoResize
            ;

        const ImGuiIO& io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowFocus();


        // Compute some useful stuff
        const auto window = ImGui::GetCurrentWindowRead();
        const auto content_width = ImGui::GetContentRegionAvail().x;
        const auto content_height = ImGui::GetContentRegionAvail().y - 20.0f;
        ImGuiContext& g = *GImGui;
        ImGuiStyle& style = ImGui::GetStyle();
        const float font_height = g.FontSize;
        const float label_height = font_height;
        const float text_offset = 3.0f;
        ImRect rect_button;
        ImRect rect_label;

        const ImVec2 rowSize = ImVec2(content_width, 128.0f);
        const ImVec2 thumbnailSize = ImVec2(128.0f, rowSize.y);
        const ImVec2 labelSize = ImVec2(rowSize.x - thumbnailSize.x, rowSize.y);

        std::vector<Runtime::ResourceTypeId> resourceTypeIds = Runtime::ResourceRegister::GetAllResourceTypeIds();
        
        bool windowOpen = true;
        if (ImGui::Begin("Create new resource", &windowOpen, windowFlags))
        {
            for (const Runtime::ResourceTypeId& resourceTypeId : resourceTypeIds)
            {
                ImGui::BeginGroup();
                {
                    // Compute rectangles for elements that make up an item
                    {
                        rect_button = ImRect
                        (
                            ImGui::GetCursorScreenPos().x,
                            ImGui::GetCursorScreenPos().y,
                            ImGui::GetCursorScreenPos().x + thumbnailSize.x,
                            ImGui::GetCursorScreenPos().y + thumbnailSize.y
                        );

                        rect_label = ImRect
                        (
                            rect_button.Max.x,
                            rect_button.Min.y,
                            rect_button.Max.x + labelSize.x,
                            rect_button.Max.y
                        );
                    }

                    // Drop shadow effect
                    if (false)
                    {
                        static const float shadow_thickness = 2.0f;
                        ImVec4 color = ImGui::GetStyle().Colors[ImGuiCol_BorderShadow];
                        ImGui::GetWindowDrawList()->AddRectFilled(rect_button.Min, ImVec2(rect_label.Max.x + shadow_thickness, rect_label.Max.y + shadow_thickness), IM_COL32(color.x * 255, color.y * 255, color.z * 255, color.w * 255));
                    }

                    // THUMBNAIL
                    {
                        ImGui::PushID(resourceTypeId.GetTypeName().c_str());
                        //ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
                        //ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 0.25f));

                        bool pushSelectedStyle = m_createResourceSelectedTypeId == resourceTypeId;
                        if (pushSelectedStyle)
                        {
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.35f, 0.0f, 1.0f));
                        }

                        if (ImGui::Button("##dummy", rowSize))
                        {
                            m_createResourceSelectedTypeId = resourceTypeId;
                        }

                        if (pushSelectedStyle)
                        {
                            ImGui::PopStyleColor();
                        }

                        // Image
                        {
                            // Compute thumbnail size
                            Graphics::RHI_Texture* texture = nullptr;
                            Runtime::ResourceId textureResourceId(EnginePaths::GetResourcePath() + "/Editor/Icons/File.png", Runtime::Texture2D::GetStaticResourceTypeId());
                            texture = Runtime::ResourceManager::Instance().LoadSync(textureResourceId).CastTo<Runtime::Texture2D>().Get()->GetRHITexture();
                            ASSERT(texture);

                            ImVec2 image_size_max = thumbnailSize;
                            ImVec2 image_size = texture ? ImVec2(static_cast<float>(texture->GetWidth()), static_cast<float>(texture->GetHeight())) : image_size_max;
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

                            // Position the image within the square border
                            ImGui::SetCursorScreenPos(ImVec2(rect_button.Min.x + image_size_delta.x * 0.5f, rect_button.Min.y + image_size_delta.y * 0.5f));

                            // Draw the image
                            ImGui::Image(texture, image_size);
                        }

                        //ImGui::PopStyleColor(2);
                        ImGui::PopID();
                    }

                    // LABEL
                    {
                        const std::string label_text = resourceTypeId.GetTypeName();
                        const ImVec2 label_size = ImGui::CalcTextSize(label_text.c_str(), nullptr, true);

                        // Draw text background
                        ImGui::GetWindowDrawList()->AddRectFilled(rect_label.Min, rect_label.Max, IM_COL32(51, 51, 51, 190));

                        // Draw text
                        ImGui::SetCursorScreenPos(ImVec2(rect_label.Min.x + text_offset, rect_label.Min.y + text_offset));
                        if (label_size.x <= labelSize.x && label_size.y <= labelSize.y)
                        {
                            ImGui::TextUnformatted(label_text.c_str());
                        }
                        else
                        {
                            ImGui::RenderTextClipped(rect_label.Min, rect_label.Max, label_text.c_str(), nullptr, &label_size, ImVec2(0, 0), &rect_label);
                        }
                    }

                    ImGui::EndGroup();
                }
            }

            if (m_createResourceSelectedTypeId)
            {
                if (ImGui::Button("Create"))
                {
                    std::string filePath;
                    PlatformFileDialog fileDialog;

                    std::wstring resourceTypename = Platform::WStringFromString(m_createResourceSelectedTypeId.GetTypeName());
                    std::wstring extension = Platform::WStringFromString(m_createResourceSelectedTypeId.GetExtension());

                    if (fileDialog.ShowSave(&filePath, { FileDialogFilter(resourceTypename.c_str(), extension.c_str())}))
                    {
                        Runtime::ResourceManager::Instance().Create(Runtime::ResourceId(filePath, m_createResourceSelectedTypeId));
                    }
                }
            }
        }
        ImGui::End();

        if (!windowOpen)
        {
            m_showCreateResourceWindow = false;
        }
        */
    }

    void ContentWindow::SplitDirectory()
    {
        IS_PROFILE_FUNCTION();

        m_currentDirectoryParents.clear();
        std::string directory = m_currentDirectory;
        u64 splitIndex = directory.find('/');
        while (splitIndex != std::string::npos)
        {
            std::string splitDirectory = directory.substr(0, splitIndex);
            m_currentDirectoryParents.push_back(splitDirectory);
            directory = directory.substr(splitIndex + 1);
            splitIndex = directory.find('/');
        }

        if (!directory.empty())
        {
            m_currentDirectoryParents.push_back(directory);
        }
    }

    void ContentWindow::SetDirectoryFromParent(u32 parentIndex)
    {
        std::string newDirectory;
        for (size_t i = 0; i <= parentIndex; ++i)
        {
            newDirectory += m_currentDirectoryParents[i] + "/";
        }

        m_currentDirectory = newDirectory;
        SplitDirectory();
    }

    Ref<Runtime::TextureAsset> ContentWindow::PathToThumbnail(std::string const& path)
    {
        std::string_view pathExtension = FileSystem::GetExtension(path);
        if (auto iter = m_fileExtensionToTexture.find(std::string(pathExtension)); iter != m_fileExtensionToTexture.end())
        {
            return iter->second;
        }

        if (std::filesystem::is_directory(path))
        {
            return m_thumbnailToTexture[ContentWindowThumbnailType::Folder];
        }
        return m_thumbnailToTexture[ContentWindowThumbnailType::File];
    }

    void ContentWindow::DrawGeneralMenu()
    {
        ImGui::OpenPopup("##ContentWindowGeneralMenu");
        if (ImGui::BeginPopupContextItem("##ContentWindowGeneralMenu"))
        {
            if (ImGui::MenuItem("Create New C++ Class"))
            {
                IS_LOG_INFO("Create new C++ Class");
                m_showGeneralMenu = false;
            }
            else if (ImGui::MenuItem("Create World"))
            {
                m_showGeneralMenu = false;

                TObjectPtr<Runtime::World> world = Runtime::WorldSystem::Instance().CreateWorld("New World");
                //world->SaveWorld(m_currentDirectory + "/" + world->GetFileName() + Runtime::World::c_FileExtension);
                Runtime::WorldSystem::Instance().RemoveWorld(world);
            }
            ImGui::EndPopup();
        }
    }
}