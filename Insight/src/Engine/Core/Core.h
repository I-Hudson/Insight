#pragma once

#pragma warning (disable : 4005)

#ifdef  IS_PLATFORM_WINDOWS

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

#define BIT(x) (1 << x)
#define ARRAY_SIZEOF(array) (sizeof(array) / sizeof(array[0]))

#define ENUM_TO_STRING(r) return #r

#ifndef IS_ASSERT_PATH
#define IS_ASSERT_PATH "./data"
#endif

#ifdef IS_BUILD_DLL
	#define IS_API __declspec(dllexport)
#pragma warning( disable : 4251 )
#else 
	#define IS_API
#endif // IS_BUILD_DLL
#else 
	#error InSight only supports Windows!
#endif //  IS_PLATFORM_WINDOWS

#ifndef INSIGHT_MAJOR
#define INSIGHT_MAJOR 1
#endif
#ifndef INSIGHT_MINOR
#define INSIGHT_MINOR 1
#endif
#ifndef INSIGHT_PATCH
#define INSIGHT_PATCH 1
#endif
