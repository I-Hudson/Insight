#pragma once

#ifdef IS_EXPORT_CORE_DLL
#define IS_CORE __declspec(dllexport)
#else
#define IS_CORE __declspec(dllimport)
#endif

// Static array size
#define ARRAY_COUNT(array) (sizeof(array) / sizeof(array[0]))

#pragma warning( disable : 4251 )