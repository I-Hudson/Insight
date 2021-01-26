#include "ispch.h"
#include "GPUAdapterVulkan.h"
#include "VulkanUtils.h"

GPUAdapterVulkan::GPUAdapterVulkan(const VkPhysicalDevice device)
	: Gpu(device)
{
	vkGetPhysicalDeviceProperties(device, &GpuProps);
	Description = GpuProps.deviceName;
}
