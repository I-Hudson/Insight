#pragma once

#include "Editor/Defines.h"

#include "Core/Defines.h"
#include "Core/TypeAlias.h"
#include "Core/Timer.h"
#include "Core/Memory.h"
#include "Core/ReferencePtr.h"

#include "Asset/Assets/Texture.h"

#include <imgui.h>

#include <string>
#include <vector>
#include <array>

namespace Insight
{
    namespace Editor
    {
        enum class FileDialogOperations
        {
            Save,
            Load
        };
        CONSTEXPR const char* FileDialogOperationToString(FileDialogOperations operation);

        /// <summary>
        /// Single item within a folder on the file system (this includes directories and files).
        /// </summary>
        class IS_EDITOR FileDialogItem
        {
        public:
            FileDialogItem() = default;
            FileDialogItem(std::string path);

            void Clicked();
            float GetTimeSinceLastClickMs() const;

            const std::string& GetPath() const;
            const std::string& GetPathName() const;
            Ref<Runtime::TextureAsset> GetIcon() const;
            bool IsDirectory() const;

        private:
            std::string m_path;
            std::string m_pathName;
            bool m_isDirectory;
            Ref<Runtime::TextureAsset> m_icon;

            Core::Timer m_clickedTimer;
            std::chrono::milliseconds m_timeSinceLastClick;
        };

        class FileDialogNavigation
        {
        public:
            FileDialogNavigation() = default;
            ~FileDialogNavigation() = default;

            void SetPath(const std::string& path);
            /// <summary>
            /// Navigate will try and 
            /// </summary>
            /// <param name="path"></param>
            void Navigate(const std::string& path);
            void NavigateFromPathSelection(const u32 index);
            void Backwards();
            void Forwards();

            const std::string& GetPath() const;
            u32 GetPathSectionSize() const;
            const std::vector<std::string>& GetPathSections() const;
            const std::string& GetPathSection(u32 index) const;

        private:
            void SplitPathToSections();
            void UpdateHistory();

        private:
            std::string m_path;
            std::vector<std::string> m_pathSections;

            static const u32 c_HistoryLevelSize = 8;
            std::array<std::string, c_HistoryLevelSize> m_pathHistory;
        };

        class FileDialog
        {
        public:
            FileDialog() = default;
            ~FileDialog() = default;

            void Show(const std::string& path, FileDialogOperations operation);
            bool Update();
            bool Update(std::string* result);

        private:
            void DrawTop();
            void DrawContents();
            void DrawBottom();

            void UpdateItems();

        private:
            std::string m_title;
            ImVec2 m_itemSize = ImVec2(100.0f, 100.0f);
            FileDialogOperations m_operation;
            FileDialogNavigation m_navigation;
            std::vector<FileDialogItem> m_items;

            u32 m_displayItemCount;
            std::string m_inputBox;
            float m_offsetBottom = 0.0f;
            
            bool m_selectionMade = false;

            bool m_isDirty = false;
            bool m_isVisable = false;
        };
    }
}