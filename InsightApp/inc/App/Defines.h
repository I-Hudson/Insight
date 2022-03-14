#pragma once

#ifdef IS_EXPORT_APP_DLL
#define IS_APP __declspec(dllexport)
#else
#define IS_APP __declspec(dllimport)
#endif
