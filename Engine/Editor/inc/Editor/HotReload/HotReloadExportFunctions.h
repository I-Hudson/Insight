#pragma once

namespace ProjectModule
{
    constexpr const char* c_Initialise = "ProjectModuleInitialise";
    constexpr const char* c_Uninitialise = "ProjectModuleUninitialise";
    constexpr const char* c_GetEditorWindowNames = "ProjectModuleGetEditorWindowNames";
    constexpr const char* c_GetComponentNames = "ProjectModuleGetComponentNames";
    constexpr const char* c_GetTypeDrawerNames = "ProjectModuleGetTypeDrawerNames";
    constexpr const char* c_GetMetaData = "ProjectModuleGetMetaData";
}

namespace EditorWindowRegister
{
    constexpr const char* c_RegisterAllEditorWindows = "RegisterAllEditorWindows";
    constexpr const char* c_UnregisterAllEditorWindows = "UnregisterAllEditorWindows";
    constexpr const char* c_GetAllEditorWindowNames = "GetAllEditorWindowNames";
    constexpr const char* c_GetAllEditorWindowsTypeInfos = "GetAllEditorWindowsTypeInfos";
}

namespace ComponentRegister
{
    constexpr const char* c_RegisterAllComponents = "RegisterAllComponents";
    constexpr const char* c_UnregisterAllComponents = "UnregisterAllComponents";
    constexpr const char* c_GetAllComponentNames = "GetAllComponentNames";
    constexpr const char* c_GetAllComponentTypeInfos = "GetAllComponentTypeInfos";
}

namespace TypeDrawerRegister
{
    constexpr const char* c_RegisterAllTypeDrawers = "RegisterAllTypeDrawers";
    constexpr const char* c_UnregisterAllTypeDrawers = "UnregisterAllTypeDrawers";
    constexpr const char* c_GetAllTypeDrawerNames = "GetAllTypeDrawerNames";
    //constexpr const char* c_GetAllComponentTypeInfos = "GetAllComponentTypeInfos";
}

namespace ResourceRegister
{
    constexpr const char* c_RegisterAllResources = "RegisterAllResources";
    //constexpr const char* c_UnregisterResources = "UnregisterResources";
    //constexpr const char* c_GetAllResourceNames = "GetAllResourceNames";
}