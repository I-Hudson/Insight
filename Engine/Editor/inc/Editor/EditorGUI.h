#pragma once

#include "Editor/Defines.h"

#include <Reflect.h>

#include <imgui_internal.h>

namespace Insight::Editor::EditorGUI
{
    namespace Internal
    {
        bool VerifyObjectFieldPayload(std::string& payload, Reflect::Type type);
    }

    /// @brief Call after any ImGui call to set that rect as a drag/drop source element.
    /// @param id 
    /// @param payload 
    /// @return void
    void IS_EDITOR ObjectFieldSource(const char* id, const char* payload, Reflect::Type type = Reflect::Type());

    /// @brief Call after any ImGui call to set that rect as a drag/drop target element.
    /// @param id 
    /// @param label 
    /// @param type 
    /// @param dataToSet 
    /// @return bool
    bool IS_EDITOR ObjectFieldTarget(const char* id, std::string& data, Reflect::Type type = Reflect::Type());
    
    /// @brief 
    /// @param id 
    /// @param data 
    /// @param type 
    /// @return 
    bool IS_EDITOR ObjectFieldTargetCustomRect(const char* id, const ImRect& rect, std::string& data, Reflect::Type type = Reflect::Type());

#if 0
    /// @brief Call after any ImGui call to set that rect as a drag/drop target element.
    /// @tparam T 
    /// @param id 
    /// @param label 
    /// @param type 
    /// @param dataToSet 
    /// @return void
    template<typename T>
    void IS_EDITOR ObjectFieldTarget(const char* id, const char* label, T*& dataToSet, Reflect::Type type = Reflect::Type())
    {
        void*& voidData = reinterpret_cast<void*&>(dataToSet);
        ObjectFieldTarget(id, label, voidData, type);
    }
#endif
}