#pragma once

#ifdef IS_EXPORT_DLL
#ifdef IS_EXPORT_PHYSICS_DLL
#define IS_PHYSICS __declspec(dllexport)
#else
#define IS_PHYSICS __declspec(dllimport)
#endif
#else
#define IS_PHYSICS
#endif

//#pragma warning( disable : 4251 )