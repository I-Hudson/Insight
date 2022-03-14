#pragma once

#include <new>
#include "MemoryTracker.h"

#define IS_MEMORY_OVERRIDES
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
