#pragma once

#ifdef IS_EXPORT_DLL
#ifdef IS_EXPORT_FILESYSTEM_DLL
#define IS_FILESYSTEM __declspec(dllexport)
#else
#define IS_FILESYSTEM __declspec(dllimport)
#endif
#else
#define IS_FILESYSTEM
#endif

//#pragma warning( disable : 4251 )