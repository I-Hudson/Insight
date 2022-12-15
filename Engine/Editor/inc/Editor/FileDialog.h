#pragma once

#include "Core/Memory.h"
#include "Resource/Texture2D.h"

#include <string>

namespace Insight
{
    namespace Editor
    {
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
            TObjectPtr<Runtime::Texture2D> GetIcon() const;
            bool IsDirectory() const;

        private:
            std::string m_path;
            bool m_isDirectory;
            TObjectPtr<Runtime::Texture2D> m_icon;
        };

        class FileDialog
        {
        public:

        private:

        };
    }
}