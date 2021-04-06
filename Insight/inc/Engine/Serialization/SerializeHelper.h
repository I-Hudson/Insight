#pragma once

#include "Engine/Core/Core.h"
#include <glm/glm.hpp>

class SerializeHelper
{
public:
	template<typename T>
	static std::string TypeToString(const T& data)
	{ IS_CORE_STATIC_ASSERT(false, "[SerializeHelper::TypeToString] no implementation."); }

	template<typename T>
	constexpr static T StringToType(const std::string& data)
	{
		IS_CORE_STATIC_ASSERT(false, "[SerializeHelper::StringToType] no implementation.");
	}

private:
	static std::vector<std::string> split(const std::string& string, const char& splitChar)
	{
		std::string::const_iterator sIt = string.begin();
		std::vector<std::string> vecString;
		std::string currentString;
		while (sIt != string.end())
		{
			if (*sIt == splitChar)
			{
				if (!currentString.empty())
				{
					vecString.push_back(currentString);
				}
				currentString = "";
			}
			else
			{
				currentString += *sIt;
			}
			++sIt;
		}

		return vecString;
	}
};

#define ARRAY_SPLIT_CHAR ':'
#define ARRAY_END_CHAR ';'

template<>
std::string SerializeHelper::TypeToString<glm::vec2>(const glm::vec2& data)
{
	return std::to_string(data.x) + ARRAY_SPLIT_CHAR + std::to_string(data.y) + ARRAY_SPLIT_CHAR + ARRAY_END_CHAR;
}

template<>
std::string SerializeHelper::TypeToString<glm::vec3>(const glm::vec3& data)
{
	return std::to_string(data.x) + ARRAY_SPLIT_CHAR + std::to_string(data.y) + ARRAY_SPLIT_CHAR + std::to_string(data.z) + ARRAY_SPLIT_CHAR + ARRAY_END_CHAR;
}

template<>
std::string SerializeHelper::TypeToString<glm::vec4>(const glm::vec4& data)
{
	return std::to_string(data.x) + ARRAY_SPLIT_CHAR + std::to_string(data.y) + ARRAY_SPLIT_CHAR + std::to_string(data.z) + ARRAY_SPLIT_CHAR + std::to_string(data.w) + ARRAY_SPLIT_CHAR + ARRAY_END_CHAR;
}

template<>
std::string SerializeHelper::TypeToString<glm::mat2>(const glm::mat2& data)
{
	return TypeToString(data[0]) + TypeToString(data[1]);
}

template<>
std::string SerializeHelper::TypeToString<glm::mat3>(const glm::mat3& data)
{
	return TypeToString(data[0]) + TypeToString(data[1]) + TypeToString(data[2]);
}

template<>
std::string SerializeHelper::TypeToString<glm::mat4>(const glm::mat4& data)
{
	return TypeToString(data[0]) + TypeToString(data[1]) + TypeToString(data[2]) + TypeToString(data[3]);
}

template<>
glm::vec2 SerializeHelper::StringToType<glm::vec2>(const std::string& string)
{
	const int vectorSize = 2;
	float f[vectorSize];
	std::vector<std::string> vecString = split(string, ARRAY_SPLIT_CHAR);
	std::string::const_iterator stringIt = string.begin();
	for (size_t i = 0; i < vectorSize; ++i)
	{
		std::string floatString;
		while (*stringIt != ARRAY_SPLIT_CHAR)
		{
			floatString += *stringIt;
			++stringIt;
		}
		++stringIt;

		IS_CORE_ASSERT(!floatString.empty(), "[SerializeHelper::StringToType<glm::vec2>] This can't be empty;");
		f[i] = std::stof(floatString);
	}
	return glm::vec2(f[0], f[1]);
}
template<>
glm::vec3 SerializeHelper::StringToType<glm::vec3>(const std::string& string)
{
	const int vectorSize = 3;
	float f[vectorSize];
	std::vector<std::string> vecString = split(string, ARRAY_SPLIT_CHAR);
	std::string::const_iterator stringIt = string.begin();
	for (size_t i = 0; i < vectorSize; ++i)
	{
		std::string floatString;
		while (*stringIt != ARRAY_SPLIT_CHAR)
		{
			floatString += *stringIt;
			++stringIt;
		}
		++stringIt;

		IS_CORE_ASSERT(!floatString.empty(), "[SerializeHelper::StringToType<glm::vec3>] This can't be empty;");
		f[i] = std::stof(floatString);
	}
	return glm::vec3(f[0], f[1], f[2]);
}
template<>
glm::vec4 SerializeHelper::StringToType<glm::vec4>(const std::string& string)
{
	const int vectorSize = 4;
	float f[vectorSize];
	std::vector<std::string> vecString = split(string, ARRAY_SPLIT_CHAR);
	std::string::const_iterator stringIt = string.begin();
	for (size_t i = 0; i < vectorSize; ++i)
	{
		std::string floatString;
		while (*stringIt != ARRAY_SPLIT_CHAR)
		{
			floatString += *stringIt;
			++stringIt;
		}
		++stringIt;
		
		IS_CORE_ASSERT(!floatString.empty(), "[SerializeHelper::StringToType<glm::vec4>] This can't be empty;");
		f[i] = std::stof(floatString);
	}
	return glm::vec4(f[0], f[1], f[2], f[3]);
}

template<>
glm::mat2 SerializeHelper::StringToType<glm::mat2>(const std::string& string)
{
	std::vector<std::string> vecsString = split(string, ARRAY_END_CHAR);

	std::function<glm::vec2(const std::string&)> parseVector = [](const std::string& string)
	{
		return StringToType<glm::vec2>(string);
	};
	return glm::mat2(parseVector(vecsString[0]), parseVector(vecsString[1]));
}
template<>
glm::mat3 SerializeHelper::StringToType<glm::mat3>(const std::string& string)
{
	std::vector<std::string> vecsString = split(string, ARRAY_END_CHAR);

	std::function<glm::vec3(const std::string&)> parseVector = [](const std::string& string)
	{
		return StringToType<glm::vec3>(string);
	};
	return glm::mat3(parseVector(vecsString[0]), parseVector(vecsString[1]), parseVector(vecsString[2]));
}
template<>
glm::mat4 SerializeHelper::StringToType<glm::mat4>(const std::string& string)
{
	std::vector<std::string> vecsString = split(string, ARRAY_END_CHAR);

	std::function<glm::vec4(const std::string&)> parseVector = [](const std::string& string)
	{
		return StringToType<glm::vec4>(string);
	};
	return glm::mat4(parseVector(vecsString[0]), parseVector(vecsString[1]), parseVector(vecsString[2]), parseVector(vecsString[3]));
}
