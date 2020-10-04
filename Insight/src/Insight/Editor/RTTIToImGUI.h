#pragma once

#include "Insight/Core.h"
#include "Insight/Log.h"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

template<typename T>
void RTTIProperrtClampZero(void* objectPtr);

bool RTTIPropertyReadOnly(const RTTIPropertyEditorFlags& proertyFlags);

static bool RTTIToImGUI_Input(void* objectPtr, const std::string& typeName, const std::string& propertyName, const RTTIPropertyEditorFlags& proertyFlags)
{
#define TYPE_NAME(x) typeid(x).name()

	if (typeName == TYPE_NAME(bool))
	{
		return ImGui::Checkbox(propertyName.c_str(), static_cast<bool*>(objectPtr));
	}

	else if (typeName == TYPE_NAME(int))
	{
		bool r = false;
		if (RTTIPropertyReadOnly(proertyFlags))
		{
			ImGui::InputInt(propertyName.c_str(), static_cast<int*>(objectPtr), 0, 0, ImGuiInputTextFlags_ReadOnly);
		}
		else
		{
			return ImGui::DragInt(propertyName.c_str(), static_cast<int*>(objectPtr), 1.0f, 0, (proertyFlags & RTTIPropertyEditorFlags_ClampZero) != 0 ? std::numeric_limits<int>::max() : 0);
		}
		return r;
	}

	else if (typeName == TYPE_NAME(float))
	{
		if (RTTIPropertyReadOnly(proertyFlags))
		{
			ImGui::InputFloat(propertyName.c_str(), static_cast<float*>(objectPtr), 0.0f, 0.0f, "%.3f", ImGuiInputTextFlags_ReadOnly);
		}
		else
		{
			return ImGui::DragFloat(propertyName.c_str(), static_cast<float*>(objectPtr), 1.0f, 0.0f, (proertyFlags & RTTIPropertyEditorFlags_ClampZero) != 0 ? std::numeric_limits<float>::max() : 0.0f);
		}
	}

	else if (typeName == TYPE_NAME(std::string))
	{
		return ImGui::InputText(propertyName.c_str(), static_cast<std::string*>(objectPtr), RTTIPropertyReadOnly(proertyFlags));
	}

	else { IS_CORE_ERROR("Type '{0}' does not have a converstion to ImGUI.", typeName); }

	return false;
}

template<typename T>
void RTTIProperrtClampZero(void* objectPtr)
{
	T* t = static_cast<T*>(objectPtr);
	if (*t < 0)
	{
		*t = 0;
	}
}

bool RTTIPropertyReadOnly(const RTTIPropertyEditorFlags& proertyFlags)
{
	return (proertyFlags & RTTIPropertyEditorFlags_ReadOnly) != 0 ? true : false;
}