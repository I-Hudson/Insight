#pragma once
#ifdef IS_VULKAN
// Include all the headers files for the Vulkan SDK

#ifdef _WIN32
#include "Insight/Log.h"
#include "Insight/Memory/MemoryManager.h"
#include "Insight/Renderer/Vulkan/VulkanInits.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#else
#define VK_USE_PLATFORM_XCB_KHR
#include <vulkan/vulkan.h>
#include <vulkan/vk_sdk_platform.h>
#endif

inline void ThrowIfFailed(VkResult res)
{
	if (res != VK_SUCCESS)
	{
		IS_CORE_ASSERT(false, res);
	}
}
#endif