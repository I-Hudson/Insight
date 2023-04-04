#pragma once

#include "MemoryTracker.h"
#include "Core/Delegate.h"

#include <new>
#include <atomic>
#include <type_traits>
#include <utility>

/// Helper macro for making a new pointer with tracking.
#define NewTracked(Type)			::New<Type>()
/// Helper macro for making a new pointer with args and tracking.
#define NewArgsTracked(Type, ...)	::New<Type>(__VA_ARGS__)
/// /// Helper macro for tracking a exists pointer.
#define TrackPtr(Ptr)				::Insight::Core::MemoryTracker::Instance().Track(Ptr, sizeof(*Ptr), Insight::Core::MemoryTrackAllocationType::Single)

#define DeleteTracked(Ptr)			Delete(Ptr);

#define UntrackPtr(Ptr)				::Insight::Core::MemoryTracker::Instance().UnTrack(Ptr)

NO_DISCARD FORCE_INLINE void* New(u64 bytes, Insight::Core::MemoryAllocCategory memoryAllocCategory)
{
	void* ptr = std::malloc(bytes);
	Insight::Core::MemoryTracker::Instance().Track(ptr, bytes, memoryAllocCategory, Insight::Core::MemoryTrackAllocationType::Single);
	return ptr;
}
NO_DISCARD FORCE_INLINE void* New(u64 bytes)
{
	return New(bytes, Insight::Core::MemoryAllocCategory::General);
}

template<typename T, typename... Params>
NO_DISCARD FORCE_INLINE T* New(Params&&... params)
{
	T* ptr = new T(std::forward<Params>(params)...);
	Insight::Core::MemoryTracker::Instance().Track(ptr, sizeof(T), Insight::Core::MemoryTrackAllocationType::Single);
	return ptr;
}
template<typename T, Insight::Core::MemoryAllocCategory MemoryAllocCategory,typename... Params>
NO_DISCARD FORCE_INLINE T* New(Params&&... params)
{
	T* ptr = new T(std::forward<Params>(params)...);
	Insight::Core::MemoryTracker::Instance().Track(ptr, sizeof(T), MemoryAllocCategory, Insight::Core::MemoryTrackAllocationType::Single);
	return ptr;
}

template<typename... Params>
NO_DISCARD FORCE_INLINE void* NewNoTrack(u64 size)
{
	void* rawPtr = malloc(size);//new T(std::forward<Params>(params)...);
	return rawPtr;
}

FORCE_INLINE void DeleteNoTrack(void* ptr)
{
	free(ptr);
}

template<typename T>
FORCE_INLINE void Delete(T*& pointer)
{
	if (pointer != nullptr)
	{
		Insight::Core::MemoryTracker::Instance().UnTrack(pointer);
		delete pointer;
	}
	pointer = nullptr;
}

template<typename T>
class UPtr;
template<typename T>
class RPtr;
template<typename T>
class WPtr;
template<typename T>
class Ptr;

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
}

//// <summary>
//// Custom unique pointer implementaion.
//// </summary>
//// <typeparam name="T"></typeparam>
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

	/// Can't copy UPtr.
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
	bool operator==(const T* other) const { return m_ptr == other; }


	operator bool() const { return m_ptr; }

	T* operator->() const { return m_ptr; }
	T* operator*() const { return m_ptr; }

private:
	T* m_ptr = nullptr;

	template<typename>
	friend class UPtr;
};

//// <summary>
//// Custom shared pointer implmentation.
//// </summary>
//// <typeparam name="T"></typeparam>
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
		m_refCount = New<Insight::RefCount, Insight::Core::MemoryAllocCategory::General>();
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
		m_refCount = New<Insight::RefCount, Insight::Core::MemoryAllocCategory::General>();
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
	operator bool() { return IsValid(); }

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

//// <summary>
//// Custom weak pointer implmentaiton.
//// </summary>
//// <typeparam name="T"></typeparam>
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
	operator bool() { return IsValid(); }

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

	/// Take ownership of 'm_ptr' via a reference pointer.
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

