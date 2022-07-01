#pragma once


#ifdef IS_EXPORT_ECS_DLL
#define IS_ECS __declspec(dllexport)
#else
#define IS_ECS __declspec(dllimport)
#endif

#pragma warning( disable : 4251 )