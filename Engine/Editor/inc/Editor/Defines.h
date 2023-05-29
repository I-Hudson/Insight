#pragma once

/**
* DLL defines.
*/

#ifndef IS_MONOLITH
#ifdef IS_EXPORT_EDITOR_DLL
#define IS_EDITOR __declspec(dllexport)
#else
#define IS_EDITOR __declspec(dllimport)
#endif
#else
#define IS_EDITOR
#endif

#pragma warning( disable : 4251 )