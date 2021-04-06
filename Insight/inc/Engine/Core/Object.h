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

	virtual void OnCreate() { }
	virtual void OnDestroy() { }
	virtual bool IsValid() { return m_refCount > 0; }

	void SetUUID(const std::string& uuid);
	const std::string& GetUUID() const { return m_uuid; }

	bool Equals(const Object* pObject) const;
	virtual bool Equals(const Object& pObject) const;

	const Type& GetType() const { ASSERT(!m_type.GetTypeName().empty()); return m_type; }

	void RegisterOnDestroyCallback(void* callerClass, ObjectCallback callback);
	void UnregisterOnDestroyCallback(void* callerClass);

	inline void AddRef();
	inline void Release();

	//template<typename T>
	//INLINE T* New()
	//{
	//	IS_CORE_STATIC_ASSERT((std::is_base_of<Object, T>::value), "'T' does not inherit from 'Object'.");
	//	T* ptr = ::New<T>();
	//	Object* objPtr = static_cast<Object*>(ptr);
	//	objPtr->m_type.SetType<T>();
	//	objPtr->OnCreate();
	//	return ptr;
	//}

	//template<typename T, typename... Args>
	//INLINE T* New(Args&&... args)
	//{
	//	IS_CORE_STATIC_ASSERT((std::is_base_of<Object, T>::value), "'T' does not inherit from 'Object'.");
	//	T* ptr = ::New<T>(std::forward<Args>(args)...);
	//	Object* objPtr = static_cast<Object*>(ptr);
	//	objPtr->m_type.SetType<T>();
	//	objPtr->OnCreate();
	//	return ptr;
	//}

protected:
	template<typename T>
	void SetType();

private:
	uint64_t m_refCount;
	Type m_type;
	std::string m_uuid;
	std::unordered_map<void*, ObjectCallback> m_onDestroyCallbacks;
};

#define REG_ON_DESTROY(objectPtr, callerClass, func) Object::RegisterOnDestroyCallback(std::bind(&func, objectPtr, std::placeholders::_1), callerClass)
#define UNREG_ON_DESTROY() Object::UnregisterOnDestroyCallback(this)

template<typename T>
inline void Object::SetType()
{
	m_type.SetType<T>();
}