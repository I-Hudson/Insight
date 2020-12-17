#pragma once

#include "Insight/Core.h"
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

//	static void SerializeMat4(tinyxml2::XMLNode* out, tinyxml2::XMLDocument* doc,
//		const std::string& id, const glm::mat4& mat4)
//	{
//		tinyxml2::XMLElement* ViewMatrixX = doc->NewElement((id + "X").c_str());
//		ViewMatrixX->SetAttribute("x", mat4[0].x); ViewMatrixX->SetAttribute("y", mat4[0].y); ViewMatrixX->SetAttribute("z", mat4[0].z); ViewMatrixX->SetAttribute("w", mat4[0].w);
//		out->InsertEndChild(ViewMatrixX);
//
//		tinyxml2::XMLElement* ViewMatrixY = doc->NewElement((id + "Y").c_str());
//		ViewMatrixY->SetAttribute("x", mat4[1].x); ViewMatrixY->SetAttribute("y", mat4[1].y); ViewMatrixY->SetAttribute("z", mat4[1].z); ViewMatrixY->SetAttribute("w", mat4[1].w);
//		out->InsertEndChild(ViewMatrixY);
//
//		tinyxml2::XMLElement* ViewMatrixZ = doc->NewElement((id + "Z").c_str());
//		ViewMatrixZ->SetAttribute("x", mat4[2].x); ViewMatrixZ->SetAttribute("y", mat4[2].y); ViewMatrixZ->SetAttribute("z", mat4[2].z); ViewMatrixZ->SetAttribute("w", mat4[2].w);
//		out->InsertEndChild(ViewMatrixZ);
//
//		tinyxml2::XMLElement* ViewMatrixW = doc->NewElement((id + "W").c_str());
//		ViewMatrixW->SetAttribute("x", mat4[3].x); ViewMatrixW->SetAttribute("y", mat4[3].y); ViewMatrixW->SetAttribute("z", mat4[3].z); ViewMatrixW->SetAttribute("w", mat4[3].w);
//		out->InsertEndChild(ViewMatrixW);
//	}
//
//	static glm::mat4 DeserializeMat4(tinyxml2::XMLNode* in, const std::string& id)
//	{
//		glm::mat4 resultMat4;
//		resultMat4[0] = ParseVec4FromAttribute(in->FirstChildElement((id + "X").c_str()));
//		resultMat4[1] = ParseVec4FromAttribute(in->FirstChildElement((id + "Y").c_str()));
//		resultMat4[2] = ParseVec4FromAttribute(in->FirstChildElement((id + "Z").c_str()));
//		resultMat4[3] = ParseVec4FromAttribute(in->FirstChildElement((id + "W").c_str()));
//
//		return resultMat4;
//	}
//
//private:
//	static glm::vec4 ParseVec4FromAttribute(tinyxml2::XMLElement* element)
//	{
//		if (element == nullptr)
//		{
//			IS_CORE_ERROR("[SerializeHelper::ParseVec4FromAttribute]: Parse error. Element does not exists.");
//			return glm::vec4();
//		}
//
//		glm::vec4 vec4;
//		vec4.x = element->FloatAttribute("x");
//		vec4.y = element->FloatAttribute("y");
//		vec4.z = element->FloatAttribute("z");
//		vec4.w = element->FloatAttribute("w");
//
//		return vec4;
//	}
};

template<>
std::string SerializeHelper::TypeToString<glm::vec2>(const glm::vec2& data)
{
	return "[" + std::to_string(data.x) + "," + std::to_string(data.y) + "]";
}

template<>
std::string SerializeHelper::TypeToString<glm::vec3>(const glm::vec3& data)
{
	return "[" + std::to_string(data.x) + "," + std::to_string(data.y) + "," + std::to_string(data.z) + "]";
}

template<>
std::string SerializeHelper::TypeToString<glm::vec4>(const glm::vec4& data)
{
	return "[" + std::to_string(data.x) + "," + std::to_string(data.y) + "," + std::to_string(data.z) + "," + std::to_string(data.w) + "]";
}

template<>
std::string SerializeHelper::TypeToString<glm::mat2>(const glm::mat2& data)
{
	return TypeToString(data[0]) + "," + TypeToString(data[1]);
}

template<>
std::string SerializeHelper::TypeToString<glm::mat3>(const glm::mat3& data)
{
	return TypeToString(data[0]) + "," + TypeToString(data[1]) + "," + TypeToString(data[2]);
}

template<>
std::string SerializeHelper::TypeToString<glm::mat4>(const glm::mat4& data)
{
	return TypeToString(data[0]) + "," + TypeToString(data[1]) + "," + TypeToString(data[2]) + "," + TypeToString(data[3]);
}
