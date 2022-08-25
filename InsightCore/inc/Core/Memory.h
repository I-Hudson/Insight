#pragma once

#include "MemoryTracker.h"
#include "Platform/Platform.h"

#include <new>
#include <atomic>
#include <type_traits>
#include <utility>

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

template<typename T>
class UPtr;
template<typename T>
class RPtr;
template<typename T>
class WPtr;

namespace Insight
{
	class RefCount
	{
	public:
		int Inc() { return m_strongRefs++; }
		int Dec() { return m_strongRefs--; }
		int IncW() { return m_weakRefs++; }
		int DecW() { return m_weakRefs--; }

		bool HasRefs() const { return m_strongRefs > 0; }

	private:
		std::atomic<int> m_strongRefs = 0;
		std::atomic<int> m_weakRefs = 0;
	};

	//template<typename T>
	//class PtrBase
	//{
	//private:
	//	using TPtr = T*;
	//	using RefCountPtr = RefCount*;

	//protected:
	//	void ConstructFromPtr(T* ptr)
	//	{
	//		m_ptr = ptr;
	//		m_refCount = NewTracked(RefCount);
	//		Inc();
	//	}

	//	template<typename T2>
	//	void ConstructFromCopy(const PtrBase<T2>& other)
	//	{
	//		m_ptr = other.m_ptr;
	//		m_refCount = other.m_refCount;
	//		Inc();
	//	}

	//	template <class T2>
	//	void MoveFromOther(PtrBase<T2>&& other)
	//	{
	//		//std::swap(m_ptr, other.m_ptr);
	//		std::swap(m_refCount, other.m_refCount);
	//	}

	//	template <class T2>
	//	void ConstructWeakFromOther(const PtrBase<T2>& other)
	//	{
	//		m_ptr = other.m_ptr;
	//		m_refCount = other.m_refCount;
	//		IncW();
	//	}
	//	template<typename T2>
	//	void ConstructFromWeak(const PtrBase<T2>& other)
	//	{
	//		m_ptr = other.m_ptr;
	//		m_refCount = other.m_refCount;
	//		Inc();
	//	}

	//	void Inc()
	//	{
	//		if (m_refCount)
	//		{
	//			m_refCount->Inc();
	//		}
	//	}
	//	void Dec()
	//	{
	//		if (m_refCount)
	//		{
	//			m_refCount->Dec();
	//		}

	//		if (m_refCount->HasRefs())
	//		{
	//			DeleteTracked(m_ptr);
	//			DeleteTracked(m_refCount);
	//		}
	//	}

	//	void IncW()
	//	{
	//		if (m_refCount)
	//		{
	//			m_refCount->IncW();
	//		}
	//	}
	//	void DecW()
	//	{
	//		if (m_refCount)
	//		{
	//			m_refCount->DecW();
	//		}
	//	}

	//	void Swap(PtrBase& other)
	//	{
	//		std::swap(m_ptr, other.m_ptr);
	//		std::swap(m_refCount, other.m_refCount);
	//	}

	//	TPtr GetPtr() const { return m_ptr; }

	//protected:
	//	TPtr m_ptr = nullptr;
	//	RefCountPtr m_refCount = nullptr;

	//	template <class T0>
	//	friend class _Ptr_base;

	//	friend class RPtr<T>;
	//	friend class WPtr<T>;
	//};
}

template<typename T>
class UPtr
{
public:
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

	template<typename T2, std::enable_if_t<std::_SP_pointer_compatible<T2, T>::value, int> = 0>
	UPtr(T2* ptr)
	{
		m_ptr = ptr;
	}

