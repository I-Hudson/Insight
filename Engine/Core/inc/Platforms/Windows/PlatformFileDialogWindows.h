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
            bool ShowSave(std::string* selectedItem, std::string_view folder, bool appendExtension = true);
            /**
             * @brief Show a windows platform common dialog for load use. 
             * @param selectedItem 
             * @return bool
            */
            bool ShowLoad(std::string* selectedItem, std::string_view folder);

            /// @brief Show a windows platform common dialog for save operations.
            ///  'fileFilters' first entry is the display text and second entry is the extension is the format of ".extension". The '.' is not added and must be provided.
            /// @param selectedItem 
            /// @param fileFilters 
            /// @return bool
            bool ShowSave(std::string* selectedItem, std::string_view folder, const std::vector<FileDialogFilter>& fileFilters, bool appendExtension = true);
            /// @brief Show a windows platform common dialog for load operations.
            /// 'fileFilters' first entry is the display text and second entry is the extension is the format of ".extension". The '.' is not added and must be provided.
            /// @param selectedItem 
            /// @param fileFilters 
            /// @return bool
            bool ShowLoad(std::string* selectedItem, std::string_view folder, const std::vector<FileDialogFilter>& fileFilters);

            /// @brief Show a windows platform common dialog for Save/Load use.
            /// @param operation 
            /// @param selectedItem 
            /// @return bool
            bool Show(PlatformFileDialogOperations operation, std::string* selectedItem, std::string_view folder, bool appendExtension);
            /// @brief Show a windows platform common dialog for Save/Load use.
            /// 'fileFilters' first entry is the display text and second entry is the extension is the format of ".extension". The '.' is not added and must be provided.
            /// @param operation 
            /// @param selectedItem 
            /// @param fileFilters 
            /// @return bool
            bool Show(PlatformFileDialogOperations operation, std::string* selectedItem, std::string_view folder, const std::vector<FileDialogFilter>& fileFilters, bool appendExtension);

        private:
            u32 PlatformFileDialogOperationsToFileDialogOptions(PlatformFileDialogOperations operation);
            bool OpenDialog(PlatformFileDialogOperations operation, std::string* selectedItem, std::string_view folder, const std::vector<FileDialogFilter>& fileFilters);
            bool SaveDialog(PlatformFileDialogOperations operation, std::string* selectedItem, const std::string_view folder, const std::vector<FileDialogFilter>& fileFilters, bool appendExtension);
        
            bool ShowDialog(IFileDialog* dialog, PlatformFileDialogOperations operation, std::string* selectedItem, const std::string_view folder, const std::vector<FileDialogFilter>& fileFilters, bool appendExtension);
        };
    }
}
#endif // IS_PLATFORM_WINDOWS