//// <summary>
//// Wrapper around a raw pointer. Should be used like a "view" to the pointer.
//// Shouldn't allow deletion of the pointer. 
//// (Similar to a weak pointer. But I don't want this to be able to have any form of ownership
//// over the pointer)
//// </summary>
//// <typeparam name="T"></typeparam>
template<typename T>
class Ptr
{
public:
	CONSTEXPR Ptr() = default;
	CONSTEXPR Ptr(T* pointer)									{ m_ptr = pointer; }
	CONSTEXPR Ptr(Ptr const& other)		: m_ptr(other.m_ptr)	{ }
	CONSTEXPR Ptr(const UPtr<T>& other)	: m_ptr(other.Get())	{ }
	CONSTEXPR Ptr(Ptr&& other)							        { m_ptr = other.m_ptr; other.m_ptr = nullptr; }
	~Ptr()														{ m_ptr = nullptr; }

	/// Assign
	CONSTEXPR Ptr& operator=(Ptr const& other)			        { m_ptr = other.m_ptr; return *this; }
	CONSTEXPR Ptr& operator=(T* other)					        { m_ptr = other; return *this; }

	/// Compare
	CONSTEXPR bool operator==(Ptr const& other) const			    { m_ptr = other.m_ptr; }
	CONSTEXPR bool operator==(T* other) const					    { m_ptr = other; }
	
	CONSTEXPR operator bool() const								{ return Get() != nullptr; }
	CONSTEXPR T* operator->() const                               { return m_ptr; }

	/// [INTERNAL] Used for compatibility with low level code. Should be used to a minimum
	/// in higher level systems.
	CONSTEXPR T* Get() const										{ return m_ptr; }

	template<typename T2>
	CONSTEXPR Ptr<T2> CastTo() { return static_cast<T2*>(m_ptr); }

private:
	T* m_ptr = nullptr;
};

//// <summary>
//// Return a UPtr.
//// </summary>
//// <typeparam name="T"></typeparam>
//// <typeparam name="...Args"></typeparam>
//// <param name="...args"></param>
//// <returns></returns>
template<typename T, typename... Args>
UPtr<T> MakeUPtr(Args&&... args)
{
	if constexpr (sizeof...(Args) > 0)
	{
		return UPtr<T>(New<T, Insight::Core::MemoryAllocCategory::General>(std::forward<Args>(args)...));
	}
	else
	{
		return UPtr<T>(New<T, Insight::Core::MemoryAllocCategory::General>());
	}
}

//// <summary>
//// Return a RPtr.
//// </summary>
//// <typeparam name="T"></typeparam>
//// <typeparam name="...Args"></typeparam>
//// <param name="...args"></param>
//// <returns></returns>
template<typename T, typename... Args>
RPtr<T> MakeRPtr(Args&&... args)
{
	if constexpr (sizeof...(Args) > 0)
	{
		return RPtr<T>(New<T, Insight::Core::MemoryAllocCategory::General>(std::forward<Args>(args)...));
	}
	else
	{
		return RPtr<T>(New<T, Insight::Core::MemoryAllocCategory::General>());
	}
}



template<typename T>
class TObjectOwnPtr;
template<typename T>
using TObjectOPtr = TObjectOwnPtr<T>;

template<typename T>
class TObjectPtr;

class ReferenceCountObject
{
public:
	void Incr() { ++m_strongRef; }
	void Decr() { --m_strongRef; }

	void IncrW() { ++m_weakRef; }
	void DecrW() { --m_weakRef; }

	bool IsValid() const { return m_strongRef.load() > 0; }
	int GetCount() const { return m_strongRef.load() + m_weakRef.load(); }
	void Reset() { m_strongRef = 0; m_weakRef = 0; }

private:
	std::atomic<int> m_strongRef = 0;
	std::atomic<int> m_weakRef = 0;
};

/// <summary>
/// Pointer warpper class to store a pointer which is derived from 'Object'.
/// This is an owning pointer and when destroyed the pointer is deleted.
/// </summary>
/// <typeparam name="T"></typeparam>
template<typename T>
class TObjectOwnPtr
{
	using Ptr = T*;
	using Ref = T&;

public:
	TObjectOwnPtr() { }
	TObjectOwnPtr(Ptr pointer)
	{
		Reset();
		m_ptr = pointer;
#ifdef TOBJECTPTR_REF_COUNTING
		m_refCount = New<ReferenceCountObject, Insight::Core::MemoryAllocCategory::General>();
#endif // TOBJECTPTR_REF_COUNTING
		Incr();
	}
	TObjectOwnPtr(const TObjectOwnPtr& other) = delete;
	template<typename TOther>
	TObjectOwnPtr(const TObjectOwnPtr<TOther>& other) = delete;
	TObjectOwnPtr(TObjectOwnPtr&& other)
	{
		Reset();
		m_ptr = other.m_ptr;
		other.m_ptr = nullptr;
#ifdef TOBJECTPTR_REF_COUNTING
		m_refCount = other.m_refCount;
		other.m_refCount = nullptr;
#endif // TOBJECTPTR_REF_COUNTING
	}
	template<typename TOther>
	TObjectOwnPtr(TObjectOwnPtr<TOther>&& other)
	{
		static_assert(std::is_convertible_v<T, TOther>, "[TObjectOwnPtr] Unable to convert to TOther.");
		Reset();
		m_ptr = static_cast<Ptr>(other.m_ptr);
		other.m_ptr = nullptr;
#ifdef TOBJECTPTR_REF_COUNTING
		m_refCount = other.m_refCount;
		other.m_refCount = nullptr;
#endif // TOBJECTPTR_REF_COUNTING
	}
	virtual ~TObjectOwnPtr()
	{
		Reset();
	}

