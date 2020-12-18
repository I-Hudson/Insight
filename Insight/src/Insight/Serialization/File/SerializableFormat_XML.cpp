#include "ispch.h"
#include "SerializableFormat_XML.h"
#include "SerializableElementTypes.h"

namespace Insight
{
	namespace Serialization
	{
		void SerializableFile_XML::SerializeData(const std::string& fileName)
		{
			for (size_t i = 0; i < m_rootNodes.size(); ++i)
			{
				SerializeElement(m_document, nullptr, m_rootNodes[i]);
			}
		}

		void SerializableFile_XML::DeserializeData(const std::string& fileName)
		{
			tinyxml2::XMLNode* root = m_document.FirstChild();
			while (root != nullptr)
			{
				SharedPtr<SerializableElement> rootElement = CreateSharedPtr<SerializableElement>(root->Value());
				m_rootNodes.push_back(rootElement);
				DeserializeElement(m_document, root, rootElement);
				root = root->NextSibling();
			}
		}

		bool SerializableFile_XML::SaveFile(const std::string& fileName)
		{
			SerializeData(fileName);

			if (tinyxml2::XMLError err = m_document.SaveFile((fileName + ".xml").c_str()))
			{
				IS_CORE_ERROR(m_document.ErrorIDToName(err));
				return false;
			}
			m_document.Clear();

			return true;
		}

		bool SerializableFile_XML::LoadFile(const std::string& fileName)
		{
			std::string filePath = fileName;
			auto extension = filePath.find(".xml");
			if (extension == std::string::npos)
			{
				filePath.append(".xml");
			}

			tinyxml2::XMLError err = m_document.LoadFile(filePath.c_str());
			if (err != tinyxml2::XMLError::XML_SUCCESS)
			{
				IS_CORE_ERROR(m_document.ErrorIDToName(err));
				return false;
			}

			DeserializeData(filePath);
			m_document.Clear();

			return true;
		}

		void SerializableFile_XML::SerializeElement(tinyxml2::XMLDocument& doc, tinyxml2::XMLNode* node, SharedPtr<SerializableElement> element)
		{
			tinyxml2::XMLElement* elementRootNode = doc.NewElement(element->GetElementName().c_str());
			if (node)
			{
				node->InsertEndChild(elementRootNode);

			}
			else
			{
				doc.InsertEndChild(elementRootNode);
			}

			auto allDataTypes = element->GetAllData();
			for (auto data : allDataTypes)
			{
				elementRootNode->SetAttribute(data->GetKey().c_str(), data->GetValue().c_str());
			}

			auto allChildren = element->GetAllChildren();
			for (auto child : allChildren)
			{
				SerializeElement(doc, elementRootNode, child);
			}

		}

		void SerializableFile_XML::DeserializeElement(tinyxml2::XMLDocument& doc, const tinyxml2::XMLNode* node, SharedPtr<SerializableElement> element)
		{
			// Get all the children.
			const tinyxml2::XMLElement* xmlElement = (tinyxml2::XMLElement*)node;//node->FirstChildElement();
			const tinyxml2::XMLAttribute* attri = xmlElement->FirstAttribute();
			while (attri != nullptr)
			{
				element->AddAttribute(attri->Name(), attri->Value());
				attri = attri->Next();
			}

			const tinyxml2::XMLElement* childElement = xmlElement->FirstChildElement();
			while (childElement != nullptr)
			{
				bool hasChildren = element->HasChildren();
				auto childPtr = element->AddChild(childElement->Name());
			
				if (hasChildren)
				{
					auto elementChildren = element->GetAllChildren();
					childPtr->m_previousSibling = elementChildren[elementChildren.size() - 2];
					elementChildren[elementChildren.size() - 2]->m_nextSibling = childPtr;
				}
				
				DeserializeElement(doc, childElement, childPtr);

				childElement = childElement->NextSiblingElement();
			}
		}
	}
}