#pragma once

#ifdef IS_PLATFORM_WINDOWS

#include "Core/Defines.h"
#include "Platforms/PlatformFileDialog.h"

#include <string>
#include <vector>

namespace Insight
{
    namespace Platforms::Windows
    {
        class IS_CORE PlatformFileDialogWindows
        {
        public:
            /**
             * @brief Show a windows platform common dialog for save use. 
             * @param selectedItem 
             * @return bool
            */
            static bool ShowSave(std::string* selectedItem);
            /**
             * @brief Show a windows platform common dialog for load use. 
             * @param selectedItem 
             * @return bool
            */
            static bool ShowLoad(std::string* selectedItem);

            /// @brief Show a windows platform common dialog for save operations.
            ///  'fileFilters' first entry is the display text and second entry is the extension is the format of ".extension". The '.' is not added and must be provided.
            /// @param selectedItem 
            /// @param fileFilters 
            /// @return bool
            static bool ShowSave(std::string* selectedItem, const std::vector<std::pair<const char*, const char*>>& fileFilters);
            /// @brief Show a windows platform common dialog for load operations.
            /// 'fileFilters' first entry is the display text and second entry is the extension is the format of ".extension". The '.' is not added and must be provided.
            /// @param selectedItem 
            /// @param fileFilters 
            /// @return bool
            static bool ShowLoad(std::string* selectedItem, const std::vector<std::pair<const char*, const char*>>& fileFilters);

            /// @brief Show a windows platform common dialog for Save/Load use.
            /// @param operation 
            /// @param selectedItem 
            /// @return bool
            static bool Show(PlatformFileDialogOperations operation, std::string* selectedItem);
            /// @brief Show a windows platform common dialog for Save/Load use.
            /// 'fileFilters' first entry is the display text and second entry is the extension is the format of ".extension". The '.' is not added and must be provided.
            /// @param operation 
            /// @param selectedItem 
            /// @param fileFilters 
            /// @return bool
            static bool Show(PlatformFileDialogOperations operation, std::string* selectedItem, const std::vector<std::pair<const char*, const char*>>& fileFilters);
        };
    }
}
#endif // IS_PLATFORM_WINDOWS