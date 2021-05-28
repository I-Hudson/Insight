#pragma once

#include "VulkanHeaders.h"
#include "Engine/Graphics/GPUDevice.h"
#include "Engine/Graphics/GPUResource.h"
#include "VulkanDebug.h"
#include "Engine/GraphicsAPI/Vulkan/Debug/GPUDebugVulkan.h"
#include "Config.h"
#include "Types.h"

namespace Insight::GraphicsAPI::Vulkan
{
	class GPUFenceManagerVulkan;
	class GPUSemaphoreManagerVulkan;
	class GPUSwapchainVulkan;

	class GPUContextVulkan;
	class GPUDeviceVulkan;
	class GPUAdapterVulkan;
	class CmdBufferVulkan;
	class QueueVulkan;
	class FenceVulkan;
	class RenderPassVulkan;
	class FenceManagerVulkan;
	class PipelineEventManagerVulkan;

	class GPUTextureVulkan;
	class GPUBufferVulkan;

	class GPUDeviceVulkan : public Graphics::GPUDevice
	{
	public:
		GPUDeviceVulkan();
		virtual ~GPUDeviceVulkan() override;

		VkQueue GetQueue(GPUQueue queue);
		u32 GetQueueFamilyIndex(GPUQueue queue);

		virtual Graphics::GPUFenceManager* GetDefaultFenceManager() override;
		virtual Graphics::GPUSemaphoreManager* GetDefaultSignalManager() override;

		virtual Graphics::GPUContext* GetMainContext() override;
		virtual GPUAdapter* GetAdapter() override;
		virtual bool Init();
		virtual bool LoadContent();
		virtual bool CanDraw() { return true; }
		virtual void Dispose();
		virtual void WaitForGPU() override;
		virtual u32 GetQueueIndex(GPUQueue queue) override;

		virtual void BeginFrame() override;
		virtual void EndFrame() override;

		struct OptionalVulkanDeviceExtensions
		{
			u32 HasKHRMaintenance1 : 1;
			u32 HasKHRMaintenance2 : 1;
			u32 HasMirrorClampToEdge : 1;
			u32 HasKHRExternalMemoryCapabilities : 1;
			u32 HasKHRGetPhysicalDeviceProperties2 : 1;
			u32 HasEXTValidationCache : 1;
			u32 HasDebugMakerEXT : 1;
		};
		static OptionalVulkanDeviceExtensions OptionalDeviceExtensions;

	private:
		static void GetInstanceExtensions(std::vector<std::string>& instanceExtensions, std::vector<std::string>& layerExtensions);
		void GetDeviceExtensionsAndLayers(VkPhysicalDevice gpu, std::vector<std::string>& deviceExtensions, std::vector<std::string>& layerExtensions);

		bool CheckForDeviceExtension(const std::string& ext, bool add, std::vector<std::string>& deviceExtensions);

	public:
		VkDevice Device;
		VkInstance m_instance;

		GPUContextVulkan* MainContext = nullptr;

		GPUFenceManagerVulkan* GPUFenceManager;
		GPUSemaphoreManagerVulkan* GPUSignalManager;

		/// <summary>
		/// Graphics queue.
		/// </summary>
		QueueVulkan* GraphicsQueue;

		/// <summary>
		/// Compute queue.
		/// </summary>
		QueueVulkan* ComputeQueue;

		/// <summary>
		/// Transfer queue.
		/// </summary>
		QueueVulkan* TransferQueue;

		/// <summary>
		/// The physical device limits.
		/// </summary>
		VkPhysicalDeviceLimits PhysicalDeviceLimits;

		/// <summary>
		/// The Vulkan memory allocator.
		/// </summary>
		VmaAllocator VmaAllocator = VK_NULL_HANDLE;

	private:

		std::vector<VkQueueFamilyProperties> m_queueFamilyProps;

		VkPhysicalDeviceFeatures m_physicalDeviceFeatures;

		GPUAdapterVulkan* m_adapter = nullptr;

		CriticalSection m_fenceLock;
		CriticalSection m_pipelineEventLock;

		//TODO: Remove this when QueueVulkan is made.
		u32 m_graphicsQueueIndex;
		u32 m_computeQueueIndex;
		u32 m_transferQueueIndex;
		u32 m_graphicsQueueFamilyIndex;
		u32 m_computeQueueFamilyIndex;
		u32 m_transferQueueFamilyIndex;
		VkQueue m_graphicsQueue;
		VkQueue m_computeQueue;
		VkQueue m_transferQueue;

		friend GPUContextVulkan;
		friend FenceManagerVulkan;
		friend PipelineEventManagerVulkan;
		friend GPUSwapchainVulkan;
	};

	template<typename Resource>
	class GPUResouceVulkan : public Graphics::GPUResouceBase<GPUDeviceVulkan, Resource>
	{
	public:
	};
}