#pragma once

#include "Insight/Core.h"
#include "SerializableFile.h"
#include "tinyxml2.h"

namespace Insight
{
	namespace Serialization
	{
		class IS_API SerializableFile_XML : public SerializableFile
		{
		public:

		protected:
			virtual void SerializeData(const std::string& fileName) override;
			virtual void DeserializeData(const std::string& fileName) override;
			virtual void SaveFile(const std::string& fileName) override;

		private: 
			void SerializeElement(tinyxml2::XMLDocument& doc, tinyxml2::XMLNode* node, SharedPtr<SerializableElement> element);

			tinyxml2::XMLDocument m_document;
		};
	}
}