#pragma once
#ifdef IS_VULKAN
// Include all the headers files for the Vulkan SDK

#ifdef _WIN32
#include "Insight/Log.h"
#include "Insight/Memory/MemoryManager.h"
#include "VulkanInits.h"
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#else
#define VK_USE_PLATFORM_XCB_KHR
#include <vulkan/vk_sdk_platform.h>
#endif

std::string errorString(VkResult errorcode);

inline void ThrowIfFailed(VkResult res)
{
	if (res != VK_SUCCESS)
	{	
		IS_CORE_ASSERT(false, errorString(res).c_str());
	}
}

inline std::string errorString(VkResult errorCode)
{
	switch (errorCode)
	{
#define STR(r) case VK_ ##r: return #r
		STR(NOT_READY);
		STR(TIMEOUT);
		STR(EVENT_SET);
		STR(EVENT_RESET);
		STR(INCOMPLETE);
		STR(ERROR_OUT_OF_HOST_MEMORY);
		STR(ERROR_OUT_OF_DEVICE_MEMORY);
		STR(ERROR_INITIALIZATION_FAILED);
		STR(ERROR_DEVICE_LOST);
		STR(ERROR_MEMORY_MAP_FAILED);
		STR(ERROR_LAYER_NOT_PRESENT);
		STR(ERROR_EXTENSION_NOT_PRESENT);
		STR(ERROR_FEATURE_NOT_PRESENT);
		STR(ERROR_INCOMPATIBLE_DRIVER);
		STR(ERROR_TOO_MANY_OBJECTS);
		STR(ERROR_FORMAT_NOT_SUPPORTED);
		STR(ERROR_SURFACE_LOST_KHR);
		STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
		STR(SUBOPTIMAL_KHR);
		STR(ERROR_OUT_OF_DATE_KHR);
		STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
		STR(ERROR_VALIDATION_FAILED_EXT);
		STR(ERROR_INVALID_SHADER_NV);
#undef STR
	default:
		return "UNKNOWN_ERROR";
	}
}
#endif