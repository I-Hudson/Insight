#pragma once 

#include "Editor/HotReload/HotReloadMetaData.h"

#include <string>

namespace Insight::Editor
{
    /// @brief Hold a reference to a load dynamic library which has been loaded.
    class HotReloadLibrary
    {
    public:
        HotReloadLibrary();
        HotReloadLibrary(const HotReloadLibrary& other) = delete;
        ~HotReloadLibrary();

        HotReloadLibrary& operator=(const HotReloadLibrary& other) = delete;

        void Load(std::string_view libraryPath);
        void Unload();
        bool IsLoaded() const;

        HotReloadMetaData GetMetaData() const;
        std::string_view GetName() const;
        std::string_view GetPath() const;
        std::string_view GetFullPath() const;

    private:
        void* m_libraryHandle = nullptr;
        std::string m_name;
        std::string m_path;
        std::string m_fullPath;
    };
}