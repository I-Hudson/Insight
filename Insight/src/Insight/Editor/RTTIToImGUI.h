#pragma once

#include "Insight/Core.h"
#include "Insight/Log.h"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include <glm/glm.hpp>

template<typename T>
void RTTIPropertyClampZero(void* objectPtr);
template<typename T>
void RTTIPropertyClampArray(T* objectPtr, const int& size, const T& min, const T& max);

bool RTTIPropertyReadOnly(const RTTIPropertyEditorFlags& proertyFlags);

bool RTTIPropertyDrawSingleRowMatrix2(glm::mat2* mat2, const int& rowIndex, const std::string& propertyName, const std::string& rowName, const RTTIPropertyEditorFlags& propertyFlags);
bool RTTIPropertyDrawSingleRowMatrix3(glm::mat3* mat3, const int& rowIndex, const std::string& propertyName, const std::string& rowName, const RTTIPropertyEditorFlags& propertyFlags);
bool RTTIPropertyDrawSingleRowMatrix4(glm::mat4* mat4, const int& rowIndex, const std::string& propertyName, const std::string& rowName, const RTTIPropertyEditorFlags& propertyFlags);

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

	else if (typeName == TYPE_NAME(glm::mat2))
	{
		glm::mat2* data = static_cast<glm::mat2*>(objectPtr);
		bool r = false;
		r |= RTTIPropertyDrawSingleRowMatrix2(data, 0, propertyName, " X", proertyFlags);
		r |= RTTIPropertyDrawSingleRowMatrix2(data, 1, propertyName, " Y", proertyFlags);
		return r;
	}

	else if (typeName == TYPE_NAME(glm::mat3))
	{
		glm::mat3* data = static_cast<glm::mat3*>(objectPtr);
		bool r = false;
		r |= RTTIPropertyDrawSingleRowMatrix3(data, 0, propertyName, " X", proertyFlags);
		r |= RTTIPropertyDrawSingleRowMatrix3(data, 1, propertyName, " Y", proertyFlags);
		r |= RTTIPropertyDrawSingleRowMatrix3(data, 2, propertyName, " Z", proertyFlags);
		return r;
	}

	else if (typeName == TYPE_NAME(glm::mat4))
	{
		glm::mat4* data = static_cast<glm::mat4*>(objectPtr);
		bool r = false;
		r |= RTTIPropertyDrawSingleRowMatrix4(data, 0, propertyName, " X", proertyFlags);
		r |= RTTIPropertyDrawSingleRowMatrix4(data, 1, propertyName, " Y", proertyFlags);
		r |= RTTIPropertyDrawSingleRowMatrix4(data, 2, propertyName, " Z", proertyFlags);
		r |= RTTIPropertyDrawSingleRowMatrix4(data, 3, propertyName, " W", proertyFlags);
		return r;
	}

	else { IS_CORE_ERROR("Type '{0}' does not have a converstion to ImGUI.", typeName); }

	return false;
}

template<typename T>
void RTTIPropertyClampZero(void* objectPtr)
{
	T* t = static_cast<T*>(objectPtr);
	if (*t < 0)
	{
		*t = 0;
	}
}

template<typename T>
void RTTIPropertyClampArray(T* objectPtr, const int& size, const T& min, const T& max)
{
	T* t = static_cast<T*>(objectPtr);
	for (size_t i = 0; i < size; ++i)
	{
		T* v = &t[i];
		if (min != -1 && *v < min)
		{
			*v = min;
		}
		//else if (min != -1 && (*t)[i] > max)
		//{
		//	(*t)[i] = max;
		//}
	}
}

bool RTTIPropertyReadOnly(const RTTIPropertyEditorFlags& proertyFlags)
{
	return (proertyFlags & RTTIPropertyEditorFlags_ReadOnly) != 0 ? true : false;
}

bool RTTIPropertyClampZero(const RTTIPropertyEditorFlags& proertyFlags)
{
	return (proertyFlags & RTTIPropertyEditorFlags_ClampZero) != 0 ? true : false;
}

float RTTITPropertyClampZero(const RTTIPropertyEditorFlags& proertyFlags)
{
	return RTTIPropertyClampZero(proertyFlags) == true ? std::numeric_limits<float>::max() : 0.0f;
}

bool RTTIPropertyDrawSingleRowMatrix2(glm::mat2* mat2, const int& rowIndex, const std::string& propertyName, const std::string& rowName, const RTTIPropertyEditorFlags& propertyFlags)
{
	float row[2] = { (*mat2)[rowIndex].x, (*mat2)[rowIndex].y, };
	std::string label = propertyName + rowName;
	bool r = false;
	if (RTTIPropertyReadOnly(propertyFlags))
	{
		ImGui::InputFloat2(label.c_str(), row, "%.3f", ImGuiInputTextFlags_ReadOnly);
	}
	else
	{
		r = ImGui::DragFloat2(label.c_str(), row, 1.0f, 0.0f, RTTITPropertyClampZero(propertyFlags));
		(*mat2)[rowIndex].x = row[0]; (*mat2)[rowIndex].y = row[1];
	}
	return r;
}


bool RTTIPropertyDrawSingleRowMatrix3(glm::mat3* mat3, const int& rowIndex, const std::string& propertyName, const std::string& rowName, const RTTIPropertyEditorFlags& propertyFlags)
{
	float row[3] = { (*mat3)[rowIndex].x, (*mat3)[rowIndex].y, (*mat3)[rowIndex].z };
	std::string label = propertyName + rowName;
	bool r = false;
	if (RTTIPropertyReadOnly(propertyFlags))
	{
		ImGui::InputFloat3(label.c_str(), row, "%.3f", ImGuiInputTextFlags_ReadOnly);
	}
	else
	{
		r = ImGui::DragFloat3(label.c_str(), row, 1.0f, 0.0f, RTTITPropertyClampZero(propertyFlags));
		(*mat3)[rowIndex].x = row[0]; (*mat3)[rowIndex].y = row[1]; (*mat3)[rowIndex].z = row[2];
	}
	return r;
}

bool RTTIPropertyDrawSingleRowMatrix4(glm::mat4* mat4, const int& rowIndex, const std::string& propertyName, const std::string& rowName, const RTTIPropertyEditorFlags& propertyFlags)
{
	float row[4] = { (*mat4)[rowIndex].x, (*mat4)[rowIndex].y, (*mat4)[rowIndex].z, (*mat4)[rowIndex].w };
	std::string label = propertyName + rowName;
	bool r = false;
	if (RTTIPropertyReadOnly(propertyFlags))
	{
		ImGui::InputFloat4(label.c_str(), row, "%.3f", ImGuiInputTextFlags_ReadOnly);
	}
	else
	{
		r = ImGui::DragFloat4(label.c_str(), row, 1.0f, 0.0f, RTTITPropertyClampZero(propertyFlags));
		(*mat4)[rowIndex].x = row[0]; (*mat4)[rowIndex].y = row[1]; (*mat4)[rowIndex].z = row[2]; (*mat4)[rowIndex].w = row[3];
	}
	return r;
}