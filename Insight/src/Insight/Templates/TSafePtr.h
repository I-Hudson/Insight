#pragma once

#include "Insight/Log.h"
#include "Insight/Object.h"

template<typename T>
class TSafePtr
{
public:
	TSafePtr()
		: m_ptr(NEW_ON_HEAP(T))
	{
		IS_CORE_STATIC_ASSERT(Insight::Object, T, "'T' is not derived from 'Insight::Object'.");
	}

	TSafePtr(T* ptr)
		: m_ptr(ptr)
	{
		IS_CORE_STATIC_ASSERT(Insight::Object, T, "'T' is not derived from 'Insight::Object'.");
	}

	TSafePtr(T& ptr)
		: m_ptr(&ptr)
	{
		IS_CORE_STATIC_ASSERT(Insight::Object, T, "'T' is not derived from 'Insight::Object'.");
	}

	~TSafePtr()
	{
		Release();
	}

	void operator delete(void* ptr) = delete;

	T* operator=(T* ptr)
	{
		m_ptr = ptr;
		return m_ptr;
	}

	T* operator=(T ptr)
	{
		*m_ptr = ptr;
		return m_ptr;
	}

	// Release this safe ptr container.
	void Release()
	{
		if (m_ptr != nullptr)
		{
			DELETE_ON_HEAP(m_ptr);
		}
	}

	T* Get()
	{
		return m_ptr;
	}

	T* operator ->() const { return m_ptr; }
	//operator T*() const { return m_ptr; }

private:
	T* m_ptr;
};