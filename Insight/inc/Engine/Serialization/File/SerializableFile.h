#pragma once
#include "Engine/Core/Common.h"
#include "SerializableElementTypes.h"
#include <glm/glm.hpp>

namespace Serialization
{
	class SerializableElement;
	class SerializableFile_XML;

	class IS_API SerializableFile
	{
	public:
		virtual ~SerializableFile();

		SerializableElement* GetNewElement(const std::string& name);

		static SerializableFile* New();

		virtual void SerializeData(const std::string& fileName) = 0;
		virtual void DeserializeData(const std::string& fileName) = 0;
		virtual bool SaveFile(const std::string& fileName) = 0;
		virtual bool LoadFile(const std::string& fileName) = 0;

		SerializableElement* GetFirstChild();
		SerializableElement* GetLastChild();

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

		SerializableElement* GetFirstChild(const std::string& childName);
		SerializableElement* GetFirstChild();
		SerializableElement* GetLastChild();

		SerializableElement* PreviousSibling() const { return m_previousSibling; }
		SerializableElement* NextSibling() const { return m_nextSibling; }

		SerializableAttribute* GetFirstAttribute(const std::string& elementKey);

		SerializableElement* AddChild(const std::string& childName);

		void AddAttribute(const std::string& key, const std::string& value);

		bool HasChildren() const;
		bool HasAttributes() const;

		const std::vector<SerializableElement*> GetAllChildren() const { return m_children; }
		const std::vector<SerializableAttribute*> GetAllData() const { return m_attributes; }

	private:
		std::string m_elementName;
		std::vector<SerializableElement*> m_children;
		std::vector<SerializableAttribute*> m_attributes;

		SerializableElement* m_previousSibling;
		SerializableElement* m_nextSibling;

		friend SerializableFile_XML;
	};
}