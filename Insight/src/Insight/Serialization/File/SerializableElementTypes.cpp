#include "ispch.h"
#include "SerializableElementTypes.h"
#include "Insight/Serialization/SerializeHelper.h"
#include "glm/gtx/string_cast.hpp"

namespace Insight
{
	namespace Serialization
	{
		/*
		* BASE TYPES
		*/
		std::string SerializableElementString::GetValue()
		{
			return m_data;
		}
		void SerializableElementString::SetKeyData(const std::string& key, const std::string& data)
		{
			m_elmentName = key;
			m_data = data;
		}
		void SerializableElementString::StringToData(const std::string& key, const std::string& dataString)
		{
			m_elmentName = key;
			m_data = dataString;
		}

		std::string SerializableElementInt::GetValue()
		{
			return std::to_string(m_data);
		}
		void SerializableElementInt::SetKeyData(const std::string& key, const I32& data)
		{
			m_elmentName = key;
			m_data = data;
		}
		void SerializableElementInt::StringToData(const std::string& key, const std::string& dataString)
		{
			m_elmentName = key;
			m_data = std::stoi(dataString);
		}

		std::string SerializableElementFloat::GetValue()
		{
			return std::to_string(m_data);
		}
		void SerializableElementFloat::SetKeyData(const std::string& key, const float& data)
		{
			m_elmentName = key;
			m_data = data;
		}
		void SerializableElementFloat::StringToData(const std::string& key, const std::string& dataString)
		{
			m_elmentName = key;
			m_data = std::stof(dataString);
		}

		/*
		* VECTORS
		*/
		std::string SerializableElementVec2::GetValue()
		{
			return glm::to_string(m_data);
		}
		void SerializableElementVec2::SetKeyData(const std::string& key, const glm::vec2& data)
		{
			m_elmentName = key;
			m_data = data;
		}
		void SerializableElementVec2::StringToData(const std::string& key, const std::string& dataString)
		{
			m_elmentName = key;
		}

		std::string SerializableElementVec3::GetValue()
		{
			return glm::to_string(m_data);
		}
		void SerializableElementVec3::SetKeyData(const std::string& key, const glm::vec3& data)
		{
			m_elmentName = key;
			m_data = data;
		}
		void SerializableElementVec3::StringToData(const std::string& key, const std::string& dataString)
		{
			m_elmentName = key;
		}

		std::string SerializableElementVec4::GetValue()
		{
			return glm::to_string(m_data);
		}
		void SerializableElementVec4::SetKeyData(const std::string& key, const glm::vec4& data)
		{
			m_elmentName = key;
			m_data = data;
		}
		void SerializableElementVec4::StringToData(const std::string& key, const std::string& dataString)
		{
			m_elmentName = key;
		}

		/*
		* MATRIXS
		*/
		std::string SerializableElementMat2::GetValue()
		{
			return glm::to_string(m_data);
		}
		void SerializableElementMat2::SetKeyData(const std::string& key, const glm::mat2& data)
		{
			m_elmentName = key;
			m_data = data;
		}
		void SerializableElementMat2::StringToData(const std::string& key, const std::string& dataString)
		{
			m_elmentName = key;
		}

		std::string SerializableElementMat3::GetValue()
		{
			return glm::to_string(m_data);
		}
		void SerializableElementMat3::SetKeyData(const std::string& key, const glm::mat3& data)
		{
			m_elmentName = key;
			m_data = data;
		}
		void SerializableElementMat3::StringToData(const std::string& key, const std::string& dataString)
		{
			m_elmentName = key;
		}

		std::string SerializableElementMat4::GetValue()
		{
			return SerializeHelper::TypeToString<glm::mat4>(m_data); //glm::to_string(m_data);
		}
		void SerializableElementMat4::SetKeyData(const std::string& key, const glm::mat4& data)
		{
			m_elmentName = key;
			m_data = data;
		}
		void SerializableElementMat4::StringToData(const std::string& key, const std::string& dataString)
		{
			m_elmentName = key;
		}
	}
}