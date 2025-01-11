#pragma once

#include "Editor/EditorWindows/IEditorWindow.h"

#include "Graphics/RHI/RHI_Texture.h"
#include "Core/ReferencePtr.h"
//#include "Asset/Assets/Texture.h"

#include "Core/Timer.h"

#include <Reflect/ReflectStructs.h>

#include "Editor/EditorWindows/Generated/ContentWindow_reflect_generated.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <vector>
#include <string>

namespace Insight
{
    namespace Runtime
    {
        class TextureAsset;
        class Asset;
    }

    namespace Editor
    {
        class ContentWindow;

        enum ContentWindowThumbnailType
        {
            Folder,
            File,
            Material,
            Model,
            Mesh
        };

        class IContentThumbnail
        {
        public:
            void Draw(const Runtime::Asset* asset, ContentWindow& contentWindow);
            void DrawButton(const Runtime::Asset* asset, ContentWindow& contentWindow);
            void DrawImage(const Runtime::Asset* asset, ContentWindow& contentWindow);

            virtual void DrawContent(const Runtime::Asset* asset, ContentWindow& contentWindow) const = 0;
            virtual Reflect::TypeInfo GetAssetTypeInfo() const = 0;

            ImVec2 ItemSize;
            ImRect RectSize;
            bool ItemSelected = false;
        };
        class ContentThumbnailModelAsset : public IContentThumbnail
        {
        public:
            virtual void DrawContent(const Runtime::Asset* asset, ContentWindow& contentWindow) const override;
            virtual Reflect::TypeInfo GetAssetTypeInfo() const override;
        };

        REFLECT_CLASS();
        class ContentWindow : public IEditorWindow
        {
            REFLECT_GENERATED_BODY();

        public:
            constexpr static const char* c_ContentWindowResourceDragSource = "ConentWindowResourceDragSource";
            constexpr static const char* c_ContentWindowAssetDragSource = "ConentWindowAssetDragSource";

            ContentWindow();
            ContentWindow(u32 minWidth, u32 minHeight);
            ContentWindow(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight);
            virtual ~ContentWindow() override;

            EDITOR_WINDOW(ContentWindow, EditorWindowCategories::Windows);
            // Begin - IEditorWindow
            virtual void Initialise() override;
            virtual void Shutdown() override;
            virtual void OnDraw() override;
            // End - IEditorWindow

            Ref<Runtime::TextureAsset> PathToThumbnail(std::string const& path);

        private:
            void TopBar();
            void CentreArea();
            void BottomBar();

            void ImportResource();
            void CreateNewResourceWindow();

            void SplitDirectory();
            void SetDirectoryFromParent(u32 parentIndex);

            void DrawGeneralMenu();

        private:
            std::unordered_map<ContentWindowThumbnailType, Ref<Runtime::TextureAsset>> m_thumbnailToTexture;
            std::unordered_map<std::string, Ref<Runtime::TextureAsset>> m_fileExtensionToTexture;
            std::vector<IContentThumbnail*> m_contentThumbnail;

            Core::Timer m_lastClickTimer;
            std::string itemClickedId;

            std::vector<std::string> m_currentDirectoryParents;
            std::string m_currentDirectory;
            std::string m_currentItemSelected;

            std::string m_importFilePath;
            int m_resourceTypeToLoadIndex = 0;
            bool m_showImportWindow = false;
            bool m_convertResourceToEngineFormat = false;

            bool m_showCreateResourceWindow = false;

            bool m_showGeneralMenu = false;

            friend IContentThumbnail;
        };
    }
}