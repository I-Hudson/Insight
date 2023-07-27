#include "Editor/ContentFolderListener.h"
#include "Resource/ResourceManager.h"
#include "Asset/AssetRegistry.h"

#include "FileSystem/FileSystem.h"

#include <iostream>

namespace Insight::Editor
{
    void ContentFolderListener::handleFileAction(efsw::WatchID watchid, const std::string& dir,
                       const std::string& filename, efsw::Action action,
                       std::string oldFilename)
    {
        std::string fullPath = dir + filename;
        FileSystem::PathToUnix(fullPath);

        switch (action)
        {
        case efsw::Actions::Add:
        {

            std::cout << "DIR (" << dir << ") FILE (" << filename << ") has event Added"
                << std::endl;

            TObjectPtr<Runtime::IResource> resource = Runtime::ResourceManager::Instance().LoadSync(fullPath, false);
        }
            break;

        case efsw::Actions::Delete:
        {
            std::cout << "DIR (" << dir << ") FILE (" << filename << ") has event Delete"
                << std::endl;

            Runtime::ResourceManager::Instance().Unload(fullPath);
        }
            break;

        case efsw::Actions::Modified:
            std::cout << "DIR (" << dir << ") FILE (" << filename << ") has event Modified"
                << std::endl;
            break;

        case efsw::Actions::Moved:
            std::cout << "DIR (" << dir << ") FILE (" << filename << ") has event Moved from ("
                << oldFilename << ")" << std::endl;
            break;

        default:
            std::cout << "Should never happen!" << std::endl;
        }
    }
}