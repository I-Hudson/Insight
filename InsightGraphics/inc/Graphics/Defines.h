#pragma once


#ifdef IS_EXPORT_GRAPHICS_DLL
#define IS_GRAPHICS __declspec(dllexport)
#else
#define IS_GRAPHICS __declspec(dllimport)
#endif

#pragma warning( disable : 4251 )