	operator bool() const { return m_ptr != nullptr; }
	operator Ptr() const { return m_ptr; }
	Ptr operator*() const { return m_ptr; }
	Ptr operator->() const { return m_ptr; }

	TObjectOwnPtr& operator=(const TObjectOwnPtr& pointer) = delete;
	template<typename TOther>
	TObjectOwnPtr& operator=(const TObjectOwnPtr<TOther>& pointer) = delete;
	TObjectOwnPtr& operator=(TObjectOwnPtr&& other)
	{ 
		Reset(); 
		m_ptr = other.m_ptr; 
		other.m_ptr = nullptr;
#ifdef TOBJECTPTR_REF_COUNTING
		m_refCount = other.m_refCount;
		other.m_refCount = nullptr;
#endif // TOBJECTPTR_REF_COUNTING
		return *this; 
	}
	template<typename TOther>
	TObjectOwnPtr& operator=(TObjectOwnPtr&& other)
	{ 
		static_assert(std::is_convertible_v<T, TOther>, "[TObjectOwnPtr] Unable to convert to TOther."); 
		Reset(); 
		m_ptr = static_cast<Ptr>(other.m_ptr); 
		other.m_ptr = nullptr;
#ifdef TOBJECTPTR_REF_COUNTING
		m_refCount = other.m_refCount;
		other.m_refCount = nullptr;
#endif // TOBJECTPTR_REF_COUNTING
		return *this; 
	}

	bool operator==(const TObjectPtr<T>& other) const
	{ return m_ptr == other.m_ptr; }
	template<typename TOther>
	bool operator==(const TObjectPtr<TOther>& other) const
	{ return m_ptr == other.m_ptr; }
	bool operator==(const TObjectOwnPtr& other) const
	{ return m_ptr == other.m_ptr; }
	template<typename TOther>
	bool operator==(const TObjectOwnPtr<TOther>& other) const
	{ return m_ptr == other.m_ptr; }

	bool operator!=(const TObjectPtr<T>& other) const
	{
		return !(*this == other);
	}
	template<typename TOther>
	bool operator!=(const TObjectPtr<TOther>& other) const
	{
		return !(*this == other);
	}
	bool operator!=(const TObjectOwnPtr& other) const
	{
		return !(*this == other);
	}
	template<typename TOther>
	bool operator!=(const TObjectOwnPtr<TOther>& other) const
	{
		return !(*this == other);
	}

	void Reset()
	{
		if (m_ptr)
		{
			Delete(m_ptr);
		}
#ifdef TOBJECTPTR_REF_COUNTING
		Decr();
		if (m_refCount
			&& m_refCount->GetCount() == 0)
		{
			Delete(m_refCount);
		}
		m_refCount = nullptr;
#endif // TOBJECTPTR_REF_COUNTING
	}

	Ptr Get() const { return m_ptr; }

#ifdef TEST_ENABLED
	ReferenceCountObject*		GetReferenceCountObject()	const { return m_refCount; }
	Ptr							GetPtr()					const { return m_ptr; }
#endif

private:
	void Incr()
	{
#ifdef TOBJECTPTR_REF_COUNTING
		if (m_refCount)
		{
			m_refCount->Incr();
		}
#endif // TOBJECTPTR_REF_COUNTING
	}
	void Decr()
	{
#ifdef TOBJECTPTR_REF_COUNTING
		if (m_refCount)
		{
			m_refCount->Decr();
		}
#endif // TOBJECTPTR_REF_COUNTING
	}


private:
	Ptr m_ptr = nullptr;
#ifdef TOBJECTPTR_REF_COUNTING
	ReferenceCountObject* m_refCount = nullptr;
#endif // TOBJECTPTR_REF_COUNTING
	template<typename>
	friend class TObjectOwnPtr;
	template<typename>
	friend class TObjectPtr;
};

