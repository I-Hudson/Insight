#pragma once

#pragma warning (disable : 4005)

#ifdef  IS_PLATFORM_WINDOWS

#include "InsightAlias.h"
#include "Insight/Object.h"
#include "Insight/Instrumentor/Instrumentor.h"

#ifdef IS_DEBUG

#define IS_TODO(x) IS_TODO_CORE(x, __FILE__, __FUNCTION__, __LINE__)
#define IS_TODO_CORE(x, file, func, line)  \
					  printf("\033[01;33m"); \
					  printf("%s: %s: %d: %s", file, func, line, x); \
					  printf("\033[0m \n");

#define IS_IMPLERMENT(x) IS_IMPLERMENT_CORE(x, __FILE__, __FUNCTION__, __LINE__)
#define IS_IMPLERMENT_CORE(x, file, func, line) \
					 printf("//------------------------------------------------ \n"); \
					 printf("Missing IMPLEREMNT: %s  \n", x); \
					 printf("%s, %d, %s  \n", func, line, file); \
					 printf("//------------------------------------------------ \n");


#else 

#define IS_TODO(...)
#define IS_TODO_CORE(...)
#define IS_IMPLERMENT(...)
#define IS_IMPLERMENT_CORE(...)

#endif // IS_DEBUG 

#define NEW_ON_HEAP(type, ...) Insight::Memory::MemoryManager::NewOnFreeList<type>(__VA_ARGS__)
#define NEW_ARR_ON_HEAP(type, ...) Insight::Memory::MemoryManager::NewArrOnFreeList<type>(__VA_ARGS__)

#define DELETE_ON_HEAP(ptr)  if(ptr) { Insight::Memory::MemoryManager::DeleteOnFreeList(ptr); ptr = nullptr; }
#define DELETE_ARR_ON_HEAP(ptr) if(ptr) { Insight::Memory::MemoryManager::DeleteArrOnFreeList(ptr); ptr = nullptr; }

#define NEW_ON_STACK(type, ...) Insight::Memory::MemoryManager::NewOnStack<type>(__VA_ARGS__)
#define DELETE_ON_STACK(ptr)  if(ptr) { Insight::Memory::MemoryManager::DeleteOnStack((uint64_t)ptr); ptr = nullptr; }

#define TRACK_CLASS() Insight::Memory::MemoryManager::TrackObject(this, "Class", __FILE__, __LINE__);
#define UNTRACK_CLASS() Insight::Memory::MemoryManager::UnTrackObject(this);

#define TRACK_OBJECT(x) Insight::Memory::MemoryManager::TrackObject(x, "Object (Ptr/Ref)", __FILE__, __LINE__);
#define UNTRACK_OBJECT(x) Insight::Memory::MemoryManager::UnTrackObject(x);

#define GET_ALLOCATION_OF_TYPE(ptr) Insight::Memory::MemoryManager::Instance()->GetAllocationOfType(ptr);

#define BIT(x) (1 << x)
#define ARRAY_SIZEOF(ARR) ((int)(sizeof(ARR) / sizeof(*(ARR))))

#ifndef IS_ASSERT_PATH
#define IS_ASSERT_PATH "./data"
#endif

#ifdef IS_BUILD_DLL
	#define IS_API //__declspec(dllexport)

#pragma warning( disable : 4251 )
	#else 
		#define IS_API //__declspec(dllimport)
	#endif // IS_BUILD_DLL
#else 
	#error InSight only supports Windows!
#endif //  IS_PLATFORM_WINDOWS