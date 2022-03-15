#pragma once

#include "Graphics/GPU/GPUDevice.h"
#include "Graphics/GPU/RHI/Vulkan/GPUAdapter_Vulkan.h"

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

			class GPUDevice_Vulkan : public GPUDevice
			{
			public:
				virtual bool Init() override;
				virtual void Destroy() override;

				virtual void WaitForGPU() const override;

				virtual GPUAdapter_Vulkan* GetAdapter() override;

				vk::Instance& GetInstance() { return m_instnace; }
				vk::Device& GetDevice() { return m_device; }
				vk::Queue& GetQueue(GPUQueue queue);
				VmaAllocator_T* GetVMAAllocator() const { return m_vmaAllocator; }

			private:
				GPUAdapter_Vulkan FindAdapter();
				std::vector<vk::DeviceQueueCreateInfo> GetDeviceQueueCreateInfos(std::vector<QueueInfo>& queueInfo);
				void GetDeviceExtensionAndLayers(std::vector<std::string>& extensions, std::vector<std::string>& layers);

			private:

				GPUAdapter_Vulkan m_adapter;
				vk::Instance m_instnace{ nullptr };
				vk::Device m_device{ nullptr };
				VmaAllocator_T* m_vmaAllocator{ nullptr };

				std::unordered_map<GPUQueue, vk::Queue> m_queues;
			};

			class GPUResource_Vulkan : public GPUDeviceResource
			{
			public:
				GPUResource_Vulkan()
				{
					m_device = dynamic_cast<GPUDevice_Vulkan*>(GraphicsManager::Instance().m_sharedData.GPUDevice);
				}

				GPUDevice_Vulkan* GetDevice() { return m_device; }

			private:
				GPUDevice_Vulkan* m_device;
			};
		}
	}
}