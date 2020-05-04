#pragma once


#ifdef  IS_PLATFORM_WINDOWS

#ifdef IS_DEBUG

#define IS_TODO(x) IS_TODO_CORE(x, __FILE__, __FUNCTION__, __LINE__)
#define IS_TODO_CORE(x, file, func, line)  \
					  printf("\033[01;33m"); \
					  printf("%s: %s: %d: %s", file, func, line, x); \
					  printf("\033[0m \n");
#else 

#define IS_TODO(...)

#endif // IS_DEBUG

#define BIT(x) (1 << x)

#ifdef IS_BUILD_DLL
	#define IS_API //__declspec(dllexport)

#pragma warning( disable : 4251 )
	#else 
		#define IS_API //__declspec(dllimport)
	#endif // IS_BUILD_DLL
#else 
	#error InSight only supports Windows!
#endif //  IS_PLATFORM_WINDOWS