/// <summary>
/// Pointer warpper class to store a pointer which is derived from 'Object'.
/// This is a non owning pointer.
/// </summary>
/// <typeparam name="T"></typeparam>
template<typename T>
class TObjectPtr
{
	static_assert(!std::is_pointer_v<T>, "[TObjectPtr] Template 'T' must not be a pointer type for the template.");
	static_assert(!std::is_reference_v<T>, "[TObjectPtr] Template 'T' must not be a reference type for the template.");

	using Ptr = T*;
	using Ref = T&;

public:
	TObjectPtr() { }
	TObjectPtr(std::nullptr_t) { }

	TObjectPtr(const TObjectPtr& other)
	{
		Reset();
		m_ptr = other.m_ptr;
#ifdef TOBJECTPTR_REF_COUNTING
		m_refCount = other.m_refCount;
#endif // TOBJECTPTR_REF_COUNTING
		Incr();
	}
	template<typename TOther>
	TObjectPtr(const TObjectPtr<TOther>& other)
	{
		static_assert(std::is_convertible_v<TOther*, Ptr> || std::is_convertible_v<Ptr, TOther*>, "[TObjectPtr::ConstructorCopy] Must be able to convert from 'TOther' to 'T'.");
		Reset();
		m_ptr = static_cast<Ptr>(other.m_ptr);
#ifdef TOBJECTPTR_REF_COUNTING
		m_refCount = other.m_refCount;
#endif // TOBJECTPTR_REF_COUNTING
		Incr();
	}

	TObjectPtr(TObjectPtr&& other)
	{
		Reset();
		m_ptr = static_cast<Ptr>(other.m_ptr);
		other.m_ptr = nullptr;
#ifdef TOBJECTPTR_REF_COUNTING
		m_refCount = other.m_refCount;
		other.m_refCount = nullptr;
#endif // TOBJECTPTR_REF_COUNTING
	}
	template<typename TOther>
	TObjectPtr(TObjectPtr<TOther>&& other)
	{
		static_assert(std::is_convertible_v<TOther*, Ptr> || std::is_convertible_v<Ptr, TOther*>, "[TObjectPtr::ConstructorMove] Must be able to convert from 'TOther' to 'T'.");
		Reset();
		m_ptr = static_cast<Ptr>(other.m_ptr);
		other.m_ptr = nullptr;
#ifdef TOBJECTPTR_REF_COUNTING
		m_refCount = other.m_refCount;
		other.m_refCount = nullptr;
#endif // TOBJECTPTR_REF_COUNTING
	}

	TObjectPtr(const TObjectOwnPtr<T>& other)
	{
		Reset();
		m_ptr = other.m_ptr;
#ifdef TOBJECTPTR_REF_COUNTING
		m_refCount = other.m_refCount;
#endif // TOBJECTPTR_REF_COUNTING
		Incr();
	}
	template<typename TOther>
	TObjectPtr(const TObjectOwnPtr<TOther>& other)
	{
		static_assert(std::is_convertible_v<TOther*, Ptr> || std::is_convertible_v<Ptr, TOther*>, "[TObjectPtr::ConstructorCopy] Must be able to convert from 'TOther' to 'T'.");
		Reset();
		m_ptr = static_cast<Ptr>(other.m_ptr);
#ifdef TOBJECTPTR_REF_COUNTING
		m_refCount = other.m_refCount;
#endif // TOBJECTPTR_REF_COUNTING
		Incr();
	}

	TObjectPtr(TObjectOwnPtr<T>&& other) = delete;
	template<typename TOther>
	TObjectPtr(TObjectOwnPtr<TOther>&& other) = delete;

	~TObjectPtr()
	{
		Reset();
	}

	operator bool() const 
	{ 
		if (m_refCount
			&& !m_refCount->IsValid())
		{
			m_ptr = nullptr;
		}
		return m_ptr != nullptr; 
	}
	operator Ptr() const 
	{ 
		if (m_refCount
			&& !m_refCount->IsValid())
		{
			m_ptr = nullptr;
		}
		return m_ptr; 
	}
	Ptr operator->() const { return m_ptr; }

