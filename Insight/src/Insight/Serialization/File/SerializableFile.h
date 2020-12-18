#pragma once
#include "Insight/Core.h"
#include "Insight/Memory/MemoryManager.h"
#include "SerializableElementTypes.h"
#include <glm/glm.hpp>

namespace Insight
{
	namespace Serialization
	{
		class SerializableElement;
		class SerializableFile_XML;

		class IS_API SerializableFile
		{
		public:
			virtual ~SerializableFile();

			SharedPtr<SerializableElement> GetNewElement(const std::string& name);

			static UniquePtr<SerializableFile> Create();

			virtual void SerializeData(const std::string& fileName) = 0;
			virtual void DeserializeData(const std::string& fileName) = 0;
			virtual bool SaveFile(const std::string& fileName) = 0;
			virtual bool LoadFile(const std::string& fileName) = 0;

			SharedPtr<SerializableElement> GetFirstChild();
			SharedPtr<SerializableElement> GetLastChild();

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

			SharedPtr<SerializableElement> GetFirstChild(const std::string& childName);
			SharedPtr<SerializableElement> GetFirstChild();
			SharedPtr<SerializableElement> GetLastChild();

			WeakPtr<SerializableElement> PreviousSibling() const { return m_previousSibling; }
			WeakPtr<SerializableElement> NextSibling() const { return m_nextSibling; }

			SharedPtr<SerializableAttribute> GetFirstAttribute(const std::string& elementKey);

			SharedPtr<SerializableElement> AddChild(const std::string& childName);

			void AddAttribute(const std::string& key, const std::string& value);

			bool HasChildren() const;
			bool HasAttributes() const;

			const std::vector<SharedPtr<SerializableElement>> GetAllChildren() const { return m_children; }
			const std::vector<SharedPtr<SerializableAttribute>> GetAllData() const { return m_attributes; }

		private:
			std::string m_elementName;
			std::vector<SharedPtr<SerializableElement>> m_children;
			std::vector<SharedPtr<SerializableAttribute>> m_attributes;

			WeakPtr<SerializableElement> m_previousSibling;
			WeakPtr<SerializableElement> m_nextSibling;

			friend SerializableFile_XML;
		};
	}
}