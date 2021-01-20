#include "ispch.h"
#include "Insight/Core/Object.h"

#include "Insight/Core/Log.h"
#include "Insight/Memory/MemoryManager.h"
#include "Platform/Platform.h"

	Object::Object()
		: m_refCount(0)
		, m_uuid(Platform::UUIDToString(Platform::GenerateUUID()))
	{
		AddRef();
	}

	Object::~Object()
	{
		for (auto& it : m_onDestroyCallbacks)
		{
			it.second(this);
		}
		m_onDestroyCallbacks.clear();

		Release();
		IS_CORE_ASSERT(m_refCount == 0, "Object's reference count is not 0. Something has not released this.");
	}

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

	void Object::RegisterOnDestroyCallback(void* callerClass, ObjectCallback callback)
	{
		if (m_onDestroyCallbacks.find(callerClass) == m_onDestroyCallbacks.end())
		{
			m_onDestroyCallbacks[callerClass] = callback;
			return;
		}
		IS_CORE_ERROR("[Object::RegisterOnDestroyCallback] Callback has already been registered.");
	}

	void Object::UnregisterOnDestroyCallback(void* callerClass)
	{
		if (m_onDestroyCallbacks.find(callerClass) != m_onDestroyCallbacks.end())
		{
			m_onDestroyCallbacks.erase(callerClass);
		}
	}

	inline void Object::AddRef()
	{
		++m_refCount;
	}

	inline void Object::Release()
	{
		--m_refCount;
	}