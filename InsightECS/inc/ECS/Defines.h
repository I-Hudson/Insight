#pragma once

#ifndef IS_MONOLITH
#ifdef IS_EXPORT_ECS_DLL
#define IS_ECS __declspec(dllexport)
#else
#define IS_ECS __declspec(dllimport)
#endif
#else
#define IS_ECS
#endif

#pragma warning( disable : 4251 )