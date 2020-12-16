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
				tinyxml2::XMLElement* root = m_document.NewElement("ROOT");
				m_document.InsertEndChild(root);

				SerializeElement(m_document, root, m_rootNodes[i]);
			}
		}

		void SerializableFile_XML::DeserializeData(const std::string& fileName)
		{

		}

		void SerializableFile_XML::SaveFile(const std::string& fileName)
		{
			SerializeData(fileName);

			if (tinyxml2::XMLError err = m_document.SaveFile((fileName + ".xml").c_str()))
			{
				IS_CORE_ASSERT(false, m_document.ErrorIDToName(err));
			}
		}

		void SerializableFile_XML::SerializeElement(tinyxml2::XMLDocument& doc, tinyxml2::XMLNode* node, SharedPtr<SerializableElement> element)
		{
			tinyxml2::XMLElement* elementRootNode = doc.NewElement(element->GetElementName().c_str());
			node->InsertEndChild(elementRootNode);

			if (!element->NoDataTypes())
			{
				auto allDataTypes = element->GetAllData();
				for (auto data : allDataTypes)
				{
					tinyxml2::XMLElement* xmlElement = doc.NewElement(data->GetKey().c_str());
					xmlElement->SetText(data->GetValue().c_str());
					elementRootNode->InsertEndChild(xmlElement);
				}
			}

			if (!element->NoChildren())
			{
				tinyxml2::XMLElement* childRoot = doc.NewElement("Children");
				auto allChildren = element->GetAllChildren();
				for (auto child : allChildren)
				{
					SerializeElement(doc, childRoot, child);
				}
				elementRootNode->InsertEndChild(childRoot);
			}
		}
	}
}