#include "Platforms/Windows/PlatformFileDialogWindows.h"
#include "FileSystem/FileSystem.h"

#ifdef IS_PLATFORM_WINDOWS

#include <Windows.h>
#include <shobjidl.h>     // for IFileDialogEvents and IFileDialogControlEvents

#include "Platforms/Platform.h"
#include "Core/Logger.h"
#include "Core/StringUtils.h"

#include <sstream>
#include <Shlwapi.h>

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
            return Show(PlatformFileDialogOperations::SaveFile, selectedItem, fileFilters);
        }

        bool PlatformFileDialogWindows::ShowLoad(std::string* selectedItem, const std::vector<std::pair<const char*, const char*>>& fileFilters)
        {
            return Show(PlatformFileDialogOperations::LoadFile, selectedItem, fileFilters);
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

            if (operation == PlatformFileDialogOperations::LoadFile 
                || operation == PlatformFileDialogOperations::SelectFile 
                || operation == PlatformFileDialogOperations::SelectFolder
                || operation == PlatformFileDialogOperations::SelectAll)
            {
                if (OpenDialog(operation, selectedItem, fileFilters))
                {
                    return true;
                }
            }
            else if (operation == PlatformFileDialogOperations::SaveFile)
            {
                if (SaveDialog(operation, selectedItem, fileFilters))
                {
                    return true;
                }
            }
            else
            {
                FAIL_ASSERT();
            }

            DWORD errorCode = CommDlgExtendedError();
            if (errorCode != 0)
            {
                IS_CORE_ERROR("[PlatformFileDialogWindows::Show] Error code: {}.", errorCode);
            }

            return false;
        }

        u32 PlatformFileDialogWindows::PlatformFileDialogOperationsToFileDialogOptions(PlatformFileDialogOperations operation)
        {
            switch (operation)
            {
            case Insight::PlatformFileDialogOperations::LoadFile:     return FOF_FILESONLY;
            case Insight::PlatformFileDialogOperations::SaveFile:     return FOF_FILESONLY;
            case Insight::PlatformFileDialogOperations::SelectFolder: return FOS_PICKFOLDERS;
            case Insight::PlatformFileDialogOperations::SelectFile:   return FOF_FILESONLY;
            case Insight::PlatformFileDialogOperations::SelectAll:    return FOF_FILESONLY | FOS_PICKFOLDERS;
            default:
                break;
            }
            FAIL_ASSERT();
            return 0;
        }

        bool PlatformFileDialogWindows::OpenDialog(PlatformFileDialogOperations operation, std::string* selectedItem, const std::vector<std::pair<const char*, const char*>>& fileFilters)
        {
            IFileOpenDialog* openDialogHandle;
            HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileDialog, reinterpret_cast<void**>(&openDialogHandle));
            ASSERT(SUCCEEDED(hr));

            openDialogHandle->SetOptions(PlatformFileDialogOperationsToFileDialogOptions(operation));

            hr = openDialogHandle->Show(NULL);
            if (hr != S_OK)
            {
                openDialogHandle->Release();
                return false;
            }

            IShellItem* item;
            hr = openDialogHandle->GetResult(&item);
            if (hr != S_OK)
            {
                IS_CORE_ERROR("[PlatformFileDialogWindows::OpenDialog] Unable to get selected item.");
                item->Release();
                openDialogHandle->Release();
                return false;
            }

            PWSTR filePath;
            hr = item->GetDisplayName(SIGDN_FILESYSPATH, &filePath);
            if (hr != S_OK)
            {
                IS_CORE_ERROR("[PlatformFileDialogWindows::OpenDialog] Unable to get selected item display name.");
                item->Release();
                openDialogHandle->Release();
                return false;
            }

            *selectedItem = Platform::StringFromWString(filePath);
            if ((*selectedItem).back() == '\0')
            {
                (*selectedItem).pop_back();
            }
            (*selectedItem) = FileSystem::FileSystem::PathToUnix(*selectedItem);

            item->Release();
            openDialogHandle->Release();

            return true;
        }

        bool PlatformFileDialogWindows::SaveDialog(PlatformFileDialogOperations operation, std::string* selectedItem, const std::vector<std::pair<const char*, const char*>>& fileFilters)
        {
            IFileSaveDialog* saveDialogHandle;
            HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL, IID_IFileDialog, reinterpret_cast<void**>(&saveDialogHandle));
            ASSERT(SUCCEEDED(hr));

            saveDialogHandle->SetOptions(PlatformFileDialogOperationsToFileDialogOptions(operation));

            hr = saveDialogHandle->Show(NULL);
            if (hr != S_OK)
            {
                saveDialogHandle->Release();
                return false;
            }

            IShellItem* item;
            hr = saveDialogHandle->GetResult(&item);
            if (hr != S_OK)
            {
                IS_CORE_ERROR("[PlatformFileDialogWindows::OpenDialog] Unable to get selected item.");
                item->Release();
                saveDialogHandle->Release();
                return false;
            }

            PWSTR filePath;
            hr = item->GetDisplayName(SIGDN_FILESYSPATH, &filePath);
            if (hr != S_OK)
            {
                IS_CORE_ERROR("[PlatformFileDialogWindows::OpenDialog] Unable to get selected item display name.");
                item->Release();
                saveDialogHandle->Release();
                return false;
            }

            *selectedItem = Platform::StringFromWString(filePath);
            if ((*selectedItem).back() == '\0')
            {
                (*selectedItem).pop_back();
            }
            (*selectedItem) = FileSystem::FileSystem::PathToUnix(*selectedItem);

            item->Release();
            saveDialogHandle->Release();

            return true;
        }
    }
}

#endif // IS_PLATFORM_WINDOWS