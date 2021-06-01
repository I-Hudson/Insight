#pragma once

#if !defined(IS_STANDARD_POINTER)
#include "Engine/Templates/TPointers.h"
#endif
#include "InsightAlias.h"
#include "Log.h"
#include "Type.h"
#include "Engine/Memory/Memory.h"

class Object;

typedef std::function<void(Object*)> ObjectCallback;

class Object
{
public:
	Object();
	virtual ~Object();

	void SetUUID(const std::string& uuid);
	const std::string& GetUUID() const { return m_uuid; }

	bool Equals(const Object* pObject) const;
	virtual bool Equals(const Object& pObject) const;

	const Type& GetType() const { ASSERT(!m_type.GetTypeName().empty()); return m_type; }

protected:
	template<typename T>
	void SetType();

private:
	Type m_type;
	std::string m_uuid;
};

template<typename T>
inline void Object::SetType()
{
	m_type.SetType<T>();
}