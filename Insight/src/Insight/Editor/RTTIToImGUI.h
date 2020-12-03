#pragma once

#include "Insight/Core.h"
#include "Insight/Editor/UIHelper.h"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include <glm/glm.hpp>

static bool RTTIToImGUI_Input(void* objectPtr, const std::string& typeName, const std::string& propertyName, const RTTIPropertyEditorFlags& proertyFlags)
{
#define TYPE_NAME(x) typeid(x).name()

	if (typeName == TYPE_NAME(bool))
	{
		return DrawBool(propertyName, static_cast<bool*>(objectPtr));
	}

	else if (typeName == TYPE_NAME(int))
	{
		return DrawInt(propertyName, static_cast<int*>(objectPtr), (proertyFlags & UIFlags_ClampZero) != 0 ? std::numeric_limits<int>::max() : 0);
	}

	else if (typeName == TYPE_NAME(float))
	{
		return DrawFloat(propertyName, static_cast<float*>(objectPtr), (proertyFlags & UIFlags_ClampZero) != 0 ? std::numeric_limits<float>::max() : 0.0f);
	}

	else if (typeName == TYPE_NAME(std::string))
	{
		return DrawString(propertyName.c_str(), static_cast<std::string*>(objectPtr), proertyFlags);
	}

	else if (typeName == TYPE_NAME(glm::mat2))
	{
		glm::mat2* data = static_cast<glm::mat2*>(objectPtr);
		return DrawMat2(propertyName, data, proertyFlags);
	}

	else if (typeName == TYPE_NAME(glm::mat3))
	{
		glm::mat3* data = static_cast<glm::mat3*>(objectPtr);
		return DrawMat3(propertyName, data, proertyFlags);

	}

	else if (typeName == TYPE_NAME(glm::mat4))
	{
		glm::mat4* data = static_cast<glm::mat4*>(objectPtr);
		return DrawMat4(propertyName, data, proertyFlags);
	}

	else { IS_CORE_ERROR("Type '{0}' does not have a converstion to ImGUI.", typeName); }

	return false;
}