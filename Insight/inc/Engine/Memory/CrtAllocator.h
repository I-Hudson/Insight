#pragma once

#include "Engine/Platform/Platform.h"

class CrtAllocator
{
public:

	FORCE_INLINE static void* Allocate(U64 size, U64 alignment = 16)
	{
		return Platform::Allocate(size, alignment);
	}

	FORCE_INLINE static void Free(void* ptr)
	{
		Platform::Free(ptr);
	}
};