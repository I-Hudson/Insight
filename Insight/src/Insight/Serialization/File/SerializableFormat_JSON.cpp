#include "ispch.h"
#include "SerializableFormat_JSON.h"
#include "SerializableElementTypes.h"

namespace Insight
{
	namespace Serialization
	{
		void SerializableFile_JSON::SerializeData(const std::string& fileName)
		{
			for (size_t i = 0; i < m_rootNodes.size(); ++i)
			{
				SerializeElement(m_document, nullptr, m_rootNodes[i]);
			}
		}

		void SerializableFile_JSON::DeserializeData(const std::string& fileName)
		{
			auto root = m_document.begin();
			while (root != m_document.end())
			{
				SharedPtr<SerializableElement> rootElement = CreateSharedPtr<SerializableElement>(root.key());
				m_rootNodes.push_back(rootElement);
				DeserializeElement(m_document, &(*root), rootElement);
				++root;
			}
		}

		bool SerializableFile_JSON::SaveFile(const std::string& fileName)
		{
			m_document.clear();
			SerializeData(fileName);

			std::ofstream ostream;
			ostream.open(fileName + ".json");
			if (!ostream.is_open())
			{
				IS_CORE_ERROR("File: '{0}' could not be opened.", fileName);
				m_document.clear();
				return false;
			}

			ostream << m_document;
			if (!ostream.good())
			{
				IS_CORE_ERROR("JSON saving. Something went wrong.");
			}
			ostream.close();

			return true;
		}

		bool SerializableFile_JSON::LoadFile(const std::string& fileName)
		{
			m_document.clear();

			std::string filePath = fileName;
			auto extension = filePath.find(".json");
			if (extension == std::string::npos)
			{
				filePath.append(".json");
			}

			//tinyxml2::XMLError err = m_document.LoadFile(filePath.c_str());
			if (true)
			{
				IS_CORE_ERROR("File: '{0}' was unable to be opened.", fileName);
				return false;
			}

			DeserializeData(filePath);
			m_document.clear();

			return true;
		}

		void SerializableFile_JSON::SerializeElement(nlohmann::json& doc, nlohmann::json* node, SharedPtr<SerializableElement> element)
		{
			nlohmann::json elementNode;

			nlohmann::json dataTypesArray = elementNode["DataTypes"];
			auto allDataTypes = element->GetAllData();
			for (auto data : allDataTypes)
			{
				dataTypesArray[data->GetKey().c_str()] = data->GetValue();
			}

			nlohmann::json children = elementNode["Children"];
			auto allChildren = element->GetAllChildren();
			for (auto child : allChildren)
			{
				SerializeElement(doc, &children, child);
			} 

			if (node)
			{
				node->push_back(elementNode);
			}
			else
			{
				doc.push_back(elementNode);
			}
		}

		void SerializableFile_JSON::DeserializeElement(nlohmann::json& doc, const nlohmann::json* node, SharedPtr<SerializableElement> element)
		{

		}
	}
}