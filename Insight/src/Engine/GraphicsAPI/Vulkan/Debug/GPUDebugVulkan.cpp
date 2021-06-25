
#include "Engine/GraphicsAPI/Vulkan/Debug/GPUDebugVulkan.h"
#include "Engine/GraphicsAPI/Vulkan/GPUDeviceVulkan.h"
#include "Engine/GraphicsAPI/Vulkan/VulkanHeaders.h"
#include "Engine/GraphicsAPI/Vulkan/GPUCommandBufferVulkan.h"

namespace Insight::GraphicsAPI::Vulkan
{
	PFN_vkCmdBeginDebugUtilsLabelEXT CmdBeginDebugRegionEXT = VK_NULL_HANDLE;
	PFN_vkCmdEndDebugUtilsLabelEXT CmdEndDebugRegionEXT = VK_NULL_HANDLE;
	PFN_vkCmdInsertDebugUtilsLabelEXT CmdInsertDebugRegionEXT = VK_NULL_HANDLE;
	PFN_vkSetDebugUtilsObjectNameEXT SetDebugObjectEXT = VK_NULL_HANDLE;

	//PFN_vkCmdDebugMarkerInsertEXT pfnCmdDebugMarkerInsert = VK_NULL_HANDLE;

	VkObjectType ToVulkanDebugObjectType(Graphics::Debug::DebugObject object)
	{
		return static_cast<VkObjectType>(object);
	}

	void GPUDebugMarkerVulkan::Init()
	{
		GPUDeviceVulkan* gpuDevice = static_cast<GPUDeviceVulkan*>(GPUDeviceVulkan::Instance());
		VkDevice device = gpuDevice->Device;
		CmdBeginDebugRegionEXT = reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>(vkGetDeviceProcAddr(device, "vkCmdBeginDebugUtilsLabelEXT"));
		CmdEndDebugRegionEXT = reinterpret_cast<PFN_vkCmdEndDebugUtilsLabelEXT>(vkGetDeviceProcAddr(device, "vkCmdEndDebugUtilsLabelEXT"));
		CmdInsertDebugRegionEXT = reinterpret_cast<PFN_vkCmdInsertDebugUtilsLabelEXT>(vkGetDeviceProcAddr(device, "vkCmdInsertDebugUtilsLabelEXT"));
		SetDebugObjectEXT = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT"));

		ASSERT(SetDebugObjectEXT && "[GPUDebugMarkerVulkan::Init] 'vkDebugMarkerSetObjectNameEXT' was not found.");
	}

	void GPUDebugMarkerVulkan::SetObjectName(const std::string& name, Graphics::Debug::DebugObject debugObject, u64 objectHandle)
	{
		// Check for valid function pointer (may not be present if not running in a debugging application)
		if (SetDebugObjectEXT)
		{
			GPUDeviceVulkan* gpuDevice = static_cast<GPUDeviceVulkan*>(GPUDeviceVulkan::Instance());
			VkDevice device = gpuDevice->Device;

			VkDebugUtilsObjectNameInfoEXT nameInfo = {};
			nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
			nameInfo.objectType = ToVulkanDebugObjectType(debugObject);
			nameInfo.objectHandle = objectHandle;
			nameInfo.pObjectName = name.c_str();
			SetDebugObjectEXT(device, &nameInfo);
		}
	}

	void GPUDebugMarkerVulkan::SetObjectTag(const std::string& name, Graphics::Debug::DebugObject debugObject, u64 objectHandle)
	{
		ASSERT(false && "[GPUDebugMarkerVulkan::SetObjectTag] Not supported.");
		//// Check for valid function pointer (may not be present if not running in a debugging application)
		//if (pfnDebugMarkerSetObjectTag)
		//{
		//	GPUDeviceVulkan* gpuDevice = static_cast<GPUDeviceVulkan*>(GPUDeviceVulkan::Instance());
		//	VkDevice device = gpuDevice->Device;

		//	VkDebugMarkerObjectTagInfoEXT tagInfo = {};
		//	tagInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_TAG_INFO_EXT;
		//	tagInfo.objectType = ToVulkanDebugObjectType(debugObject);
		//	tagInfo.object = object;
		//	tagInfo.tagName = name.c_str();
		//	tagInfo.tagSize = tagSize;
		//	tagInfo.pTag = tag;
		//	pfnDebugMarkerSetObjectTag(device, &tagInfo);
		//}
	}

	void GPUDebugMarkerVulkan::BeginRegion(Graphics::GPUCommandBuffer* commandBuffer, const std::string& name, glm::vec4 colour)
	{
		// Check for valid function pointer (may not be present if not running in a debugging application)
		if (CmdBeginDebugRegionEXT)
		{
			GPUCommandBufferVulkan* cmdBuffer = static_cast<GPUCommandBufferVulkan*>(commandBuffer);
			VkDebugUtilsLabelEXT markerInfo = {};
			markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
			memcpy(markerInfo.color, &colour[0], sizeof(float) * 4);
			markerInfo.pLabelName = name.c_str();
			CmdBeginDebugRegionEXT(cmdBuffer->GetCmdBuffer(), &markerInfo);
		}
	}

	void GPUDebugMarkerVulkan::Insert(Graphics::GPUCommandBuffer* commandBuffer, const std::string& name, glm::vec4 colour)
	{
		// Check for valid function pointer (may not be present if not running in a debugging application)
		if (CmdInsertDebugRegionEXT)
		{
			GPUCommandBufferVulkan* cmdBuffer = static_cast<GPUCommandBufferVulkan*>(commandBuffer);
			VkDebugUtilsLabelEXT markerInfo = {};
			markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
			memcpy(markerInfo.color, &colour[0], sizeof(float) * 4);
			markerInfo.pLabelName = name.c_str();
			CmdInsertDebugRegionEXT(cmdBuffer->GetCmdBuffer(), &markerInfo);
		}
	}

	void GPUDebugMarkerVulkan::EndRegion(Graphics::GPUCommandBuffer* commandBuffer, const std::string& name, glm::vec4 colour)
	{
		// Check for valid function (may not be present if not running in a debugging application)
		if (CmdEndDebugRegionEXT)
		{
			GPUCommandBufferVulkan* cmdBuffer = static_cast<GPUCommandBufferVulkan*>(commandBuffer);
			CmdEndDebugRegionEXT(cmdBuffer->GetCmdBuffer());
		}
	}
}
