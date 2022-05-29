#pragma once

#include <new>
#include "MemoryTracker.h"
#include <atomic>
#include <type_traits>

#define NewTracked(Type)			static_cast<Type*>(Insight::Core::MemoryNewObject(new Type()).Ptr)
#define NewArgsTracked(Type, ...)	static_cast<Type*>(Insight::Core::MemoryNewObject(new Type(__VA_ARGS__)).Ptr)
#define TrackPtr(Ptr)				Insight::Core::MemoryTracker::Instance().Track(Ptr, Insight::Core::MemoryTrackAllocationType::Single)

#define DeleteTracked(Ptr)								\
Insight::Core::MemoryTracker::Instance().UnTrack(Ptr);	\
if (Ptr)												\
{														\
	delete Ptr;											\
	Ptr = nullptr;										\
};
#define UntrackPtr(Ptr)				Insight::Core::MemoryTracker::Instance().UnTrack(Ptr)

//#define IS_MEMORY_OVERRIDES
#ifdef IS_MEMORY_OVERRIDES
void* operator new(size_t size)
{
	if (size == 0)
	{
		return nullptr;
	}

	if (void* ptr = std::malloc(size))
	{
		Insight::Core::MemoryTracker::Instance().Track(ptr, Insight::Core::MemoryTrackAllocationType::Single);
		return ptr;
	}
	return nullptr;
}

void* operator new[](size_t size)
{
	if (size == 0)
	{
		return nullptr;
	}

	if (void* ptr = std::malloc(size))
	{
		Insight::Core::MemoryTracker::Instance().Track(ptr, Insight::Core::MemoryTrackAllocationType::Array);
		return ptr;
	}
	return nullptr;
}

void operator delete(void* ptr)
{
	Insight::Core::MemoryTracker::Instance().UnTrack(ptr);
	std::free(ptr);
}

void operator delete[](void* ptr)
{
	Insight::Core::MemoryTracker::Instance().UnTrack(ptr);
	std::free(ptr);
}
#endif

namespace Insight
{
	struct RefCount
	{
		int Inc() { m_strongRefs++; }
		int Dec() { m_strongRefs--; }
		int IncW() { m_weakRefs++; }
		int DecW() { m_weakRefs--; }

		bool HasRefs() const { return m_strongRefs > 0; }

	private:
		std::atomic<int> m_strongRefs = 0;
		std::atomic<int> m_weakRefs = 0;
	};
}

template<typename T>
struct UPtr
{
	UPtr() 
	{ }
	UPtr(T* ptr)
	{
		m_ptr = ptr;
	}
	UPtr(UPtr&& other)
	{
		Reset();
		m_ptr = other.m_ptr;
		other.Release();
	}
	~UPtr()
	{
		Reset();
	}

	UPtr& operator=(UPtr&& other)
	{
		Reset();
		m_ptr = other.m_ptr;
		other.Release();
		return *this;
	}

	// Can't copy UPtr.
	UPtr(const UPtr& other) = delete;
	UPtr& operator=(const UPtr& other) = delete;

	UPtr& operator=(std::nullptr_t) noexcept
	{
		Reset();
		return *this;
	}

	void Reset() 
	{
		if (m_ptr)
		{
			DeleteTracked(m_ptr);
		}
	}
	void Release() { m_ptr = nullptr; }
	bool IsValid() const { return m_ptr != nullptr; }
	T* Get() const { return m_ptr; }

	bool operator==(const UPtr& other) const { return m_ptr == other.m_ptr; }
	bool operator==(const UPtr* other) const { return m_ptr == other; }
	operator bool() const { return m_ptr; }

	T* operator->() const { return m_ptr; }
	T* operator*() const { return m_ptr; }

private:
	T* m_ptr = nullptr;
};

template<typename T>
struct RPtr : RefCount
{
	RPtr() { }
	RPtr(T* ptr) { }
	RPtr(const RPtr& other) 
	{
		m_ptr = other.m_ptr;
		m_refCount = other.m_refCount;
		m_refCount->Inc();
	}
	RPtr(RPtr&& other)
	{
		m_ptr = other.m_ptr;
		m_refCount = other.m_refCount;
	}
	~RPtr()
	{
		Reset();
	}

	RPtr& operator=(const RPtr& other)
	{
		m_ptr = other.m_ptr;
		m_refCount = other.m_refCount;
		m_refCount->Inc();
		return *this;
	}
	RPtr& operator=(RPtr&& other)
	{
		m_ptr = other.m_ptr;
		m_refCount = other.m_refCount;
		return *this;
	}

	void Reset()
	{
		if (!m_refCount->HasRefs())
		{
			DeleteTracked(m_ptr);
			DeleteTracked(m_refCount);
		}
	}
	void Release() { m_ptr = nullptr; m_refCount->Dec(); m_refCount = nullptr; }
	bool IsValid() const { return m_ptr; }

	bool operator==(const RPtr& other) const { return m_ptr == other.m_ptr; }
	bool operator==(const RPtr* other) const { return m_ptr == other; }

	T* operator->() const { return m_ptr; }
	T* operator*() const { return m_ptr; }

private:
	T* m_ptr;
	Insight::RefCount* m_refCount = nullptr;
};

namespace Insight::Core
{
	struct IS_CORE MemoryNewObject
	{
		MemoryNewObject(void* ptr);
		void* Ptr;
	};

	template<typename T, typename... Args>
	UPtr<T> MakeUPtr(Args&&... args)
	{
		if constexpr (sizeof...(Args) > 0)
		{
			return UPtr(NewArgsTracked(T, std::forward<Args...>(args)...));
		}
		else
		{
			return UPtr(NewTracked(T));
		}
	}
}