	TObjectPtr& operator=(std::nullptr_t)
	{
		Reset();
	}

	/// @brief Assign copy operator for TObjectPtr.
	/// @param other 
	/// @return TObjectPtr&
	TObjectPtr& operator=(const TObjectPtr& other)
	{
		Reset();
		m_ptr = other.m_ptr;
#ifdef TOBJECTPTR_REF_COUNTING
		m_refCount = other.m_refCount;
#endif // TOBJECTPTR_REF_COUNTING
		Incr();
		return *this;
	}
	/// @brief Assign copy operator for TObjectPtr template.
	/// @tparam TOther 
	/// @param other 
	/// @return TObjectPtr&
	template<typename TOther>
	TObjectPtr& operator=(const TObjectPtr<TOther>& other)
	{
		static_assert(std::is_convertible_v<TOther*, Ptr> || std::is_convertible_v<Ptr, TOther*>, "[TObjectPtr::operator=(Copy)] Must be able to convert from 'TOther' to 'T'.");
		Reset();
		m_ptr = static_cast<Ptr>(other.m_ptr);
#ifdef TOBJECTPTR_REF_COUNTING
		m_refCount = other.m_refCount;
#endif // TOBJECTPTR_REF_COUNTING
		Incr();
		return *this;
	}

	/// @brief Assign move operator for TObjectPtr.
	/// @param other 
	/// @return TObjectPtr&
	TObjectPtr& operator=(TObjectPtr&& other)
	{
		Reset();
		m_ptr = other.m_ptr;
		other.m_ptr = nullptr;
#ifdef TOBJECTPTR_REF_COUNTING
		m_refCount = other.m_refCount;
		other.m_refCount = nullptr;
#endif // TOBJECTPTR_REF_COUNTING
		return *this;
	}
	/// @brief Assign move operator for TObjectPtr template.
	/// @tparam TOther 
	/// @param other 
	/// @return TObjectPtr&
	template<typename TOther>
	TObjectPtr& operator=(TObjectPtr&& other)
	{
		static_assert(std::is_convertible_v<TOther*, Ptr> || std::is_convertible_v<Ptr, TOther*>, "[TObjectPtr::operator=(Move)] Unable to convert to TOther.");
		Reset();
		m_ptr = static_cast<Ptr>(other.m_ptr);
		other.m_ptr = nullptr;
#ifdef TOBJECTPTR_REF_COUNTING
		m_refCount = other.m_refCount;
		other.m_refCount = nullptr;
#endif // TOBJECTPTR_REF_COUNTING
		return *this;
	}

	/// @brief Assign copy operator for TObjectOwnPtr.
	/// @param other 
	/// @return TObjectPtr&
	TObjectPtr& operator=(const TObjectOwnPtr<T>& other)
	{
		Reset();
		m_ptr = other.m_ptr;
#ifdef TOBJECTPTR_REF_COUNTING
		m_refCount = other.m_refCount;
#endif // TOBJECTPTR_REF_COUNTING
		Incr();
		return *this;
	}
	/// @brief Assign copy operator for TObjectOwnPtr template.
	/// @tparam TOther 
	/// @param other 
	/// @return TObjectPtr&
	template<typename TOther>
	TObjectPtr& operator=(const TObjectOwnPtr<TOther>& other)
	{
		static_assert(std::is_convertible_v<TOther*, Ptr> || std::is_convertible_v<Ptr, TOther*>, "[TObjectPtr::operator=(Copy)] Must be able to convert from 'TOther' to 'T'.");
		Reset();
		m_ptr = static_cast<Ptr>(other.m_ptr);
#ifdef TOBJECTPTR_REF_COUNTING
		m_refCount = other.m_refCount;
#endif // TOBJECTPTR_REF_COUNTING
		Incr();
		return *this;
	}	 

	TObjectPtr& operator=(TObjectOwnPtr<T>&& other) = delete;
	template<typename TOther>
	TObjectPtr& operator=(TObjectOwnPtr<TOther>&& other) = delete;

	bool operator==(std::nullptr_t) const { return m_ptr == nullptr; }
	bool operator!=(std::nullptr_t) const { return m_ptr != nullptr; }

	bool operator==(const TObjectPtr& other) const
	{
		return m_ptr == other.m_ptr;
	}
	template<typename TOther>
	bool operator==(const TObjectPtr<TOther>& other) const
	{
		return m_ptr == other.m_ptr;
	}

