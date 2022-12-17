#pragma once

#ifdef IS_EXPORT_DLL
#ifdef IS_EXPORT_RUNTIME_DLL
#define IS_RUNTIME __declspec(dllexport)
#else
#define IS_RUNTIME __declspec(dllimport)
#endif
#else
#define IS_RUNTIME
#endif

#pragma warning( disable : 4251 )