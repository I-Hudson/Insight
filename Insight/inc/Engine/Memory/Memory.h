#pragma once

#include "CrtAllocator.h"
#include "ProfilerMemory.h"
using Allocator = CrtAllocator;

/// <summary>
/// Add additional information to the allocation just made.
/// </summary>
/// <typeparam name="T"></typeparam>
/// <param name="ptr"></param>
template<typename T>
INLINE void AddAdditionInformation(T* ptr)
{
	Insight::Core::ProfilerMemory::Instance()->AddName(ptr, typeid(T).name());
}


/// <summary>
/// Allocate a single block of memory and create a new object there.
/// </summary>
/// <typeparam name="T"></typeparam>
/// <typeparam name="MemoryAllocator"></typeparam>
/// <returns></returns>
template<typename T, class MemoryAllocator = Allocator>
INLINE T* New()
{
	T* ptr = (T*)MemoryAllocator::Allocate(sizeof(T));
	new(ptr) T();

#ifdef IS_DEBUG
	AddAdditionInformation(ptr);
#endif

	return ptr;
}

/// <summary>
/// Allocate a single block of memory and create a new object there with specified parameters.
/// </summary>
/// <typeparam name="T"></typeparam>
/// <typeparam name="MemoryAllocator"></typeparam>
/// <typeparam name="...Args"></typeparam>
/// <param name="...args"></param>
/// <returns></returns>
template<typename T, class MemoryAllocator = Allocator, typename... Args>
INLINE T* New(Args&&... args)
{
	T* ptr = (T*)MemoryAllocator::Allocate(sizeof(T));
	new(ptr) T(std::forward<Args>(args)...);

#ifdef IS_DEBUG
	AddAdditionInformation(ptr);
#endif

	return ptr;
}

/// <summary>
/// Allocate multiple blocks for an array of type T.
/// </summary>
/// <typeparam name="T"></typeparam>
/// <typeparam name="MemoryAllocator"></typeparam>
/// <param name="count"></param>
/// <returns></returns>
template<typename T, class MemoryAllocator = Allocator>
INLINE T* NewArray(u32 count)
{
	T* ptr = (T*)MemoryAllocator::Allocate(sizeof(T) * count);
	T* dst = ptr;
	while (--count)
	{
		new(dst) T();
		++(T*&)dst;
	}

#ifdef IS_DEBUG
	AddAdditionInformation(ptr);
#endif

	return ptr;
}

/// <summary>
/// Allocate multiple blocks for an unknown type.
/// </summary>
/// <typeparam name="MemoryAllocator"></typeparam>
/// <param name="sizeInBytes"></param>
/// <returns></returns>
template<class MemoryAllocator = Allocator>
INLINE void* NewArrayBytes(u32 sizeInBytes, u64 alignemnt = 16)
{
	void* ptr = MemoryAllocator::Allocate(sizeInBytes, alignemnt);

#ifdef IS_DEBUG
	AddAdditionInformation<void>(ptr);
#endif
	return ptr;
}

///
template<typename T, class MemoryAllocator = Allocator>
INLINE void Delete(T* ptr)
{
	ptr->~T();
	Platform::Free(ptr);
}

template<typename T, class MemoryAllocator = Allocator>
INLINE void DeleteArray(T* ptr, u32 count)
{
	T* dst = ptr;
	while (--count)
	{
		dst->~T();
		++dst;
	}
	Platform::Free(ptr);
	ptr = nullptr;
}

INLINE void DeleteArrayBytes(void* ptr)
{
	Platform::Free(ptr);
}