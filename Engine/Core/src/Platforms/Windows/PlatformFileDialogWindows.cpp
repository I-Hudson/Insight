#include "Platforms/Windows/PlatformFileDialogWindows.h"

#ifdef IS_PLATFORM_WINDOWS

#include <Windows.h>

#include "Platforms/Platform.h"
#include "Core/Logger.h"
#include "Core/StringUtils.h"

#include <sstream>

namespace Insight
{
    namespace Platforms::Windows
    {
        bool PlatformFileDialogWindows::ShowSave(std::string* selectedItem)
        {
            return ShowSave(selectedItem, {});
        }

        bool PlatformFileDialogWindows::ShowLoad(std::string* selectedItem)
        {
            return ShowLoad(selectedItem, {});
        }

        bool PlatformFileDialogWindows::ShowSave(std::string* selectedItem, const std::vector<std::pair<const char*, const char*>>& fileFilters)
        {
            return Show(PlatformFileDialogOperations::Save, selectedItem, fileFilters);
        }

        bool PlatformFileDialogWindows::ShowLoad(std::string* selectedItem, const std::vector<std::pair<const char*, const char*>>& fileFilters)
        {
            return Show(PlatformFileDialogOperations::Load, selectedItem, fileFilters);
        }

        bool PlatformFileDialogWindows::Show(PlatformFileDialogOperations operation, std::string* selectedItem)
        {
            return Show(operation, selectedItem, {});
        }

        bool PlatformFileDialogWindows::Show(PlatformFileDialogOperations operation, std::string* selectedItem, const std::vector<std::pair<const char*, const char*>>& fileFilters)
        {
            std::string filter;
            // Place '!' instead of '\0' as when appending to the string some '\0' characters are overwritten.
            // This is down to '\0' being the null terminate char so when appending to the string 
            // '\0' is seen as the end on the string, when it's not. So use '!' then just replace '!' with '\0' later.
            filter = "All ()!*.*!";
            for (size_t i = 0; i < fileFilters.size(); ++i)
            {
                std::string displayText = fileFilters.at(i).first;
                RemoveWhilteSpaces(displayText);
                std::string extensionText = fileFilters.at(i).second;
                RemoveWhilteSpaces(extensionText);

                filter += displayText;
                filter += "!";
                filter += extensionText;
                filter += "!";
            }
            filter += "!";
            std::replace(filter.begin(), filter.end(), '!', '\0');

            // common dialog box structure, setting all fields to 0 is important
            OPENFILENAMEA ofn = { 0 };
            CHAR szFile[260] = { 0 };
            // Initialize remaining fields of OPENFILENAME structure
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = NULL;
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = sizeof(szFile);
            ofn.lpstrFilter = filter.c_str();
            ofn.nFilterIndex = 1;
            ofn.lpstrFileTitle = NULL;
            ofn.nMaxFileTitle = 0;
            ofn.lpstrInitialDir = NULL;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

            if (operation == PlatformFileDialogOperations::Load)
            {
                if (GetOpenFileNameA(&ofn) == TRUE)
                {
                    // use ofn.lpstrFile here
                    if (selectedItem)
                    {
                        *selectedItem = szFile;
                    }
                    return true;
                }
            }
            else if (operation == PlatformFileDialogOperations::Save)
            {
                if (GetSaveFileNameA(&ofn) == TRUE)
                {
                    // use ofn.lpstrFile here
                    if (selectedItem)
                    {
                        *selectedItem = szFile;
                    }
                    return true;
                }
            }

            DWORD errorCode = CommDlgExtendedError();
            if (errorCode != 0)
            {
                IS_CORE_ERROR("[PlatformFileDialogWindows::Show] Error code: {}.", errorCode);
            }

            return false;
        }
    }
}

#endif // IS_PLATFORM_WINDOWS