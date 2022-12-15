#pragma once

#include <string>

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

        class FileDialog
        {
        public:
            FileDialog() = default;
            ~FileDialog() = default;

            void Show(const std::string& path, FileDialogOperations operation);

        private:
            void DrawTopBar();
            void DrawContents();
            void DrawBottomBar();

            void UpdateItems();

        private:
            FileDialogOperations m_operation;
            std::vector<FileDialogItem> m_items;
            bool m_isDirty = false;
        };
    }
}