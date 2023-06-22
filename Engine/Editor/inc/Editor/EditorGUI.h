#pragma once

#include "Editor/Defines.h"

#include <Reflect.h>

namespace Insight::Editor::EditorGUI
{
    /// @brief Call after any ImGui call to set that rect as a drag/drop source element.
    /// @param id 
    /// @param payload 
    /// @return void
    void IS_EDITOR ObjectFieldSource(const char* id, const char* payload);

    /// @brief Call after any ImGui call to set that rect as a drag/drop target element.
    /// @param id 
    /// @param label 
    /// @param type 
    /// @param dataToSet 
    /// @return void
    void IS_EDITOR ObjectFieldTarget(const char* id, const char* label, Reflect::Type type, void*& dataToSet);
    /// @brief Call after any ImGui call to set that rect as a drag/drop target element.
    /// @tparam T 
    /// @param id 
    /// @param label 
    /// @param type 
    /// @param dataToSet 
    /// @return void
    template<typename T>
    void IS_EDITOR ObjectFieldTarget(const char* id, const char* label, Reflect::Type type, T*& dataToSet)
    {
        void*& voidData = reinterpret_cast<void*&>(dataToSet);
        ObjectFieldTarget(id, label, type, voidData);
    }
}