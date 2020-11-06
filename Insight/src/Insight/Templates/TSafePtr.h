#pragma once

#include "Insight/Log.h"
#include "Insight/Object.h"

template<typename T>
class TSafePtr
{
public:
	TSafePtr(T* ptr)
		: m_ptr(ptr)
	{
		IS_CORE_STATIC_ASSERT(Insight::Object, T, "'T' is not derived from 'Insight::Object'.");
		static_cast<Insight::Object*>(m_ptr)->AddRef();
	}

	TSafePtr(T& ptr)
		: m_ptr(&ptr)
	{
		IS_CORE_STATIC_ASSERT(Insight::Object, T, "'T' is not derived from 'Insight::Object'.");
		static_cast<Insight::Object*>(m_ptr)->AddRef();
	}

	~TSafePtr()
	{
		Release();
	}

	void operator delete(void* ptr) { Release(); }

	// Release this safe ptr container.
	void Release()
	{
		if (m_ptr != nullptr)
		{
			static_cast<Insight::Object*>(m_ptr)->Release();
			m_ptr = nullptr;
		}
	}

	T* operator ->() const { return m_ptr; }
	operator T*() const { return m_ptr; }

private:
	T* m_ptr;
};