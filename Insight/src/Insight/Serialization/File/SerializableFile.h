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

			SharedPtr<SerializableElement> GetNewElement(const std::string& name);

			static UniquePtr<SerializableFile> Create();

			virtual void SerializeData(const std::string& fileName) = 0;
			virtual void DeserializeData(const std::string& fileName) = 0;
			virtual void SaveFile(const std::string& fileName) = 0;

		protected:
			std::vector<SharedPtr<SerializableElement>> m_rootNodes;
		};

		class IS_API SerializableElement
		{
		public:
			SerializableElement() = delete;
			SerializableElement(const std::string& elementName);
			~SerializableElement();

			const std::string& GetElementName() const { return m_elementName; }

			SharedPtr<SerializableElement> AddChild(const std::string& childName);

			void AddString(const std::string& key, const std::string& value);
			void AddInt(const std::string& key, const U32& value);
			void AddFloat(const std::string& key, const float& value);

			void AddVec2(const std::string& key, const glm::vec2& value);
			void AddVec3(const std::string& key, const glm::vec3& value);
			void AddVec4(const std::string& key, const glm::vec4& value);

			void AddMat2(const std::string& key, const glm::mat2& value);
			void AddMat3(const std::string& key, const glm::mat3& value);
			void AddMat4(const std::string& key, const glm::mat4& value);

			bool NoChildren() const;
			bool NoDataTypes() const;

			const std::vector<SharedPtr<SerializableElement>> GetAllChildren() const { return m_children; }
			const std::vector<SharedPtr<SerializableElementType>> GetAllData() const { return m_dataTypes; }

		protected:
			std::string m_elementName;
			std::vector<SharedPtr<SerializableElement>> m_children;

			std::vector<SharedPtr<SerializableElementType>> m_dataTypes;
		};
	}
}