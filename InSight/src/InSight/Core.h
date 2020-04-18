#pragma once

#ifdef  IS_PLATFORM_WINDOWS
	#ifdef IS_BUILD_DLL
		#define IS_API __declspec(dllexport)
	#else 
		#define IS_API __declspec(dllimport)
	#endif // IS_BUILD_DLL
#else 
	#error InSight only supports Windows!
#endif //  IS_PLATFORM_WINDOWS


