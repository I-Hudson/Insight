#pragma once

#include "Editor/EditorWindows/IEditorWindow.h"

#include "Graphics/RHI/RHI_Texture.h"
#include "Core/ReferencePtr.h"
//#include "Asset/Assets/Texture.h"

#include "Core/Timer.h"

#include "Editor/EditorWindows/Generated/ContentWindow_reflect_generated.h"

#include <vector>
#include <string>

namespace Insight
{
    namespace Runtime
    {
        class TextureAsset;
    }

    namespace Editor
    {
        enum ContentWindowThumbnailType
        {
            Folder,
            File,
            Material,
            Model,
            Mesh
        };

        REFLECT_CLASS()
        class ContentWindow : public IEditorWindow
        {
            REFLECT_GENERATED_BODY()

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
        private:
            void TopBar();
            void CentreArea();
            void BottomBar();

            void ImportResource();
            void CreateNewResourceWindow();

            void SplitDirectory();
            void SetDirectoryFromParent(u32 parentIndex);

            Ref<Runtime::TextureAsset> PathToThumbnail(std::string const& path);

            void DrawGeneralMenu();

        private:
            std::unordered_map<ContentWindowThumbnailType, Ref<Runtime::TextureAsset>> m_thumbnailToTexture;
            std::unordered_map<std::string, Ref<Runtime::TextureAsset>> m_fileExtensionToTexture;

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
        };
    }
}