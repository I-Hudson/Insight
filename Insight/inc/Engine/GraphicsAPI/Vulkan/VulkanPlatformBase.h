#pragma once

#include "Engine/Core/Common.h"
#include "vulkan/vulkan.h"

class VulkanPlatformBase
{
public:

	static void EnabledPhysicalDeviceFeatures(const VkPhysicalDeviceFeatures& deviceFeatures, VkPhysicalDeviceFeatures& featuresToEnable)
	{
        featuresToEnable = deviceFeatures;
        featuresToEnable.shaderResourceResidency = VK_FALSE;
        featuresToEnable.shaderResourceMinLod = VK_FALSE;
        featuresToEnable.sparseBinding = VK_FALSE;
        featuresToEnable.sparseResidencyBuffer = VK_FALSE;
        featuresToEnable.sparseResidencyImage2D = VK_FALSE;
        featuresToEnable.sparseResidencyImage3D = VK_FALSE;
        featuresToEnable.sparseResidency2Samples = VK_FALSE;
        featuresToEnable.sparseResidency4Samples = VK_FALSE;
        featuresToEnable.sparseResidency8Samples = VK_FALSE;
        featuresToEnable.sparseResidencyAliased = VK_FALSE;
	}
};