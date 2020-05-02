#pragma once


#ifdef  IS_PLATFORM_WINDOWS

#ifdef IS_DEBUG

#define IS_TODO(...)  printf("\033[0;35m"); \
					  printf(__VA_ARGS__); \
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


