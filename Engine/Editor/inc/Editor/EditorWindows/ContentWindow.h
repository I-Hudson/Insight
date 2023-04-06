#pragma once

#include "Editor/EditorWindows/IEditorWindow.h"

#include "Graphics/RHI/RHI_Texture.h"

#include "Core/Timer.h"

#include <vector>
#include <string>

namespace Insight
{
    namespace Runtime
    {
        class Texture2D;
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

        class ContentWindow : public IEditorWindow
        {
        public:
            ContentWindow();
            ContentWindow(u32 minWidth, u32 minHeight);
            ContentWindow(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight);
            virtual ~ContentWindow() override;

            EDITOR_WINDOW(ContentWindow, EditorWindowCategories::Windows);

            virtual void OnDraw() override;

        private:
            void Setup();

            void TopBar();
            void CentreArea();
            void BottomBar();

            void SplitDirectory();
            void SetDirectoryFromParent(u32 parentIndex);

            Runtime::Texture2D* PathToThumbnail(std::string const& path);

        private:
            std::unordered_map<ContentWindowThumbnailType, Runtime::Texture2D*> m_thumbnailToTexture;

            Core::Timer m_lastClickTimer;
            std::string itemClickedId;

            std::vector<std::string> m_currentDirectoryParents;
            std::string m_currentDirectory;
            std::string m_currentItemSelected;

            std::string m_importFilePath;
            bool m_displayResourceTypeToload = false;
            int m_resourceTypeToLoadIndex = 0;
        };
    }
}