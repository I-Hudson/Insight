
#include "Engine/Core/Object.h"

#include "Engine/Core/Log.h"
#include "Engine/Memory/MemoryManager.h"
#include "Engine/Platform/Platform.h"

Object::Object()
	: m_uuid(Platform::UUIDToString(Platform::GenerateUUID()))
{ }

Object::~Object()
{ }

void Object::SetUUID(const std::string& uuid)
{
	m_uuid = uuid;
}

bool Object::Equals(const Object* pObject) const
{
	if (pObject == nullptr)
	{
		return false;
	}
	return Equals(*pObject);
}

bool Object::Equals(const Object& pObject) const
{
	return this == &pObject;
}