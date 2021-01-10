#pragma once
#include "Insight/Core/Core.h"
#include "Editor/EditorWindow.h"

namespace Insight
{
    namespace Editor
    {
        class AssetWindow : public EditorWindow
        {
        public:
            AssetWindow(SharedPtr<Module::EditorModule> editorModule);

            virtual void Update(const float& deltaTime) override;

            struct Info
            {
                Info(const std::string& name, bool isHidden) : Name(name), IsHidden(isHidden)
                {
                }
                std::string Name;
                bool IsHidden;
            };

            //Enum used as bit flags.
            enum FilterMode
            {
                FilterMode_Files = 0x01,
                FilterMode_Dirs = 0x02
            };

        private:
            void ClearFileList();
            void FilterFiles(const int& filterMode);
            void ParsePathTabs(const std::string& str);

            bool ReadAssetDirectory(const std::string& assetDirectoryPath);
            static bool AlphaSortComparator(const Info& a, const Info& b);

            bool RenderNavAndSearchBarRegion();
            bool RenderFileListRegion();
            bool RenderInputTextAndExtRegion();
            bool RenderButtonsAndCheckboxRegion();

            bool OnDirClick(const int& idx);

#ifdef IS_PLATFORM_WINDOWS
            bool LoadWindowsDrives();
#endif
#ifndef IS_PLATFORM_WINDOWS
            void InitCurrentPath();
#endif
            
        private:
            std::string m_selectedFn;
            std::string m_selectedPath;
            std::string m_ext;    // Store the saved file extension

            std::vector<std::string> m_directoryList;
            std::vector<Info> m_subDirs;
            std::vector<Info> m_subFiles;
            std::string m_currentPath, m_errorMsg, m_errorTitle, m_invfileModalId, m_repfileModalId, m_inputFn;


            std::vector<std::string> m_validExts;
            ImGuiTextFilter m_filter;
            std::string m_validTypes;
            std::vector<const Info*> m_filteredDirs; // Note: We don't need to call delete. It's just for storing filtered items from subdirs and subfiles so we don't use PassFilter every frame.
            std::vector<const Info*> m_filteredFiles;
            std::vector<std::reference_wrapper<std::string>> m_inputcbFilterFiles;

            int m_filterMode, m_colItemsLimit, m_selectedIdx, m_selectedExtIdx;
            float m_colWidth, m_extBoxWidth;
            bool m_showHidden, m_showInputbarCombobox, m_isDir, m_isAppearing, m_filterDirty, m_validateFile;
        };
    }
}

