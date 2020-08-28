#pragma once
#ifdef IS_VULKAN
#include "Insight/Core.h"
#include "Vulkan.h"
#include "QueueFamily.h"

namespace Platform
{
	class Queue;

	class IS_API Device
	{
	public:
		Device(const std::vector<const char*> validationLayers, const std::vector<const char*> deviceExtensions, const std::vector<const char*> requiredExtensions);
		~Device();

		const VkInstance& GetInstance() const { return m_instance; }
		const VkSurfaceKHR& GetSurface() const { return m_surface; }
		const VkDevice& GetDevice() const { return m_device; }
		const VkPhysicalDevice& GetPhysicalDevice() const { return m_physicalDevice; }

		const QueueFamily& GetQueueFamily(const QueueFamilyType type) const;
		Queue& GetQueue(const QueueFamilyType type);

		uint32_t GetMemoryType(const uint32_t& memoryType, const VkMemoryPropertyFlags& memProb);

		VkPhysicalDeviceProperties GetDeviceProperties() { return m_deviceProperties; }

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
		VkPhysicalDeviceProperties m_deviceProperties;

		std::vector<const char*> m_validationLayers;
		std::vector<const char*> m_deviceExtensions;

		VkDebugUtilsMessengerEXT m_debugMessenger;

		std::vector<QueueFamily> m_queueFamily;
		std::vector<Queue> m_queues;
	};
}
#endif