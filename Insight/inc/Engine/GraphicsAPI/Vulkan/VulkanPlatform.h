#pragma once


#ifdef IS_PLATFORM_WINDOWS
#include "Win32/Win32VulkanPlatform.h"
#else

#error Vulkan not support on this platform.

#endif