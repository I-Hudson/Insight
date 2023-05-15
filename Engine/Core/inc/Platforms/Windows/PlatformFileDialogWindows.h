#pragma once

#ifdef IS_PLATFORM_WINDOWS

#include "Core/Defines.h"
#include "Core/TypeAlias.h"
#include "Platforms/PlatformFileDialog.h"

#include <string>
#include <vector>

struct IFileDialog;

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
            bool ShowSave(std::string* selectedItem);
            /**
             * @brief Show a windows platform common dialog for load use. 
             * @param selectedItem 
             * @return bool
            */
            bool ShowLoad(std::string* selectedItem);

            /// @brief Show a windows platform common dialog for save operations.
            ///  'fileFilters' first entry is the display text and second entry is the extension is the format of ".extension". The '.' is not added and must be provided.
            /// @param selectedItem 
            /// @param fileFilters 
            /// @return bool
            bool ShowSave(std::string* selectedItem, const std::vector<FileDialogFilter>& fileFilters);
            /// @brief Show a windows platform common dialog for load operations.
            /// 'fileFilters' first entry is the display text and second entry is the extension is the format of ".extension". The '.' is not added and must be provided.
            /// @param selectedItem 
            /// @param fileFilters 
            /// @return bool
            bool ShowLoad(std::string* selectedItem, const std::vector<FileDialogFilter>& fileFilters);

            /// @brief Show a windows platform common dialog for Save/Load use.
            /// @param operation 
            /// @param selectedItem 
            /// @return bool
            bool Show(PlatformFileDialogOperations operation, std::string* selectedItem);
            /// @brief Show a windows platform common dialog for Save/Load use.
            /// 'fileFilters' first entry is the display text and second entry is the extension is the format of ".extension". The '.' is not added and must be provided.
            /// @param operation 
            /// @param selectedItem 
            /// @param fileFilters 
            /// @return bool
            bool Show(PlatformFileDialogOperations operation, std::string* selectedItem, const std::vector<FileDialogFilter>& fileFilters);

        private:
            u32 PlatformFileDialogOperationsToFileDialogOptions(PlatformFileDialogOperations operation);
            bool OpenDialog(PlatformFileDialogOperations operation, std::string* selectedItem, const std::vector<FileDialogFilter>& fileFilters);
            bool SaveDialog(PlatformFileDialogOperations operation, std::string* selectedItem, const std::vector<FileDialogFilter>& fileFilters);
        };
    }
}
#endif // IS_PLATFORM_WINDOWS