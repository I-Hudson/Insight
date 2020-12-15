#include "ispch.h"
#include "SerializableFile.h"
#include "SerializableElementTypes.h"
#include "SerializableFormat_XML.h"

namespace Insight
{
	namespace Serialization
	{
		SerializableFile::~SerializableFile()
		{
			for (auto root : m_rootNodes)
			{
				DELETE_ON_HEAP(root);
			}
		}

		SerializableElement* SerializableFile::GetNewElement(const std::string& name)
		{
			SerializableElement* elment = NEW_ON_HEAP(SerializableElement, name);
			m_rootNodes.push_back(elment);
			return elment;
		}

		SerializableFile* SerializableFile::Create()
		{
			/* Read config and choose what to use.
			if (xml)
			{
				return NEW_ON_HEAP(SerializableFileXML);
			}
			else if (json)
			{
				return NEW_ON_HEAP(SerializableFileJSON);
			}*/
			return NEW_ON_HEAP(SerializableFile_XML);
		}

		SerializableElement::SerializableElement(const std::string& elementName)
			: m_elementName(elementName)
		{ }

		SerializableElement::~SerializableElement()
		{
			for (auto child : m_children)
			{
				DELETE_ON_HEAP(child);
			}

			for (auto data : m_dataTypes)
			{
				DELETE_ON_HEAP(data);
			}
		}

		void SerializableElement::AddChild(SerializableElement* child)
		{
			m_children.push_back(child);
		}

		SerializableElement* SerializableElement::AddChild(const std::string& childName)
		{
			SerializableElement* child = NEW_ON_HEAP(SerializableElement, childName);
			m_children.push_back(child);
			return child;
		}

		SerializableElement* SerializableElement::AddString(const std::string& key, const std::string& value)
		{
			SerializableElementString* stringType = NEW_ON_HEAP(SerializableElementString);
			stringType->SetKeyData(key, value);
			m_dataTypes.push_back(stringType);
			return this;
		}

		SerializableElement* SerializableElement::AddInt(const std::string& key, const U32& value)
		{
			SerializableElementInt* stringType = NEW_ON_HEAP(SerializableElementInt);
			stringType->SetKeyData(key, value);
			m_dataTypes.push_back(stringType);
			return this;
		}

		SerializableElement* SerializableElement::AddFloat(const std::string& key, const float& value)
		{
			SerializableElementFloat* stringType = NEW_ON_HEAP(SerializableElementFloat);
			stringType->SetKeyData(key, value);
			m_dataTypes.push_back(stringType);
			return this;
		}

		SerializableElement* SerializableElement::AddVec2(const std::string& key, const glm::vec2& value)
		{
			SerializableElementVec2* stringType = NEW_ON_HEAP(SerializableElementVec2);
			stringType->SetKeyData(key, value);
			m_dataTypes.push_back(stringType);
			return this;
		}

		SerializableElement* SerializableElement::AddVec3(const std::string& key, const glm::vec3& value)
		{
			SerializableElementVec3* stringType = NEW_ON_HEAP(SerializableElementVec3);
			stringType->SetKeyData(key, value);
			m_dataTypes.push_back(stringType);
			return this;
		}

		SerializableElement* SerializableElement::AddVec4(const std::string& key, const glm::vec4& value)
		{
			SerializableElementVec4* stringType = NEW_ON_HEAP(SerializableElementVec4);
			stringType->SetKeyData(key, value);
			m_dataTypes.push_back(stringType);
			return this;
		}

		SerializableElement* SerializableElement::AddMat2(const std::string& key, const glm::mat2& value)
		{
			SerializableElementMat2* stringType = NEW_ON_HEAP(SerializableElementMat2);
			stringType->SetKeyData(key, value);
			m_dataTypes.push_back(stringType);
			return this;
		}

		SerializableElement* SerializableElement::AddMat3(const std::string& key, const glm::mat3& value)
		{
			SerializableElementMat3* stringType = NEW_ON_HEAP(SerializableElementMat3);
			stringType->SetKeyData(key, value);
			m_dataTypes.push_back(stringType);
			return this;
		}

		SerializableElement* SerializableElement::AddMat4(const std::string& key, const glm::mat4& value)
		{
			SerializableElementMat4* stringType = NEW_ON_HEAP(SerializableElementMat4);
			stringType->SetKeyData(key, value);
			m_dataTypes.push_back(stringType);
			return this;
		}

		bool SerializableElement::NoChildren() const
		{
			return m_children.size() == 0;
		}

		bool SerializableElement::NoDataTypes() const
		{
			return m_dataTypes.size() == 0;
		}
	}
}