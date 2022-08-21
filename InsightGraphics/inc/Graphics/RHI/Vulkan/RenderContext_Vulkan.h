#pragma once

#if defined(IS_VULKAN_ENABLED)

#include "Graphics/RenderContext.h"
#include "Core/Logger.h"
#include "Graphics/RHI/Vulkan/PipelineStateObject_Vulkan.h"
#include "Graphics/RHI/RHI_Buffer.h"
#include "Graphics/RHI/Vulkan/RHI_Descriptor_Vulkan.h"
#include "VmaUsage.h"
#include "Graphics/RenderGraph/RenderGraph.h"

#include <glm/vec2.hpp>
#include <vulkan/vulkan.hpp>
#include <unordered_map>
#include <set>
#include <string>

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::Vulkan
		{
			struct QueueInfo
			{
				int FamilyQueueIndex;
				GPUQueue Queue;
			};

			struct FrameResource_Vulkan : public FrameResouce
			{
				RenderContext_Vulkan* Context;
				DescriptorAllocator_Vulkan DescriptorAllocator;

				vk::Semaphore SwapchainAcquire;
				vk::Semaphore SignalSemaphore;
				vk::Fence SubmitFence;

				void Init(RenderContext_Vulkan* context);
				void Destroy();
				void Reset();
			};

			class RenderContext_Vulkan : public RenderContext
			{
			public:
				virtual bool Init() override;
				virtual void Destroy() override;

				virtual void InitImGui() override;
				virtual void DestroyImGui() override;

				virtual void Render(CommandList cmdList) override;

				virtual bool PrepareRender() override;
				virtual void PostRender(RHI_CommandList* cmdList) override;

				virtual void GpuWaitForIdle() override;
				virtual void SubmitCommandListAndWait(RHI_CommandList* cmdList) override;

				void SetObejctName(std::wstring_view name, u64 handle, vk::ObjectType objectType);
				vk::Device GetDevice() const { return m_device; }
				VmaAllocator GetVMA() const { return m_vmaAllocator; }

				u32 GetFamilyQueueIndex(GPUQueue queue) const { return m_queueFamilyLookup.at(queue); }

				vk::ImageView GetSwapchainImageView() const { return m_swapchainImageViews[m_availableSwapchainImage]; }
				vk::Format GetSwapchainColourFormat() const { return m_swapchainFormat; }
				vk::SwapchainKHR GetSwapchain() const { return m_swapchain; }

				PipelineLayoutManager_Vulkan& GetPipelineLayoutManager() { return m_pipelineLayoutManager; }
				PipelineStateObjectManager_Vulkan& GetPipelineStateObjectManager() { return m_pipelineStateObjectManager; }
				RenderpassManager_Vulkan& GetRenderpassManager() { return m_renderpassManager; }

				FrameResource_Vulkan& GetFrameResouce() { return m_frames[m_currentFrame]; }

			protected:
				virtual void WaitForGpu() override;

			private:
				vk::Instance CreateInstance();
				vk::PhysicalDevice FindAdapter();
				std::vector<vk::DeviceQueueCreateInfo> GetDeviceQueueCreateInfos(std::vector<QueueInfo>& queueInfo);
				void GetDeviceExtensionAndLayers(std::set<std::string>& extensions, std::set<std::string>& layers, bool includeAll = false);
				void CreateSwapchain();

				void SetDeviceExtensions();

			private:
				vk::Instance m_instnace{ nullptr };
				vk::Device m_device{ nullptr };
				vk::PhysicalDevice m_adapter{ nullptr };

				VmaAllocator m_vmaAllocator{ nullptr };

				vk::SurfaceKHR m_surface{ nullptr };
				vk::SwapchainKHR m_swapchain{ nullptr };
				vk::Format m_swapchainFormat;
				std::vector<vk::Image> m_swapchainImages;
				std::vector<vk::ImageView> m_swapchainImageViews;
				glm::ivec2 m_swapchainBufferSize;

				std::unordered_map<GPUQueue, vk::Queue> m_commandQueues;
				std::unordered_map<GPUQueue, u32> m_queueFamilyLookup;

				PipelineLayoutManager_Vulkan m_pipelineLayoutManager;
				PipelineStateObjectManager_Vulkan m_pipelineStateObjectManager;
				RenderpassManager_Vulkan m_renderpassManager;

				vk::DescriptorPool m_imguiDescriptorPool;
				vk::RenderPass m_imguiRenderpass;
				std::array<vk::Framebuffer, 0> m_imguiFramebuffers;

				int m_currentFrame = 0;
				int m_availableSwapchainImage = 0;

#ifdef RENDER_GRAPH_ENABLED
				FrameResource<vk::Fence> m_submitFences;
				FrameResource<vk::Semaphore> m_swapchainAcquires;
				FrameResource<vk::Semaphore> m_signalSemaphores;
#endif
				std::vector<FrameResource_Vulkan> m_frames;
			};
		}
	}
}

#endif //#if defined(IS_VULKAN_ENABLED)