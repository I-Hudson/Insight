#pragma once


#ifdef IS_EXPORT_INPUT_DLL
#define IS_INPUT __declspec(dllexport)
#else
#define IS_INPUT __declspec(dllimport)
#endif

#pragma warning( disable : 4251 )