#pragma once

#include "Engine/Core/Core.h"
#include "Engine/GraphicsAPI/Vulkan/VulkanHeaders.h"

namespace Insight::GraphicsAPI::Vulkan
{
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
}