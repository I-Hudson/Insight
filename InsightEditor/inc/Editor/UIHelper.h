#pragma once

#include "Engine/Core/Common.h"
#include "Engine/Enums/PropertyFlags.h"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include <glm/glm.hpp>
#include <imgui_internal.h>

	namespace Editor
	{
		class UIHelper
		{
			static bool IS_API InternalDrawElement(const std::unordered_map<IS_PropertyFlags, std::function<bool()>>& flagFunctions, const PropertyFlags& flags, const std::string& typeName);

			constexpr static IS_PropertyFlags IS_API InernalResolveFlags(uint32_t flags);

		public:
			/*
			/
			/ Single items
			/
			*/
			static bool IS_API DrawBool(const std::string& name, bool* v, PropertyFlags flags = IS_PropertyFlags::ShowInEditor);

			static bool IS_API DrawInt(const std::string& name, int* v, int clampMax = 0, const std::string& format = "%d", PropertyFlags flags = IS_PropertyFlags::ShowInEditor);

			static bool IS_API DrawFloat(const std::string& name, float* v, float clampMax = 0, const std::string& format = "%.3f", PropertyFlags flags = IS_PropertyFlags::ShowInEditor);

			static bool IS_API DrawString(const std::string& name, std::string* v, PropertyFlags flags = IS_PropertyFlags::ShowInEditor);

			static bool IS_API DrawString(const std::string& name, PropertyFlags flags = IS_PropertyFlags::ShowInEditor);

			static bool IS_API DrawVector(const std::string& name, const int& vecSize, float* floatArray, const float& dragSpeed = 1.0f, const float& floatMax = 0.0f);

			static bool IS_API DrawSingleRowMatrix2(glm::mat2* mat2, const int& rowIndex, const std::string& propertyName, const std::string& rowName, const PropertyFlags& flags);
			static bool IS_API DrawSingleRowMatrix3(glm::mat3* mat3, const int& rowIndex, const std::string& propertyName, const std::string& rowName, const PropertyFlags& flags);
			static bool IS_API DrawSingleRowMatrix4(glm::mat4* mat4, const int& rowIndex, const std::string& propertyName, const std::string& rowName, const PropertyFlags& flags);

			static bool IS_API DrawMat2(const std::string& name, glm::mat2* v, PropertyFlags flags = IS_PropertyFlags::ShowInEditor);
			static bool IS_API DrawMat3(const std::string& name, glm::mat3* v, PropertyFlags flags = IS_PropertyFlags::ShowInEditor);
			static bool IS_API DrawMat4(const std::string& name, glm::mat4* v, PropertyFlags flags = IS_PropertyFlags::ShowInEditor);
		};
	}