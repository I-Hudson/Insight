#pragma once

#include "Engine/Graphics/GPUSync.h"
#include "Engine/GraphicsAPI/Vulkan/GPUDeviceVulkan.h"

namespace Insight::GraphicsAPI::Vulkan
{
	class GPUFenceManagerVulkan;
	class GPUSwapchain;

	class GPUFenceVulkan : public GPUResouceVulkan<Graphics::GPUFence>
	{
	public:
		GPUFenceVulkan();
		virtual ~GPUFenceVulkan() override;

		virtual void Init(const Graphics::GPUFenceDesc& desc) override;
		virtual void Wait() override;
		virtual void Reset() override;

		VkFence* GetHandleVulkan();

		//[GPUResource]
		virtual void SetName(const std::string& name) override;
	protected:
		virtual void OnReleaseGPU() override;

	private:
		GPUFenceManagerVulkan* CastManager();

		VkFence m_fenceVulkan;
		friend class GPUFenceManagerVulkan;
	};

	class GPUFenceManagerVulkan : public Graphics::GPUFenceManager
	{
	public:
		GPUFenceManagerVulkan();
		virtual ~GPUFenceManagerVulkan() override;

		virtual void Release() override;

	protected:
		virtual void AddFence(Graphics::GPUFence* fence) override;
		virtual Graphics::GPUFence* QueryFreeFence() override;
		virtual void ReleaseFence(const Graphics::GPUFence* fence) override;

	private:
		std::vector<Graphics::GPUFence*> m_freeList;
		std::vector<Graphics::GPUFence*> m_usedList;

		friend class GPUFenceVulkan;
	};


	class GPUSemaphoreVulkan : public GPUResouceVulkan<Graphics::GPUSemaphore>
	{
	public:
		GPUSemaphoreVulkan();
		virtual ~GPUSemaphoreVulkan() override;

		virtual void Init(const Graphics::GPUSemaphoreDesc& desc) override;
		virtual void Signal() override;
		virtual void Wait() override;

		//[GPUResource]
		virtual void SetName(const std::string& name) override;
	protected:
		virtual void OnReleaseGPU() override;

	private:
		GPUSemaphoreManagerVulkan* CastManager();
		VkSemaphore m_semaphoreVulkan;

		friend class GPUSwapchainVulkan;
	};

	class GPUSemaphoreManagerVulkan : public Graphics::GPUSemaphoreManager
	{
	public:
		GPUSemaphoreManagerVulkan();
		virtual ~GPUSemaphoreManagerVulkan() override;

	protected:
		virtual void AddSemaphore(Graphics::GPUSemaphore* fence) override;
		virtual void ReleaseSemaphore(const Graphics::GPUSemaphore* fence) override;

	private:
		std::vector<Graphics::GPUSemaphore*> m_semaphores;
		friend class GPUSemaphoreVulkan;
	};
}