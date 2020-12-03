#pragma once
#include "Insight/Core.h"
#include <map>

typedef uint32_t RTTIPropertyEditorFlags;

class RTTIProperty
{
public:
	RTTIProperty(const std::string& typeName, const std::string propertyName, const RTTIPropertyEditorFlags& editorFlags, void* objectPtr)
		: m_typeName(typeName)
		, m_propertyName(propertyName)
		, m_editorFlags(editorFlags)
		, m_objectPtr(objectPtr)
	{  }

	~RTTIProperty()
	{
		m_objectPtr = nullptr;
	}

	void* GetObjectPtr()
	{
		return m_objectPtr;
	}

	template<typename T>
	T& GetPropertyValue()
	{
		return *static_cast<T*>(m_objectPtr);
	}

	const RTTIPropertyEditorFlags GetPropertyEditorFlags()
	{
		return m_editorFlags;
	}

	const std::string GetPropertyName()
	{
		return m_propertyName;
	}

	const std::string GetType()
	{
		return m_typeName;
	}

protected:
	std::string m_typeName;
	std::string m_propertyName;
	RTTIPropertyEditorFlags m_editorFlags;
	void* m_objectPtr;
};