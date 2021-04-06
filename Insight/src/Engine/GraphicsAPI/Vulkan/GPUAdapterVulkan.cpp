#include "ispch.h"
#include "Engine/GraphicsAPI/Vulkan/GPUAdapterVulkan.h"
#include "Engine/GraphicsAPI/Vulkan/VulkanUtils.h"

GPUAdapterVulkan::GPUAdapterVulkan(const VkPhysicalDevice device)
	: Gpu(device)
{
	vkGetPhysicalDeviceProperties(device, &GpuProps);
	Description = GpuProps.deviceName;
}
