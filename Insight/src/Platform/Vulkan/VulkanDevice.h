#pragma once
#ifdef IS_VULKAN
#include "Insight/Core/Core.h"
#include "VulkanHeader.h"
#include "VulkanBuffer.h"
#include "VulkanFrameBuffer.h"
#include "VmaUsage.h"

namespace vks
{
	class VulkanRenderer;
	struct RenderPassInfo;

	class IS_API VulkanDevice : public Insight::TSingleton<VulkanDevice>
	{
	public:
		explicit VulkanDevice(VkPhysicalDevice m_physicalDevice, VkInstance instance);
		~VulkanDevice();
		uint32_t        GetMemoryType(uint32_t typeBits, VkMemoryPropertyFlags m_properties, VkBool32* memTypeFound = nullptr) const;
		uint32_t&		GetQueueFamilyIndex(VkQueueFlagBits queueFlags);
		uint32_t        QueryQueueFamilyIndex(VkQueueFlagBits queueFlags) const;
		VkResult        CreateLogicalDevice(VkPhysicalDeviceFeatures m_enabledFeatures, std::vector<const char*> enabledExtensions, void* pNextChain, bool useSwapChain = true, VkQueueFlags requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);
		VkResult        CreateBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory* memory, void* data = nullptr);
		VkResult        CreateBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, vks::VulkanBuffer* buffer, void* data = nullptr);
		void			CreateBufferGPU(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, vks::VulkanBuffer* buffer, void* data);
		void            CopyBuffer(vks::VulkanBuffer* src, vks::VulkanBuffer* dst, VkQueue queue, VkBufferCopy* copyRegion = nullptr);
		VkCommandPool   CreateCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
		VkCommandBuffer CreateCommandBuffer(VkCommandBufferLevel level, VkCommandPool pool, bool begin = false, VkCommandBufferUsageFlags usage = 0);
		VkCommandBuffer CreateCommandBuffer(VkCommandBufferLevel level, bool begin = false);
		VkCommandBuffer CreateSingleUseBuffer(VkCommandBufferLevel level, bool begin = false);
		void            FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool pool, bool free = true);
		void            FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free = true);
		bool            ExtensionSupported(std::string extension);
		VkFormat        GetSupportedDepthFormat(bool checkSamplingSupport);

		VkQueue			GetQueue(VkQueueFlags queue);
		VkPipelineCache GetPipelineCache() { return m_pipelineCache; }
		VkPhysicalDevice GetPhysicalDevice() { return m_physicalDevice; }
		VkPhysicalDeviceProperties GetProperties() { return m_properties; }
		VkPhysicalDeviceFeatures GetFeatures() { return m_features; }
		VkFormat GetSwapchainFormat() { return m_swapChainFormat; }

		void CheckIdleQueue();

		void QueueIdleCommand(const std::function<void()>& funcPtr);
		void EnableIdleCommands();
		VkResult WaitForIdle();

		/** @brief Return the default render pass */
		VkRenderPass GetRenderPass() { return m_renderPass; }
		const RenderPassInfo& GetRenderPassInfo() { return m_renderPassInfo; }
		/** @brief Return the default descriptor pool */
		VkDescriptorPool GetDescriptorPool() { return m_descriptorPool; }

		VkDevice operator* () {return m_logicalDevice; }
		operator VkDevice() const { return m_logicalDevice; };

	private:
		void SetRenderPass(VkRenderPass renderPass, const RenderPassInfo& renderPassInfo);
		void CreateDescriptorPool();

	private:
		VkInstance m_instance;
		/** @brief Physical device representation */
		VkPhysicalDevice m_physicalDevice;
		/** @brief Logical device representation (application's view of the device) */
		VkDevice m_logicalDevice;
		/** @brief Properties of the physical device including limits that the application can check against */
		VkPhysicalDeviceProperties m_properties;
		/** @brief Features of the physical device that an application can use to check if a feature is supported */
		VkPhysicalDeviceFeatures m_features;
		/** @brief Features that have been enabled for use on the physical device */
		VkPhysicalDeviceFeatures m_enabledFeatures;
		/** @brief Memory types and heaps of the physical device */
		VkPhysicalDeviceMemoryProperties m_memoryProperties;
		/** @brief Queue family properties of the physical device */
		std::vector<VkQueueFamilyProperties> m_queueFamilyProperties;
		/** @brief List of extensions supported by the device */
		std::vector<std::string> m_supportedExtensions;
		/** @brief Default command pool for the graphics queue family index */
		VkCommandPool m_commandPool = VK_NULL_HANDLE;
		/** @brief Set to true when the debug marker extension is detected */
		bool m_enableDebugMarkers = false;

		VmaAllocator m_vmaAllocator;

		// Global render pass for frame buffer writes. Theses are populated from VulkanRenderer and are the default 
		// objects to use when rendering. They are used from VulkanMaterial::CreateDefault.
		VkRenderPass m_renderPass;
		RenderPassInfo m_renderPassInfo;
		VkFormat m_swapChainFormat;

		// Descriptor set pool
		VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;

		VkPipelineCache m_pipelineCache;

		bool m_enableWaitCommands;
		std::vector<std::function<void()>> m_waitIdleCommands;

		std::mutex m_mutex;

		/** @brief Contains queue family indices */
		struct
		{
			uint32_t graphics;
			uint32_t compute;
			uint32_t transfer;
		} m_queueFamilyIndices;

		struct
		{
			VkQueue graphics;
			VkQueue compute;
			VkQueue transfer;
		}m_queueFamily;

		friend class VulkanRenderer;
	};
}
#endif