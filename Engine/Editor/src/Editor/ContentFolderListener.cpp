#include "Editor/ContentFolderListener.h"
#include "Asset/AssetRegistry.h"

#include "FileSystem/FileSystem.h"

#include "Core/Profiler.h"

#include <iostream>

namespace Insight::Editor
{
    void ContentFolderListener::handleFileAction(efsw::WatchID watchid, const std::string& dir,
                       const std::string& filename, efsw::Action action,
                       std::string oldFilename)
    {
        std::string fullPath = dir + filename;
        FileSystem::PathToUnix(fullPath);

        std::lock_guard queueedActionsLock(m_queuedActionsMuetx);

        switch (action)
        {
        case efsw::Actions::Add:
        {

            std::cout << "DIR (" << dir << ") FILE (" << filename << ") has event Added"
                << std::endl;

            m_queuedActions.push_back([this, fullPath]()
                {
                    Runtime::AssetRegistry::Instance().AddAssetFromDisk(fullPath, m_assetPackage);
                });
        }
            break;

        case efsw::Actions::Delete:
        {
            std::cout << "DIR (" << dir << ") FILE (" << filename << ") has event Delete"
                << std::endl;

            m_queuedActions.push_back([this, fullPath]()
                {
                    //Runtime::AssetRegistry::Instance().UnloadAsset(fullPath);
                    Runtime::AssetRegistry::Instance().RemoveAsset(fullPath);
                });
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

    void ContentFolderListener::SetAssetPackage(Runtime::IAssetPackage* assetPackage)
    {
        m_assetPackage = assetPackage;
    }

    void ContentFolderListener::Update()
    {
        IS_PROFILE_FUNCTION();

        std::lock_guard queueedActionsLock(m_queuedActionsMuetx);
        for (const auto& action : m_queuedActions)
        {
            action();
        }
        m_queuedActions.clear();
    }
}