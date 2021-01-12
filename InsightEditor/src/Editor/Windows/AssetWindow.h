#pragma once
#include "Insight/Core/Core.h"
#include "Editor/EditorWindow.h"
#include <filesystem>

namespace Insight
{
    namespace Editor
    {
        class AssetWindow : public EditorWindow
        {
        public:
            AssetWindow(SharedPtr<Module::EditorModule> editorModule);

            virtual void Update(const float& deltaTime) override;

            struct PathInfo
            {
                PathInfo(const std::filesystem::path& path, const bool& isHidden, const bool& expanded)
                    : Path(path)
                    , IsHidden(isHidden) 
                    , Expanded(expanded)
                { }

                void Sort()
                {
                    std::sort(SubPaths.begin(), SubPaths.end(), AlphaSortComparator);
                }

                std::filesystem::path Path;
                bool IsHidden;
                bool Expanded;
                std::vector<std::filesystem::path> SubPaths;
            };

            //Enum used as bit flags.
            enum FilterMode
            {
                FilterMode_Files = 0x01,
                FilterMode_Dirs = 0x02
            };

            enum ColumnLayout
            {
                OneColumn,
                TwoColumn,
            };

        private:
            bool ReadAssetDirectory(const std::string& assetDirectoryPath);
            static bool AlphaSortComparator(const std::filesystem::path& a, const std::filesystem::path& b);
            void DrawFolderTree(const std::filesystem::path& path, const U32& flags);

            bool RenderNavAndSearchBarRegion();
            bool RenderFileListRegion(const ColumnLayout& layout);
            bool RenderInputTextAndExtRegion();
            bool RenderButtonsAndCheckboxRegion();

            bool OnDirClick(const int& idx);
            
        private:
            std::string m_selectedFn;
            std::string m_selectedPath;
            std::string m_ext;    // Store the saved file extension

            PathInfo m_assetRootPath;
            std::unordered_map<std::string, bool> m_expandedPaths;
            std::string m_searchInput;

            ColumnLayout m_layout;
            int m_filterMode, m_colItemsLimit, m_selectedIdx, m_selectedExtIdx;
            float m_colWidth, m_extBoxWidth;
            bool m_showHidden, m_showInputbarCombobox, m_isDir, m_initRead, m_filterDirty, m_validateFile;
        };
    }
}

