#include "Editor/FileDialog.h"


namespace Insight
{
    namespace Editor
    {
        FileDialogItem::FileDialogItem(std::string path)
            : m_path(std::move(path))
        {
            m_icon = Runtime::ResourceManager::Instance().Load("./Resources/Icons/Default.png", Runtime::Texture2D::GetStaticResourceTypeId());

        }

        std::string FileDialogItem::GetPath() const
        {
            return m_path;
        }

        std::string FileDialogItem::GetPathName() const
        {
            u64 lastSlashChar = m_path.find_last_of('/');
            if (lastSlashChar == std::string::npos)
            {
                lastSlashChar = 0;
            }
            u64 lastDotChar = m_path.find_last_of('.');
            return m_path.substr(lastSlashChar, lastDotChar - lastSlashChar);
        }

        TObjectPtr<Runtime::Texture2D> FileDialogItem::GetIcon() const
        {
            return m_icon;
        }

        bool FileDialogItem::IsDirectory() const
        {
            return false;
        }
    }
}