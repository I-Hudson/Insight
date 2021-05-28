#pragma once

#include "Engine/Graphics/GPUAdapter.h"
#include "vulkan/vulkan.h"

namespace Insight::GraphicsAPI::Vulkan
{
	class GPUAdapterVulkan : public GPUAdapter
	{
	public:
		GPUAdapterVulkan()
			: Gpu(VK_NULL_HANDLE)
		{
		}

		GPUAdapterVulkan(const GPUAdapterVulkan& other)
			: GPUAdapterVulkan()
		{
			*this = other;
		}

		GPUAdapterVulkan& operator=(const GPUAdapterVulkan& other)
		{
			Gpu = other.Gpu;
			GpuProps = other.GpuProps;
			Description = other.Description;
			return *this;
		}

		GPUAdapterVulkan(const VkPhysicalDevice device);

		// [GPUAdapter]
		virtual bool IsValid() const override { return Gpu != VK_NULL_HANDLE; }
		virtual u32 GetVendorId() const { return GpuProps.vendorID; }
		virtual std::string GetDescription() const { return Description; }

		/// <summary>
		/// Handle for the physical device.
		/// </summary>
		VkPhysicalDevice Gpu;

		/// <summary>
		/// Physical device properties.
		/// </summary>
		VkPhysicalDeviceProperties GpuProps;

		std::string Description;
	};
}