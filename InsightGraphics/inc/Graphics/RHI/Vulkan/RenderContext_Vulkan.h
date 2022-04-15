#pragma once

#include "Graphics/RenderContext.h"
#include "Core/Logger.h"
#include "Graphics/RHI/Vulkan/CommandList_Vulkan.h"
#include "Graphics/RHI/GPUBuffer.h"
#include "Graphics/RHI/Vulkan/PipelineStateObject_Vulkan.h"

#include <vulkan/vulkan.hpp>
#include <unordered_map>
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

			class GPUBuffer_Vulkan : public GPUBuffer
			{ };

			template<typename T>
			class RenderContextResources
			{
			public:

				T* CreateResource()
				{
					std::unique_ptr<T> resource = std::make_unique<T>();
					m_resources.push_back(std::move(resource));
					return m_resources.back().get();
				}

				void FreeResource(T* resource)
				{
					auto itr = std::find_if(m_resources.begin(), m_resources.end(), [resource](const std::unique_ptr<T>& ptr)
						{
							return resource == ptr.get();
						});

					if (itr == m_resources.end())
					{
						IS_CORE_ERROR("[RenderContextResources::FreeResource] Unable to find resource to free.");
						return;
					}
					m_resources.erase(itr);
				}

				void Destroy()
				{
					m_resources.resize(0);
				}

			private:
				std::vector<std::unique_ptr<T>> m_resources;
			};

			class RenderContext_Vulkan : public RenderContext
			{
			public:
				virtual bool Init() override;
				virtual void Destroy() override;

				virtual void Render(CommandList cmdList) override;

				vk::Device GetDevice() const { return m_device; }

				vk::Format GetSwapchainColourFormat() const { return m_swapchainFormat; }

				PipelineLayoutManager_Vulkan& GetPipelineLayoutManager() { return m_pipelineLayoutManager; }
				PipelineStateObjectManager_Vulkan& GetPipelineStateObjectManager() { return m_pipelineStateObjectManager; }
				RenderpassManager_Vulkan& GetRenderpassManager() { return m_renderpassManager; }

			private:
				virtual GPUBuffer* CreateVertexBuffer(u64 sizeBytes) override;
				virtual GPUBuffer* CreateIndexBuffer(u64 sizeBytes) override;
				virtual void FreeVertexBuffer(GPUBuffer* buffer) override;
				virtual void FreeIndexBuffer(GPUBuffer* buffer) override;

			private:
				vk::Instance CreateInstance();
				vk::PhysicalDevice FindAdapter();
				std::vector<vk::DeviceQueueCreateInfo> GetDeviceQueueCreateInfos(std::vector<QueueInfo>& queueInfo);
				void GetDeviceExtensionAndLayers(std::vector<std::string>& extensions, std::vector<std::string>& layers);

				vk::SwapchainKHR CreateSwapchain();

			private:
				vk::Instance m_instnace{ nullptr };
				vk::Device m_device{ nullptr };
				vk::PhysicalDevice m_adapter{ nullptr };

				vk::SurfaceKHR m_surface{ nullptr };
				vk::SwapchainKHR m_swapchain{ nullptr };
				vk::Format m_swapchainFormat;
				std::vector<vk::Image> m_swapchainImages;

				std::unordered_map<GPUQueue, vk::Queue> m_commandQueues;
				std::unordered_map<GPUQueue, u32> m_queueFamilyLookup;

				RenderContextResources<GPUBuffer_Vulkan> m_vertexBuffers;
				RenderContextResources<GPUBuffer_Vulkan> m_indexBuffers;

				PipelineLayoutManager_Vulkan m_pipelineLayoutManager;
				PipelineStateObjectManager_Vulkan m_pipelineStateObjectManager;
				RenderpassManager_Vulkan m_renderpassManager;

				const static int c_FrameCount = 2;
				int m_currentFrame = 0;

				struct FrameResource
				{
					CommandPool_Vulkan CommandPool;
					vk::Semaphore SwapchainAcquire;
					vk::Semaphore SignalSemaphore;
					vk::Fence SubmitFence;

					RenderContext_Vulkan* Context;

					void Init(RenderContext_Vulkan* context);
					void Destroy();
				};
				FrameResource m_frames[c_FrameCount];
			};
		}
	}
}