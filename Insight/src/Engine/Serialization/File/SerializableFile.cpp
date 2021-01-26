#include "ispch.h"
#include "SerializableFile.h"
#include "SerializableElementTypes.h"
#include "SerializableFormat_XML.h"
#include "SerializableFormat_JSON.h"

namespace Serialization
{
	SerializableFile::~SerializableFile()
	{
		for (auto root : m_rootNodes)
		{
			::Delete(root);
		}
	}

	SerializableElement* SerializableFile::GetNewElement(const std::string& name)
	{
		SerializableElement* element = ::New<SerializableElement>(name);
		m_rootNodes.push_back(element);
		return element;
	}

	SerializableFile* SerializableFile::New()
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
		return ::New<SerializableFile_XML>();
	}

	SerializableElement* SerializableFile::GetFirstChild()
	{
		if (m_rootNodes.size() > 0)
		{
			return m_rootNodes.at(0);
		}
		return {};
	}

	SerializableElement* SerializableFile::GetLastChild()
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
			::Delete(child);
		}

		for (auto data : m_attributes)
		{
			::Delete(data);
		}
	}

	SerializableElement* SerializableElement::GetFirstChild(const std::string& childName)
	{
		for (auto child : m_children)
		{
			if (child->GetElementName() == childName)
			{
				return child;
			}
		}
		return {};
	}

	SerializableElement* SerializableElement::GetFirstChild()
	{
		if (m_children.size() > 0)
		{
			return m_children.at(0);
		}
		return {};
	}

	SerializableElement* SerializableElement::GetLastChild()
	{
		if (m_children.size() > 0)
		{
			return m_children.at(m_children.size() - 1);
		}
		return {};
	}

	SerializableAttribute* SerializableElement::GetFirstAttribute(const std::string& elementKey)
	{
		for (auto& ptr : m_attributes)
		{
			if (ptr->GetKey() == elementKey)
			{
				return ptr;
			}
		}
		return {};
	}

	SerializableElement* SerializableElement::AddChild(const std::string& childName)
	{
		SerializableElement* child = ::New<SerializableElement>(childName);
		m_children.push_back(child);
		return child;
	}

	void SerializableElement::AddAttribute(const std::string& key, const std::string& value)
	{
		SerializableAttribute* attribute = ::New<SerializableAttribute>();
		attribute->SetKeyValue(key, value);
		m_attributes.push_back(attribute);
	}

	bool SerializableElement::HasChildren() const
	{
		return m_children.size() != 0;
	}

	bool SerializableElement::HasAttributes() const
	{
		return m_attributes.size() != 0;
	}
}