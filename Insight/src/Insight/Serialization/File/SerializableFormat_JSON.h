#pragma once

#include "Insight/Core/Core.h"
#include "SerializableFile.h"
#include "json.hpp"

namespace Insight
{
	namespace Serialization
	{
		class IS_API SerializableFile_JSON : public SerializableFile
		{
		public:

		protected:
			virtual void SerializeData(const std::string& fileName) override;
			virtual void DeserializeData(const std::string& fileName) override;
			virtual bool SaveFile(const std::string& fileName) override;
			virtual bool LoadFile(const std::string& fileName) override;

		private: 
			void SerializeElement(nlohmann::json& doc, nlohmann::json* node, SharedPtr<SerializableElement> element);
			void DeserializeElement(nlohmann::json& doc, const nlohmann::json* node, SharedPtr<SerializableElement> element);

			nlohmann::json m_document;
		};
	}
}