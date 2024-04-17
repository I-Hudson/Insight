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

#define EditorVisible "EditorVisible"

// Add this to a component class Reflect properties and this will remove it from the registeration functions "RegisterECSComponents" so you can't 
// create it like a pure vitual/abstract class.
#define IHT_ABSTRACT_COMPONENT IHT_ABSTRACT_COMPONENT

#pragma warning( disable : 4251 )