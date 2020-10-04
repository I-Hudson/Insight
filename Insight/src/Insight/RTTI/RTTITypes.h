#pragma once
#include "Insight/Core.h"
#include <map>

class IRTTIType
{
public:
	IRTTIType(const std::string& typeName, const std::string propertyName, void* objectPtr)
		: m_typeName(typeName)
		, m_propertyName(propertyName)
		, m_objectPtr(objectPtr)
	{  }

	~IRTTIType()
	{
		m_objectPtr = nullptr;
	}

	const void* GetObjectPtr()
	{
		return m_objectPtr;
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
	void* m_objectPtr;
};

class RTTIBoolType : public IRTTIType
{
public:
	RTTIBoolType(const std::string& typeName, const std::string propertyName, void* objectPtr)
		: IRTTIType(typeName, propertyName, objectPtr)
	{ }

	int GetValue()
	{
		return *static_cast<bool*>(m_objectPtr);
	}
};
class RTTIIntType : public IRTTIType
{
public:
	RTTIIntType(const std::string& typeName, const std::string propertyName, void* objectPtr)
		: IRTTIType(typeName, propertyName, objectPtr)
	{ }

	int GetValue()
	{
		return *static_cast<int*>(m_objectPtr);
	}
};

class RTTIFloatType : public IRTTIType
{
public:
	RTTIFloatType(const std::string& typeName, const std::string propertyName, void* objectPtr)
		: IRTTIType(typeName, propertyName, objectPtr)
	{ }

	float GetValue()
	{
		return *static_cast<float*>(m_objectPtr);
	}
};

class RTTIStringType : public IRTTIType
{
public:
	RTTIStringType(const std::string& typeName, const std::string propertyName, void* objectPtr)
		: IRTTIType(typeName, propertyName, objectPtr)
	{ }

	std::string GetValue()
	{
		return *static_cast<std::string*>(m_objectPtr);
	}
};