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
				root.reset();
			}
		}

		SharedPtr<SerializableElement> SerializableFile::GetNewElement(const std::string& name)
		{
			SharedPtr<SerializableElement> element = CreateSharedPtr<SerializableElement>(name);
			m_rootNodes.push_back(element);
			return element;
		}

		UniquePtr<SerializableFile> SerializableFile::Create()
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
			return CreateUniquePtr<SerializableFile_XML>();
		}

		SerializableElement::SerializableElement(const std::string& elementName)
			: m_elementName(elementName)
		{ }

		SerializableElement::~SerializableElement()
		{
			for (auto child : m_children)
			{
				child.reset();
			}

			for (auto data : m_dataTypes)
			{
				data.reset();
			}
		}

		SharedPtr<SerializableElement> SerializableElement::AddChild(const std::string& childName)
		{
			SharedPtr<SerializableElement>child = CreateSharedPtr<SerializableElement>(childName);
			m_children.push_back(child);
			return child;
		}

		void SerializableElement::AddString(const std::string& key, const std::string& value)
		{
			SharedPtr<SerializableElementString> type = CreateSharedPtr<SerializableElementString>();
			type->SetKeyData(key, value);
			m_dataTypes.push_back(type);
		}

		void SerializableElement::AddInt(const std::string& key, const U32& value)
		{
			SharedPtr<SerializableElementInt> type = CreateSharedPtr<SerializableElementInt>();
			type->SetKeyData(key, value);
			m_dataTypes.push_back(type);
		}

		void SerializableElement::AddFloat(const std::string& key, const float& value)
		{
			SharedPtr<SerializableElementFloat> type = CreateSharedPtr<SerializableElementFloat>();
			type->SetKeyData(key, value);
			m_dataTypes.push_back(type);
		}

		void SerializableElement::AddVec2(const std::string& key, const glm::vec2& value)
		{
			SharedPtr<SerializableElementVec2> type = CreateSharedPtr<SerializableElementVec2>();
			type->SetKeyData(key, value);
			m_dataTypes.push_back(type);
		}

		void SerializableElement::AddVec3(const std::string& key, const glm::vec3& value)
		{
			SharedPtr<SerializableElementVec3> type = CreateSharedPtr<SerializableElementVec3>();
			type->SetKeyData(key, value);
			m_dataTypes.push_back(type);
		}

		void SerializableElement::AddVec4(const std::string& key, const glm::vec4& value)
		{
			SharedPtr<SerializableElementVec4> type = CreateSharedPtr<SerializableElementVec4>();
			type->SetKeyData(key, value);
			m_dataTypes.push_back(type);
		}

		void SerializableElement::AddMat2(const std::string& key, const glm::mat2& value)
		{
			SharedPtr<SerializableElementMat2> type = CreateSharedPtr<SerializableElementMat2>();
			type->SetKeyData(key, value);
			m_dataTypes.push_back(type);
		}

		void SerializableElement::AddMat3(const std::string& key, const glm::mat3& value)
		{
			SharedPtr<SerializableElementMat3> type = CreateSharedPtr<SerializableElementMat3>();
			type->SetKeyData(key, value);
			m_dataTypes.push_back(type);
		}

		void SerializableElement::AddMat4(const std::string& key, const glm::mat4& value)
		{
			SharedPtr<SerializableElementMat4> type = CreateSharedPtr<SerializableElementMat4>();
			type->SetKeyData(key, value);
			m_dataTypes.push_back(type);
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