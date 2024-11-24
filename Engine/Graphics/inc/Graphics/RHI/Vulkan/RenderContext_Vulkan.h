#pragma once

#if defined(IS_VULKAN_ENABLED)

#include "Graphics/RenderContext.h"
#include "Graphics/RHI/RHI_Buffer.h"
#include "Graphics/RHI/Vulkan/RHI_Descriptor_Vulkan.h"
#include "Graphics/RHI/RHI_GPUCrashTracker.h"

#include "Graphics/RenderGraph/RenderGraph.h"

#include <glm/vec2.hpp>
#include <unordered_map>
#include <set>
#include <string>

struct VmaAllocator_T;

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

			/// <summary>
			/// Store all the objects needed for a single frame submit.
			/// </summary>
			struct FrameSubmitContext
			{
				std::vector<RHI_CommandList*> CommandLists;
				VkFence SubmitFences;
				VkSemaphore SwapchainAcquires;
				VkSemaphore SignalSemaphores;
			};

			class RenderContext_Vulkan : public RenderContext
			{
			public:
				virtual bool Init(RenderContextDesc desc) override;
				virtual void Destroy() override;

				virtual void InitImGui() override;
				virtual void DestroyImGui() override;

				virtual bool PrepareRender() override;
				virtual void PreRender(RHI_CommandList* cmdList) override;
				virtual void PostRender(RHI_CommandList* cmdList) override;

				virtual void CreateSwapchain(SwapchainDesc desc) override;
				virtual void SetSwaphchainResolution(Maths::IVector2 resolution) override;
				virtual Maths::IVector2 GetSwaphchainResolution() const override;

				virtual void GpuWaitForIdle() override;
				virtual void SubmitCommandListAndWait(RHI_CommandList* cmdList) override;

				virtual void MarkTimeStamp(RHI_CommandList* cmdList) { FAIL_ASSERT(); }
				virtual std::vector<u64> ResolveTimeStamps(RHI_CommandList* cmdList) { FAIL_ASSERT(); return {}; }
				virtual u64 GetTimeStampFrequency() { FAIL_ASSERT(); return 0; }

				virtual void ExecuteAsyncJobs(RHI_CommandList* cmdList) override;

				void SetObjectName(std::string_view name, u64 handle, VkObjectType objectType);

				VkDevice GetDevice() const { return m_device; }
				VkPhysicalDevice GetPhysicalDevice() const { return m_adapter; }
				VmaAllocator_T* GetVMA() const { return m_vmaAllocator; }

				u32 GetFamilyQueueIndex(GPUQueue queue) const { return m_queueFamilyLookup.at(queue); }

				virtual RHI_Texture* GetSwaphchainIamge() const override;
				VkImageView GetSwapchainImageView() const;
				VkFormat GetSwapchainColourFormat() const { return m_swapchainFormat; }
				VkSwapchainKHR GetSwapchain() const { return m_swapchain; }

				void* GetExtensionFunction(const char* function);

			protected:
				virtual void WaitForGpu() override;

			private:
				void CreateInstance();
				VkPhysicalDevice FindAdapter();
				std::vector<VkDeviceQueueCreateInfo> GetDeviceQueueCreateInfos(std::vector<QueueInfo>& queueInfo);
				void GetDeviceExtensionAndLayers(std::set<std::string>& extensions, std::set<std::string>& layers, bool includeAll = false);

				void SetDeviceExtensions();
				bool CheckInstanceExtension(const char* extension);
				bool CheckForDeviceExtension(const char* extension);

			private:
				VkInstance m_instnace{ nullptr };
				VkDevice m_device{ nullptr };
				VkPhysicalDevice m_adapter{ nullptr };

				VmaAllocator_T* m_vmaAllocator{ nullptr };
				VkAllocationCallbacks m_vmaAllocationCallbacks;

				VkSurfaceKHR m_surface{ nullptr };
				VkSwapchainKHR m_swapchain{ nullptr };
				VkFormat m_swapchainFormat;
				std::vector<RHI_Texture*> m_swapchainImages;

				std::unordered_map<GPUQueue, VkQueue> m_commandQueues;
				std::unordered_map<GPUQueue, std::mutex> m_command_queue_mutexs;
				std::unordered_map<GPUQueue, u32> m_queueFamilyLookup;

				VkDescriptorPool m_imguiDescriptorPool;
				VkRenderPass m_imguiRenderpass;
				std::array<VkFramebuffer, 0> m_imguiFramebuffers;

				u32 m_currentFrame = 0;
				u32 m_availableSwapchainImage = 0;

				Insight::Graphics::RHI_GPUCrashTracker* m_gpuCrashTracker = nullptr;

			public:
				VkDescriptorPool m_descriptor_pool = nullptr;

			private:

				FrameResource<FrameSubmitContext> m_submitFrameContexts;
			};
		}
	}
}

#endif ///#if defined(IS_VULKAN_ENABLED)