#pragma once 

#include "Core/TypeAlias.h"

//#define IS_MEMORY_OVERRIDES
#ifdef IS_MEMORY_OVERRIDES
void* operator new(size_t size);
void* operator new[](size_t size);

void operator delete(void* ptr);
void operator delete(void* ptr, u64 bytes);
void operator delete[](void* ptr);
void operator delete[](void* ptr, u64 bytes);
#endif