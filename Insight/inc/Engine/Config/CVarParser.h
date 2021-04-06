#pragma once

#include "Engine/Core/Common.h"
#include <string>
#include <glm/glm.hpp>

	template<typename T>
	class CVarParser
	{
	public:
		inline T Parse(const std::string_view& str)
		{
			T value;
			std::istringstream ifs(str.data());
			ifs >> value;

			return value;
		}
	};

	template<typename T> 
	inline T CVarParseVec(const std::string_view& str)
	{
		T value;
		std::string fValue;
		int index = 0;
		int vIndex = 0;
		auto it = str.begin();

		while (it != str.end())
		{
			if (*it != ',' && !isspace(*it))
			{
				fValue += str.substr(index, 1);
			}
			else if (*it == ',')
			{
				value[vIndex++] = std::stof(fValue);
				fValue = "";
			}
			else
			{
				if (!isspace(*it))
				{
					IS_CORE_ERROR("[CVarParseVec] Parse error. Invalid character '{0}'", *it);
				}
			}
			++index;
			++it;
		}

		value[vIndex] = std::stof(fValue);
		return value;
	}

	template<> inline glm::vec2 CVarParser<glm::vec2>::Parse(const std::string_view& str)
	{
		return CVarParseVec<glm::vec2>(str);
	}
	template<> inline glm::vec3 CVarParser<glm::vec3>::Parse(const std::string_view& str)
	{
		return CVarParseVec<glm::vec3>(str);
	}
	template<> inline glm::vec4 CVarParser<glm::vec4>::Parse(const std::string_view& str)
	{
		return CVarParseVec<glm::vec4>(str);
	}