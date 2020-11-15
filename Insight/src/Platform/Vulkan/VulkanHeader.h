#pragma once
#ifdef IS_VULKAN
// Include all the headers files for the Vulkan SDK

#ifdef _WIN32
#include "Insight/Log.h"
#include "Insight/Memory/MemoryManager.h"
#include "VulkanInitializers.h"
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#else
#define VK_USE_PLATFORM_XCB_KHR
#include <vulkan/vk_sdk_platform.h>
#endif

// Custom define for better code readability
#define VK_FLAGS_NONE 0
// Default fence timeout in nanoseconds
#define DEFAULT_FENCE_TIMEOUT 100000000000

namespace vks
{
	/** @brief Disable message boxes on fatal errors */
	extern bool errorModeSilent;

	/** @brief Returns an error code as a string */
	std::string errorString(VkResult errorCode);

	inline void ThrowIfFailed(VkResult res)
	{
		if (res != VK_SUCCESS)
		{
			IS_CORE_ASSERT(false, errorString(res).c_str());
		}
	}

	/** @brief Returns the device type as a string */
	std::string physicalDeviceTypeString(VkPhysicalDeviceType type);

	// Selected a suitable supported depth format starting with 32 bit down to 16 bit
	// Returns false if none of the depth formats in the list is supported by the device
	VkBool32 getSupportedDepthFormat(VkPhysicalDevice physicalDevice, VkFormat* depthFormat);

	// Returns if a given format support LINEAR filtering
	VkBool32 formatIsFilterable(VkPhysicalDevice physicalDevice, VkFormat format, VkImageTiling tiling);

	// Put an image memory barrier for setting an image layout on the sub resource into the given command buffer
	void setImageLayout(
		VkCommandBuffer cmdbuffer,
		VkImage image,
		VkImageLayout oldImageLayout,
		VkImageLayout newImageLayout,
		VkImageSubresourceRange subresourceRange,
		VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
	// Uses a fixed sub resource layout with first mip level and layer
	void setImageLayout(
		VkCommandBuffer cmdbuffer,
		VkImage image,
		VkImageAspectFlags aspectMask,
		VkImageLayout oldImageLayout,
		VkImageLayout newImageLayout,
		VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

	/** @brief Insert an image memory barrier into the command buffer */
	void insertImageMemoryBarrier(
		VkCommandBuffer cmdbuffer,
		VkImage image,
		VkAccessFlags srcAccessMask,
		VkAccessFlags dstAccessMask,
		VkImageLayout oldImageLayout,
		VkImageLayout newImageLayout,
		VkPipelineStageFlags srcStageMask,
		VkPipelineStageFlags dstStageMask,
		VkImageSubresourceRange subresourceRange);

	// Load a SPIR-V shader (binary)
#if defined(__ANDROID__)
	VkShaderModule loadShader(AAssetManager* assetManager, const char* fileName, VkDevice device);
#else
	std::vector<uint32_t> loadShaderBinary(const std::string& fileName);
	std::string loadShaderString(const std::string& fileName);
	VkShaderModule loadShader(const std::string& fileName, VkDevice device);
	VkPipelineShaderStageCreateInfo  loadShader(const std::string& fileName, VkDevice device, VkShaderStageFlagBits stage);
#endif

	/** @brief Checks if a file exists */
	bool fileExists(const std::string& filename);
}
#endif