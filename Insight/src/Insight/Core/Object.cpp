#include "ispch.h"
#include "Insight/Core/Object.h"

#include "Insight/Core/Log.h"
#include "Insight/Memory/MemoryManager.h"

namespace Insight
{
	Object::Object()
		: Insight::UUID()
		, m_refCount(0)
	{
		AddRef();
	}

	Object::~Object()
	{
		for (auto it = m_onDestroyCallbacks.begin(); it != m_onDestroyCallbacks.end(); ++it)
		{
			it->second(this);

			if (m_onDestroyCallbacks.empty())
			{
				break;
			}
		}
		m_onDestroyCallbacks.clear();

		Release();
		IS_CORE_ASSERT(m_refCount == 0, "Object's reference count is not 0. Something has not released this.");
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

	//std::string Object::GetType()
	//{
	//	if (!m_typeName.empty())
	//	{
	//		return m_typeName;
	//	}
	//	return std::string("Object");
	//}

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

	//void Object::operator delete(void* ptr)
	//{
	//	IS_CORE_ERROR("Object of type '{0}' can not use delete operator.", static_cast<Object*>(ptr)->GetType());
	//}
}