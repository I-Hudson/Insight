#pragma once

#include "Engine/Core/Core.h"
#include "vulkan/vulkan.h"

#include <glm/glm.hpp>

namespace Debug
{
	// Default validation layers
	extern int validationLayerCount;
	extern const char* validationLayerNames[];

	// Default debug callback
	VKAPI_ATTR VkBool32 VKAPI_CALL MessageCallback(
		VkDebugReportFlagsEXT flags,
		VkDebugReportObjectTypeEXT objType,
		uint64_t srcObject,
		size_t location,
		int32_t msgCode,
		const char* pLayerPrefix,
		const char* pMsg,
		void* pUserData);

	// Load debug function pointers and set debug callback
	// if callBack is NULL, default message callback will be used
	void SetupDebugging(
		VkInstance instance,
		VkDebugReportFlagsEXT flags,
		VkDebugReportCallbackEXT callBack);
	// Clear debug callback
	void FreeDebugCallback(VkInstance instance);
}

// Setup and functions for the VK_EXT_debug_marker_extension
// Extension spec can be found at https://github.com/KhronosGroup/Vulkan-Docs/blob/1.0-VK_EXT_debug_marker/doc/specs/vulkan/appendices/VK_EXT_debug_marker.txt
// Note that the extension will only be present if run from an offline debugging application
// The actual check for extension presence and enabling it on the device is done in the example base class
// See VulkanExampleBase::createInstance and VulkanExampleBase::createDevice (base/vulkanexamplebase.cpp)
namespace DebugMarker
{
	// Set to true if function pointer for the debug marker are available
	extern bool active;

	// Get function pointers for the debug report extensions from the device
	void Setup(VkDevice device);

	// Sets the debug name of an object
	// All Objects in Vulkan are represented by their 64-bit handles which are passed into this function
	// along with the object type
	void SetObjectName(VkDevice device, uint64_t object, VkDebugReportObjectTypeEXT objectType, const char* name);

	// Set the tag for an object
	void SetObjectTag(VkDevice device, uint64_t object, VkDebugReportObjectTypeEXT objectType, uint64_t name, size_t tagSize, const void* tag);

	// Start a new debug marker region
	void BeginRegion(VkCommandBuffer cmdbuffer, const char* pMarkerName, glm::vec4 color);

	// Insert a new debug marker into the command buffer
	void Insert(VkCommandBuffer cmdbuffer, std::string markerName, glm::vec4 color);

	// End the current debug marker region
	void EndRegion(VkCommandBuffer cmdBuffer);

	// Object specific naming functions
	void SetCommandBufferName(VkDevice device, VkCommandBuffer cmdBuffer, const char* name);
	void SetQueueName(VkDevice device, VkQueue queue, const char* name);
	void SetImageName(VkDevice device, VkImage image, const char* name);
	void SetSamplerName(VkDevice device, VkSampler sampler, const char* name);
	void SetBufferName(VkDevice device, VkBuffer buffer, const char* name);
	void SetDeviceMemoryName(VkDevice device, VkDeviceMemory memory, const char* name);
	void SetShaderModuleName(VkDevice device, VkShaderModule shaderModule, const char* name);
	void SetPipelineName(VkDevice device, VkPipeline pipeline, const char* name);
	void SetPipelineLayoutName(VkDevice device, VkPipelineLayout pipelineLayout, const char* name);
	void SetRenderPassName(VkDevice device, VkRenderPass renderPass, const char* name);
	void SetFramebufferName(VkDevice device, VkFramebuffer framebuffer, const char* name);
	void SetDescriptorSetLayoutName(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, const char* name);
	void SetDescriptorSetName(VkDevice device, VkDescriptorSet descriptorSet, const char* name);
	void SetSemaphoreName(VkDevice device, VkSemaphore semaphore, const char* name);
	void SetFenceName(VkDevice device, VkFence fence, const char* name);
	void SetEventName(VkDevice device, VkEvent _event, const char* name);
};