#pragma once

#include "Insight/Core.h"
#include <type_traits>

template<class T>
struct DefaultDelete
{
	constexpr DefaultDelete() noexcept = default;

	template<class T2, std::enable_if_t<std::is_convertible_v<T2*, T*>, int> = 0>
	DefaultDelete(const DefaultDelete<T2>& defaultDelete) noexcept
	{ }

	void operator()(T* ptr) const noexcept
	{
		IS_CORE_STATIC_ASSERT(0 < sizeof(T), "Can't delete imcomplete type.");
		Insight::Memory::MemoryManager::Instance()->DeleteOnFreeList(ptr);
	}
};

template<typename T, typename TDeleter = DefaultDelete<T>>
class TUniquePtr
{
public:
	TUniquePtr() : m_ptr(Insight::Memory::MemoryManager::Instance()->NewOnFreeList<T>()) { }
	TUniquePtr(T* ptr) : m_ptr(ptr) { }
	template<typename... Args>
	TUniquePtr(Args&&... args) : m_ptr(Insight::Memory::MemoryManager::Instance()->NewOnFreeList<T>(std::forward<Args>(args)...)) { }

	// Copy ptr, delete.
	TUniquePtr(const TUniquePtr& ptr) = delete;
	TUniquePtr& operator=(const TUniquePtr& ptr) = delete;

	// Move ptr.
	TUniquePtr(TUniquePtr&& ptr)
	{
		m_ptr = ptr.m_ptr;
		ptr.m_ptr = nullptr;
	}
	void operator=(TUniquePtr&& ptr)
	{
		Reset();

		m_ptr = ptr.m_ptr;
		ptr.m_ptr = nullptr;
	}

	T* operator->()
	{
		return m_ptr;
	}

	T& operator*()
	{
		return *m_ptr;
	}

	void Reset()
	{
		if (m_ptr != nullptr)
		{
			m_deletor(m_ptr);
		}
	}

	~TUniquePtr() { Reset(); }

	void Release()
	{
		// This removes ownership of this pointer.
		m_ptr = nullptr;
	}

private:
	T* m_ptr;
	TDeleter m_deletor;
};


//template<typename T>
//class TSafePtr
//{
//public:
//	TSafePtr()
//		: m_ptr(NEW_ON_HEAP(T))
//	{
//		IS_CORE_STATIC_ASSERT(std::is_base_of_v<Insight::Object, T>::value, "'T' is not derived from 'Insight::Object'.");
//	}
//
//	TSafePtr(T* ptr)
//		: m_ptr(ptr)
//	{
//		IS_CORE_STATIC_ASSERT(Insight::Object, T, "'T' is not derived from 'Insight::Object'.");
//	}
//
//	TSafePtr(T& ptr)
//		: m_ptr(&ptr)
//	{
//		IS_CORE_STATIC_ASSERT(Insight::Object, T, "'T' is not derived from 'Insight::Object'.");
//	}
//
//	~TSafePtr()
//	{
//		Release();
//	}
//
//	void operator delete(void* ptr) = delete;
//
//	T* operator=(T* ptr)
//	{
//		m_ptr = ptr;
//		return m_ptr;
//	}
//
//	T* operator=(T ptr)
//	{
//		*m_ptr = ptr;
//		return m_ptr;
//	}
//
//	// Release this safe ptr container.
//	void Release()
//	{
//		if (m_ptr != nullptr)
//		{
//			DELETE_ON_HEAP(m_ptr);
//		}
//	}
//
//	T* Get()
//	{
//		return m_ptr;
//	}
//
//	T* operator ->() const { return m_ptr; }
//	//operator T*() const { return m_ptr; }
//
//private:
//	T* m_ptr;
//};