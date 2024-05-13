#pragma once 

#include "Editor/Defines.h"
#include "Core/Defines.h"

#include <Reflect/Reflect.h>

namespace Insight::Editor
{
    /// @brief Contain general information about a loaded dynamic library.
    struct IS_EDITOR HotReloadMetaData
    {
        HotReloadMetaData();
        HotReloadMetaData(const HotReloadMetaData& other) = delete;
        HotReloadMetaData(HotReloadMetaData&& other) NO_EXPECT;
        ~HotReloadMetaData();

        HotReloadMetaData& operator=(const HotReloadMetaData& other) = delete;
        HotReloadMetaData& operator=(HotReloadMetaData&& other) NO_EXPECT;

        std::vector<std::string> EditorWindowNames = { };
        std::vector<std::string> ComponentNames = { };
        std::vector<std::string> TypeDrawerNames = { };

        std::vector<Reflect::TypeInfo> EditorWindowTypeInfos = { };
        std::vector<Reflect::TypeInfo> ComponentTypeInfos = { };
    };
}