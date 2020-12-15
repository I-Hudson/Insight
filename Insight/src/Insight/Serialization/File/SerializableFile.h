#pragma once
#include "Insight/Core.h"
#include "Insight/Memory/MemoryManager.h"

#include <glm/glm.hpp>

namespace Insight
{
	namespace Serialization
	{
		class SerializableElement;
		class SerializableElementType;

		class IS_API SerializableFile
		{
		public:
			virtual ~SerializableFile();

			SerializableElement* GetNewElement(const std::string& name);

			static SerializableFile* Create();

			virtual void SerializeData(const std::string& fileName) = 0;
			virtual void DeserializeData(const std::string& fileName) = 0;
			virtual void SaveFile(const std::string& fileName) = 0;

		protected:
			std::vector<SerializableElement*> m_rootNodes;
		};

		class IS_API SerializableElement
		{
		public:
			SerializableElement() = delete;
			SerializableElement(const std::string& elementName);
			~SerializableElement();

			const std::string& GetElementName() const { return m_elementName; }

			void AddChild(SerializableElement* child);
			SerializableElement* AddChild(const std::string& childName);

			SerializableElement* AddString(const std::string& key, const std::string& value);
			SerializableElement* AddInt(const std::string& key, const U32& value);
			SerializableElement* AddFloat(const std::string& key, const float& value);
							   
			SerializableElement* AddVec2(const std::string& key, const glm::vec2& value);
			SerializableElement* AddVec3(const std::string& key, const glm::vec3& value);
			SerializableElement* AddVec4(const std::string& key, const glm::vec4& value);
							   
			SerializableElement* AddMat2(const std::string& key, const glm::mat2& value);
			SerializableElement* AddMat3(const std::string& key, const glm::mat3& value);
			SerializableElement* AddMat4(const std::string& key, const glm::mat4& value);

			bool NoChildren() const;
			bool NoDataTypes() const;

			const std::vector<SerializableElement*>& GetAllChildren() const { return m_children; }
			const std::vector<SerializableElementType*>& GetAllData() const { return m_dataTypes; }

		protected:
			std::string m_elementName;
			std::vector<SerializableElement*> m_children;

			std::vector<SerializableElementType*> m_dataTypes;
		};
	}
}