	template<typename T2, std::enable_if_t<std::_SP_pointer_compatible<T2, T>::value, int> = 0>
	UPtr(UPtr<T2>&& other)
	{
		m_ptr = other.m_ptr;
		other.m_ptr = nullptr;
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
	template<typename T2, std::enable_if_t<std::_SP_pointer_compatible<T2, T>::value, int> = 0>
	UPtr(const UPtr<T2>& other) = delete;
	UPtr& operator=(const UPtr& other) = delete;
	template<typename T2>
	UPtr& operator=(const UPtr<T2>& other) = delete;

	UPtr& operator=(std::nullptr_t) noexcept
	{
		Reset();
		return *this;
	}
	template<typename T2>
	UPtr& operator=(UPtr<T2>&& other)
	{
		Reset();
		m_ptr = other.m_ptr;
		other.m_ptr = nullptr;
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

	template<typename>
	friend class UPtr;
};

template<typename T>
class RPtr
{
private:
	using TPtr = T*;
	using TRef = T&;

public:
	RPtr() { }

	RPtr(TPtr* ptr)
	{
		m_ptr = ptr;
		m_refCount = NewTracked(RefCount);
		Inc();
	}
	RPtr(const RPtr& other)
	{
		m_ptr = other.m_ptr;
		m_refCount = other.m_refCount;
		Inc();
	}
	RPtr(RPtr&& other)
	{
		m_ptr = other.m_ptr;
		m_refCount = other.m_refCount;
		other.m_ptr = nullptr;
		other.m_refCount = nullptr;
	}

	template<typename T2, std::enable_if_t<std::_SP_pointer_compatible<T2, T>::value, int> = 0>
	RPtr(T2* ptr)
	{
		m_ptr = ptr;
		m_refCount = NewTracked(Insight::RefCount);
		Inc();
	}
	template<typename T2, std::enable_if_t<std::_SP_pointer_compatible<T2, T>::value, int> = 0>
	RPtr(const RPtr<T2>& other)
	{
		m_ptr = other.m_ptr;
		m_refCount = other.m_refCount;
		Inc();
	}
	template<typename T2, std::enable_if_t<std::_SP_pointer_compatible<T2, T>::value, int> = 0>
	RPtr(RPtr<T2>&& other)
	{
		m_ptr = other.m_ptr;
		m_refCount = other.m_refCount;
		other.m_ptr = nullptr;
		other.m_refCount = nullptr;
	}
	~RPtr()
	{
		Dec();
	}

	RPtr& operator=(const RPtr& other)
	{
		RPtr(other).Swap(*this);
		return *this;
	}
	RPtr& operator=(RPtr&& other)
	{
		RPtr(std::move(other)).Swap(*this);
		return *this;
	}

	template<typename T2>
	RPtr& operator=(const RPtr<T2>& other)
	{
		RPtr(other).Swap(*this);
		return *this;
	}
	template<typename T2>
	RPtr& operator=(RPtr<T2>&& other)
	{
		RPtr(std::move(other)).Swap(*this);
		return *this;
	}

	void Swap(RPtr& other)
	{
		std::swap(m_ptr, other.m_ptr);
		std::swap(m_refCount, other.m_refCount);
	}
	template<typename T2, std::enable_if_t<std::_SP_pointer_compatible<T2, T>::value, int> = 0>
	void Swap(RPtr<T2>& other)
	{
		std::swap(m_ptr, other.m_ptr);
		std::swap(m_refCount, other.m_refCount);
	}

	void Inc()
	{
		if (m_refCount)
		{
			m_refCount->Inc();
		}
	}
	void Dec()
	{
		if (m_refCount)
		{
			m_refCount->Dec();
		}

		if (m_refCount && !m_refCount->HasRefs())
		{
			DeleteTracked(m_ptr);
			DeleteTracked(m_refCount);
		}
	}

	bool operator==(const RPtr& other) const { return Get() == other.Get(); }
	bool operator==(const RPtr* other) const { return Get() == other->Get(); }
	template<typename T2>
	bool operator==(const RPtr<T2>& other) const { return Get() == other.Get(); }
	template<typename T2>
	bool operator==(const RPtr<T2>* other) const { return Get() == other->Get(); }

	T* operator->() const { return Get(); }
	T* operator*() const { return Get(); }

	T** operator&() { return &Get(); }

	void ConstructThisFromWeak(WPtr<T> const& wPtr)
	{
		m_ptr = wPtr.m_ptr;
		m_refCount = wPtr.m_refCount;
		Inc();
	}

	void Reset()
	{
		RPtr().Swap(*this);
	}
	bool IsValid() const { return Get() != nullptr; }
	TPtr Get() const { return m_ptr; }

private:
	TPtr m_ptr = nullptr;
	Insight::RefCount* m_refCount = nullptr;

	template<typename>
	friend class RPtr;
	template<typename>
	friend class WPtr;
};

template<typename T>
class WPtr
{
private:
	using TPtr = T*;
	using TRef = T&;
	using TRPtr = RPtr<T>;

public:
	WPtr() { }

	WPtr(const WPtr& other)
	{
		m_ptr = other.m_ptr;
		m_refCount = other.m_refCount;
		IncW();
	}
	WPtr(WPtr&& other)
	{
		m_ptr = other.m_ptr;
		m_refCount = other.m_refCount;
		other.m_ptr = nullptr;
		other.m_refCount = nullptr;
	}

	template<typename T2, std::enable_if_t<std::_SP_pointer_compatible<T2, T>::value, int> = 0>
	WPtr(const WPtr<T2>& other)
	{
		m_ptr = other.m_ptr;
		m_refCount = other.m_refCount;
		IncW();
	}
	template<typename T2, std::enable_if_t<std::_SP_pointer_compatible<T2, T>::value, int> = 0>
	WPtr(WPtr<T2>&& other)
	{
		m_ptr = other.m_ptr;
		m_refCount = other.m_refCount;
		other.m_ptr = nullptr;
		other.m_refCount = nullptr;
	}
	template<typename T2, std::enable_if_t<std::_SP_pointer_compatible<T2, T>::value, int> = 0>
	WPtr(const RPtr<T2>& other)
	{
		m_ptr = other.m_ptr;
		m_refCount = other.m_refCount;
		IncW();
	}
	~WPtr()
	{
		DecW();
	}

	WPtr& operator=(const WPtr& other)
	{
		WPtr(other).Swap(*this);
		return *this;
	}
	WPtr& operator=(WPtr&& other)
	{
		WPtr(std::move(other)).Swap(*this);
		return *this;
	}

	template<typename T2, std::enable_if_t<std::_SP_pointer_compatible<T2, T>::value, int> = 0>
	WPtr& operator=(const WPtr<T2>& other)
	{
		WPtr(other).Swap(*this);
		return *this;
	}
	template<typename T2, std::enable_if_t<std::_SP_pointer_compatible<T2, T>::value, int> = 0>
	WPtr& operator=(WPtr<T2>&& other)
	{
		WPtr(std::move(other)).Swap(*this);
		return *this;
	}
	template<typename T2, std::enable_if_t<std::_SP_pointer_compatible<T2, T>::value, int> = 0>
	WPtr& operator=(const RPtr<T2>& other)
	{
		ConstructWeakFromOther(other);
		return *this;
	}

	bool operator==(const TPtr other) const { return Get() == other; }
	bool operator==(const WPtr& other) const { return Get() == other.Get(); }
	bool operator==(const TRPtr& other) const { return Get() == other.Get(); }

	template<typename T2>
	bool operator==(const T2* other) const { return Get() == other; }
	template<typename T2>
	bool operator==(const WPtr<T2>& other) const { return Get() == other.Get(); }
	template<typename T2>
	bool operator==(const RPtr<T2>& other) const { return Get() == other.Get(); }

	void IncW()
	{
		if (m_refCount)
		{
			m_refCount->IncW();
		}
	}
	void DecW()
	{
		if (m_refCount)
		{
			m_refCount->DecW();
		}
	}

	void Swap(WPtr& other)
	{
		std::swap(m_ptr, other.m_ptr);
		std::swap(m_refCount, other.m_refCount);
	}
	template<typename T2, std::enable_if_t<std::_SP_pointer_compatible<T2, T>::value, int> = 0>
	void Swap(WPtr<T2>& other)
	{
		std::swap(m_ptr, other.m_ptr);
		std::swap(m_refCount, other.m_refCount);
	}

	// Take ownership of 'm_ptr' via a reference pointer.
	RPtr<T> Lock()
	{
		RPtr<T> rPtr;
		rPtr.ConstructThisFromWeak(*this);
		return rPtr;
	}

	bool IsValid() const { return Get() != nullptr; }
	void Reset() 
	{ 
		WPtr().Swap(*this);
	}
	TPtr Get() const { return m_ptr; }

private:
	TPtr m_ptr = nullptr;
	Insight::RefCount* m_refCount = nullptr;

	template<typename>
	friend class WPtr;
	template<typename>
	friend class RPtr;
};

namespace Insight::Core
{
	struct IS_CORE MemoryNewObject
	{
		MemoryNewObject(void* ptr);
		void* Ptr;
	};
}

template<typename T, typename... Args>
UPtr<T> MakeUPtr(Args&&... args)
{
	if constexpr (sizeof...(Args) > 0)
	{
		return UPtr<T>(NewArgsTracked(T, std::forward<Args>(args)...));
	}
	else
	{
		return UPtr<T>(NewTracked(T));
	}
}

template<typename T, typename... Args>
RPtr<T> MakeRPtr(Args&&... args)
{
	if constexpr (sizeof...(Args) > 0)
	{
		return RPtr<T>(NewArgsTracked(T, std::forward<Args>(args)...));
	}
	else
	{
		return RPtr<T>(NewTracked(T));
	}
}