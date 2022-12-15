#pragma once

#include "Core/Defines.h"
#include "Core/TypeAlias.h"

#include <imgui.h>

#include <string>
#include <vector>
#include <array>

namespace Insight
{
    namespace Runtime
    {
        class Texture2D;
    }

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
        class FileDialogItem
        {
        public:
            FileDialogItem() = default;
            FileDialogItem(std::string path);

            std::string GetPath() const;
            std::string GetPathName() const;
            Runtime::Texture2D* GetIcon() const;
            bool IsDirectory() const;

        private:
            std::string m_path;
            bool m_isDirectory;
            Runtime::Texture2D* m_icon;
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
            void Backwards();
            void Forwards();

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
            void Update();

        private:
            void DrawTopBar();
            void DrawContents();
            void DrawBottomBar();

            void UpdateItems();

        private:
            std::string m_title;
            ImVec2 m_itemSize = ImVec2(100.0f, 100.0f);
            FileDialogOperations m_operation;
            FileDialogNavigation m_navigation;
            std::vector<FileDialogItem> m_items;
            bool m_isDirty = false;
            bool m_isVisable = false;
        };
    }
}