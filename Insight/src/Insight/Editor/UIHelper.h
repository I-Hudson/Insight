#pragma once

#include "Insight/Core.h"
#include "Insight/UI/UIFlags.h"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include <glm/glm.hpp>

inline bool IS_API DrawBool(const std::string& name, bool* v, uint32_t flags = 0)
{
	return ImGui::Checkbox(name.c_str(), v);
}

inline bool IS_API DrawInt(const std::string& name, int* v, int clampMax = 0, const std::string& format = "%d", uint32_t flags = 0)
{
	if ((flags & UIFlags_ReadOnly) != 0)
	{
		return ImGui::InputInt(name.c_str(), v, 0, 0, ImGuiInputTextFlags_ReadOnly);
	}
	else
	{
		return ImGui::DragInt(name.c_str(), v, 1.0f, 0, clampMax, format.c_str());
	}
}

inline bool IS_API DrawFloat(const std::string& name, float* v, float clampMax = 0, const std::string& format = "%.3f", uint32_t flags = 0)
{
	if ((flags & UIFlags_ReadOnly) != 0)
	{
		return ImGui::InputFloat(name.c_str(), v, 0.0f, 0.0f, format.c_str(), ImGuiInputTextFlags_ReadOnly);
	}
	else
	{
		return ImGui::DragFloat(name.c_str(), v, 1.0f, 0, clampMax, format.c_str());
	}
}

inline bool IS_API DrawString(const std::string& name, std::string* v, uint32_t flags = 0)
{
	if ((flags & UIFlags_ReadOnly) != 0)
	{
		return ImGui::InputText(name.c_str(), v, ImGuiInputTextFlags_ReadOnly);
	}
	else
	{
		return ImGui::InputText(name.c_str(), v);
	}
}

inline bool IS_API DrawSingleRowMatrix2(glm::mat2* mat2, const int& rowIndex, const std::string& propertyName, const std::string& rowName, const uint32_t& flags)
{
	float row[2] = { (*mat2)[rowIndex].x, (*mat2)[rowIndex].y, };
	std::string label = propertyName + rowName;
	bool r = false;
	if ((flags & UIFlags_ReadOnly) != 0)
	{
		ImGui::InputFloat2(label.c_str(), row, "%.3f", ImGuiInputTextFlags_ReadOnly);
	}
	else
	{
		r = ImGui::DragFloat2(label.c_str(), row, 1.0f, 0.0f, (flags & UIFlags_ClampZero) == 1 ? std::numeric_limits<float>::max() : 0.0f);
		(*mat2)[rowIndex].x = row[0]; (*mat2)[rowIndex].y = row[1];
	}
	return r;
}
inline bool IS_API DrawSingleRowMatrix3(glm::mat3* mat3, const int& rowIndex, const std::string& propertyName, const std::string& rowName, const uint32_t& flags)
{
	float row[3] = { (*mat3)[rowIndex].x, (*mat3)[rowIndex].y, (*mat3)[rowIndex].z };
	std::string label = propertyName + rowName;
	bool r = false;
	if ((flags & UIFlags_ReadOnly) != 0)
	{
		ImGui::InputFloat3(label.c_str(), row, "%.3f", ImGuiInputTextFlags_ReadOnly);
	}
	else
	{
		r = ImGui::DragFloat3(label.c_str(), row, 1.0f, 0.0f, (flags & UIFlags_ClampZero) == 1 ? std::numeric_limits<float>::max() : 0.0f);
		(*mat3)[rowIndex].x = row[0]; (*mat3)[rowIndex].y = row[1]; (*mat3)[rowIndex].z = row[2];
	}
	return r;
}
inline bool IS_API DrawSingleRowMatrix4(glm::mat4* mat4, const int& rowIndex, const std::string& propertyName, const std::string& rowName, const uint32_t& flags)
{
	float row[4] = { (*mat4)[rowIndex].x, (*mat4)[rowIndex].y, (*mat4)[rowIndex].z, (*mat4)[rowIndex].w };
	std::string label = propertyName + rowName;
	bool r = false;
	if ((flags & UIFlags_ReadOnly) != 0)
	{
		ImGui::InputFloat4(label.c_str(), row, "%.3f", ImGuiInputTextFlags_ReadOnly);
	}
	else
	{
		r = ImGui::DragFloat4(label.c_str(), row, 1.0f, 0.0f, (flags & UIFlags_ClampZero) == 1 ? std::numeric_limits<float>::max() : 0.0f);
		(*mat4)[rowIndex].x = row[0]; (*mat4)[rowIndex].y = row[1]; (*mat4)[rowIndex].z = row[2]; (*mat4)[rowIndex].w = row[3];
	}
	return r;
}

inline bool IS_API DrawMat2(const std::string& name, glm::mat2* v, uint32_t flags = 0)
{
	bool r = false;
	r |= DrawSingleRowMatrix2(v, 0, name, " X", flags);
	r |= DrawSingleRowMatrix2(v, 1, name, " Y", flags);
	return r;
}
inline bool IS_API DrawMat3(const std::string& name, glm::mat3* v, uint32_t flags = 0)
{
	bool r = false;
	r |= DrawSingleRowMatrix3(v, 0, name, " X", flags);
	r |= DrawSingleRowMatrix3(v, 1, name, " Y", flags);
	r |= DrawSingleRowMatrix3(v, 2, name, " Z", flags);
	return r;
}
inline bool IS_API DrawMat4(const std::string& name, glm::mat4* v, uint32_t flags = 0)
{
	bool r = false;
	r |= DrawSingleRowMatrix4(v, 0, name, " X", flags);
	r |= DrawSingleRowMatrix4(v, 1, name, " Y", flags);
	r |= DrawSingleRowMatrix4(v, 2, name, " Z", flags);
	r |= DrawSingleRowMatrix4(v, 3, name, " W", flags);
	return r;
}