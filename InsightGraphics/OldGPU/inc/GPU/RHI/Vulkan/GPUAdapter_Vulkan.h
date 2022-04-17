#pragma once

#include "Graphics/GPU/GPUAdapter.h"
#include "vulkan/vulkan.hpp"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::Vulkan
		{
			class GPUAdapter_Vulkan : public GPUAdapter
			{
			public:
				GPUAdapter_Vulkan();
				GPUAdapter_Vulkan(const GPUAdapter_Vulkan& other);
				GPUAdapter_Vulkan(const VkPhysicalDevice device);
				GPUAdapter_Vulkan(const vk::PhysicalDevice device);

				GPUAdapter_Vulkan& operator=(const GPUAdapter_Vulkan& other);

				vk::PhysicalDevice& GetPhysicalDevice() { return Gpu; }
				vk::PhysicalDeviceProperties GetPhysicalDeviceProperties() const { return GpuProps; }

				virtual bool IsValid() const override;
				virtual int GetVendorId() const override;
				virtual std::string GetName() const override;

			private:
				vk::PhysicalDevice Gpu;
				vk::PhysicalDeviceProperties GpuProps;
				std::string Description;
			};
		}
	}
}