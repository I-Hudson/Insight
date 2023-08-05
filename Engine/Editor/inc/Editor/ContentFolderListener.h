#pragma once

#include <efsw/efsw.hpp>

namespace Insight
{
    namespace Runtime
    {
        class AssetPackage;
    }

    namespace Editor
    {
        class ContentFolderListener : public efsw::FileWatchListener
        {
        public:
            efsw::WatchID WatchId;

            void handleFileAction(efsw::WatchID watchid, const std::string& dir,
                                   const std::string& filename, efsw::Action action,
                                   std::string oldFilename) override;

            void SetAssetPackage(Runtime::AssetPackage* assetPackage);

        private:
            Runtime::AssetPackage* m_assetPackage;
        };
    }
}