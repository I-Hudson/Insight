#include "Graphics/GPU/RHI/Vulkan/GPUAdapter_Vulkan.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::Vulkan
		{
			GPUAdapter_Vulkan::GPUAdapter_Vulkan()
				: Gpu(nullptr)
			{ }

			GPUAdapter_Vulkan::GPUAdapter_Vulkan(const GPUAdapter_Vulkan & other)
				: GPUAdapter_Vulkan()
			{
				*this = other;
			}

			GPUAdapter_Vulkan::GPUAdapter_Vulkan(const VkPhysicalDevice device)
			{
				Gpu = vk::PhysicalDevice(device);
				if (Gpu)
				{
					GpuProps = Gpu.getProperties();
				}
			}

			GPUAdapter_Vulkan::GPUAdapter_Vulkan(const vk::PhysicalDevice device)
			{
				Gpu = device;
				if (Gpu)
				{
					GpuProps = Gpu.getProperties();
				}
			}

			GPUAdapter_Vulkan& GPUAdapter_Vulkan::operator=(const GPUAdapter_Vulkan& other)
			{
				Gpu = other.Gpu;
				GpuProps = other.GpuProps;
				Description = other.Description;
				return *this;
			}

			bool GPUAdapter_Vulkan::IsValid() const
			{
				return Gpu != vk::PhysicalDevice(nullptr);
			}

			int GPUAdapter_Vulkan::GetVendorId() const
			{
				return GpuProps.vendorID;
			}

			std::string GPUAdapter_Vulkan::GetName() const
			{
				return GpuProps.deviceName;
			}
		}
	}
}