	bool operator==(const TObjectOwnPtr<T>& other) const
	{
		return m_ptr == other.m_ptr;
	}
	template<typename TOther>
	bool operator==(const TObjectOwnPtr<TOther>& other) const
	{
		return m_ptr == other.m_ptr;
	}

	bool operator!=(const TObjectPtr& other) const
	{
		return !(*this == other);
	}
	template<typename TOther>
	bool operator!=(const TObjectPtr<TOther>& other) const
	{
		return !(*this == other);
	}

	bool operator!=(const TObjectOwnPtr<T>& other) const
	{
		return !(*this == other);
	}
	template<typename TOther>
	bool operator!=(const TObjectOwnPtr<TOther>& other) const
	{
		return !(*this == other);
	}

	void Reset()
	{
		Decr();
		m_ptr = nullptr;
#ifdef TOBJECTPTR_REF_COUNTING
		if (m_refCount 
			&& m_refCount->GetCount() == 0)
		{
			Delete(m_refCount);
		}
		m_refCount = nullptr;
#endif
	}

	Ptr Get() const { return m_ptr; }

	template<typename TCastTo>
	TObjectPtr<TCastTo> CastTo()
	{
		TObjectPtr<TCastTo> dObjectPtr(*this);
		return dObjectPtr;
	}

#ifdef TEST_ENABLED
	ReferenceCountObject*		GetReferenceCountObject()	const { return m_refCount; }
	Ptr							GetPtr()					const { return m_ptr; }
#endif

private:
	void Incr()
	{
#ifdef TOBJECTPTR_REF_COUNTING
		if (m_refCount)
		{
			m_refCount->IncrW();
		}
#endif
	}
	void Decr()
	{
#ifdef TOBJECTPTR_REF_COUNTING
		if (m_refCount)
		{
			m_refCount->DecrW();
		}
#endif
	}

private:
	mutable Ptr m_ptr = nullptr;
#ifdef TOBJECTPTR_REF_COUNTING
	ReferenceCountObject* m_refCount = nullptr;
#endif // TOBJECTPTR_REF_COUNTING

	template<typename>
	friend class TObjectOwnPtr;
	template<typename>
	friend class TObjectPtr;
};

template<typename>
constexpr bool is_insight_unique_pointer_v = false;
template<typename T>
constexpr bool is_insight_unique_pointer_v<UPtr<T>> = true;

template<typename>
constexpr bool is_insight_shared_pointer_v = false;
template<typename T>
constexpr bool is_insight_shared_pointer_v<RPtr<T>> = true;

template<typename>
constexpr bool is_insight_weak_pointer_v = false;
template<typename T>
constexpr bool is_insight_weak_pointer_v<WPtr<T>> = true;

template<typename>
constexpr bool is_insight_object_owning_pointer_v = false;
template<typename T>
constexpr bool is_insight_object_owning_pointer_v<TObjectOwnPtr<T>> = true;

template<typename>
constexpr bool is_insight_object_pointer_v = false;
template<typename T>
constexpr bool is_insight_object_pointer_v<TObjectPtr<T>> = true;

template<typename>
constexpr bool is_insight_wrapper_pointer_v = false;
template<typename T>
constexpr bool is_insight_wrapper_pointer_v<Ptr<T>> = true;

template<typename T>
constexpr bool is_insight_smart_pointer_v = is_insight_unique_pointer_v<T> 
										 || is_insight_shared_pointer_v<T> 
										 || is_insight_weak_pointer_v<T>
										 || is_insight_object_owning_pointer_v<T>
										 || is_insight_object_pointer_v<T>
										 || is_insight_wrapper_pointer_v<T>;

template<typename>
constexpr bool is_stl_unique_pointer_v = false;
template<typename T>
constexpr bool is_stl_unique_pointer_v<std::unique_ptr<T>> = true;

template<typename>
constexpr bool is_stl_shared_pointer_v = false;
template<typename T>
constexpr bool is_stl_shared_pointer_v<std::shared_ptr<T>> = true;

template<typename>
constexpr bool is_stl_weak_pointer_v = false;
template<typename T>
constexpr bool is_stl_weak_pointer_v<std::weak_ptr<T>> = true;

template<typename T>
constexpr bool is_stl_smart_pointer_v = is_stl_unique_pointer_v<T> 
									 || is_stl_shared_pointer_v<T> 
									 || is_stl_weak_pointer_v<T>;