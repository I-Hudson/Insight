#pragma once 

#include "Core/Defines.h"
#include "Core/TypeAlias.h"

//#define IS_MEMORY_OVERRIDES
#ifdef IS_MEMORY_OVERRIDES
void* operator new(size_t size);
void* operator new[](size_t size);

void operator delete(void* ptr);
void operator delete(void* ptr, u64 bytes);
void operator delete[](void* ptr);
void operator delete[](void* ptr, u64 bytes);

#if C_PLUS_PLUS_STANDARD >= 201703L
void* operator new(size_t size, std::align_val_t al);
void* operator new[](size_t size, std::align_val_t al);

void operator delete(void* ptr, std::align_val_t al);
void operator delete(void* ptr, u64 bytes, std::align_val_t al);
void operator delete[](void* ptr, std::align_val_t al);
void operator delete[](void* ptr, u64 bytes, std::align_val_t al);
#endif // __cplusplus > 201703L
#endif