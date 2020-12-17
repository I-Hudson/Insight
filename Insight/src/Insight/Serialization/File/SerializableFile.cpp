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

		SharedPtr<SerializableElement> SerializableFile::GetFirstChild()
		{
			if (m_rootNodes.size() > 0)
			{
				return m_rootNodes.at(0);
			}
			return {};
		}

		SharedPtr<SerializableElement> SerializableFile::GetLastChild()
		{
			if (m_rootNodes.size() > 0)
			{
				return m_rootNodes.at(m_rootNodes.size() - 1);
			}
			return {};
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

		void SerializableElement::AddDataFromType(const std::string& attriKey, const std::string& attriValue, const std::string& attriType)
		{
			const SerializableElementDataType type = static_cast<const SerializableElementDataType>(std::stoi(attriType));
			SharedPtr<SerializableElementType> data;
			switch (type)
			{
			case SerializableElementDataType::String:	data = CreateSharedPtr<SerializableElementString>(); break;
			case SerializableElementDataType::Int:		data = CreateSharedPtr<SerializableElementInt>(); break;
			case SerializableElementDataType::Float:	data = CreateSharedPtr<SerializableElementFloat>(); break;
			case SerializableElementDataType::Vec2:		data = CreateSharedPtr<SerializableElementVec2>(); break;
			case SerializableElementDataType::Vec3:		data = CreateSharedPtr<SerializableElementVec3>(); break;
			case SerializableElementDataType::Vec4:		data = CreateSharedPtr<SerializableElementVec4>(); break;
			case SerializableElementDataType::Mat2:		data = CreateSharedPtr<SerializableElementMat2>(); break;
			case SerializableElementDataType::Mat3:		data = CreateSharedPtr<SerializableElementMat3>(); break;
			case SerializableElementDataType::Mat4:		data = CreateSharedPtr<SerializableElementMat4>(); break;
			}

			data->StringToData(attriKey, attriValue);
			m_dataTypes.push_back(data);
		}

		SharedPtr<SerializableElement> SerializableElement::GetFirstChild()
		{
			if (m_children.size() > 0)
			{
				return m_children.at(0);
			}
			return {};
		}

		SharedPtr<SerializableElement> SerializableElement::GetLastChild()
		{
			if (m_children.size() > 0)
			{
				return m_children.at(m_children.size() - 1);
			}
			return {};
		}

		SharedPtr<SerializableElementType> SerializableElement::GetFirstElement(const std::string& elementKey)
		{
			for (auto& ptr : m_dataTypes)
			{
				if (ptr->GetKey() == elementKey)
				{
					return ptr;
				}
			}
			return {};
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