#pragma once
#ifdef IS_VULKAN
#include "Insight/Core.h"

#include "Insight/Renderer/Vulkan/QueueFamily.h"

namespace Insight
{
	class Window;

	namespace Render
	{
		class Queue;

		struct DeviceSettings
		{
			std::vector<const char*> ValidationLayers;
			std::vector<const char*> DeviceExtensions;
			std::vector<const char*> RequiredExtensions;
			Window* Window;
		};

		class IS_API Device
		{
		public:
			Device(DeviceSettings& deviceSettings);
			~Device();

			const VkInstance& GetInstance() const { return m_instance; }
			const VkSurfaceKHR& GetSurface() const { return m_surface; }
			const VkDevice& GetDevice() const { return m_device; }
			const VkPhysicalDevice& GetPhysicalDevice() const { return m_physicalDevice; }

			const QueueFamily& GetQueueFamily(const QueueFamilyType type) const;
			Queue& GetQueue(const QueueFamilyType type);

			uint32_t GetMemoryType(const uint32_t& memoryType, const VkMemoryPropertyFlags& memProb);

			void WaitForIdle();

			void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

		private:

			bool IsDeviceSuitable(VkPhysicalDevice device);

			void CreateSurface();
			void SelectPhysicalDevice();
			void CreateDevice();

		private:
			VkInstance m_instance;
			VkSurfaceKHR m_surface;
			VkDevice m_device;
			VkPhysicalDevice m_physicalDevice;

			DeviceSettings m_deviceSettings;

			VkDebugUtilsMessengerEXT m_debugMessenger;

			std::vector<QueueFamily> m_queueFamily;
			std::vector<Queue> m_queues;
		};
	}
}
#endif