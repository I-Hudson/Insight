#pragma once

#include "Graphics/Enums.h"
#include "Graphics/PixelFormat.h"
#include <vulkan/vulkan.hpp>

extern vk::Format PixelFormatToVkFormat[static_cast<int>(PixelFormat::MAX)];
extern PixelFormat VkFormatToPixelFormat[static_cast<int>(PixelFormat::MAX)];
using VmaAllocationCreateFlags = uint32_t;
enum VmaMemoryUsage;

namespace Insight
{
	namespace Graphics
	{
		vk::ImageUsageFlags ImageUsageFlagsToVulkan(ImageUsageFlags imageUsageFlags);
		vk::Format PixelFormatToVulkan(PixelFormat format);
		vk::PipelineBindPoint GPUQueueToVulkanBindPoint(GPUQueue queue);
		vk::ShaderStageFlagBits ShaderStageFlagBitsToVulkan(ShaderStageFlagBits stage);
		vk::PrimitiveTopology PrimitiveTopologyTypeToVulkan(PrimitiveTopologyType type);
		vk::ColorComponentFlags ColourComponentFlagsToVulkan(ColourComponentFlags flags);
		vk::BlendFactor BlendFactorToVulkan(BlendFactor factor);
		vk::BlendOp BlendOpToVulkan(BlendOp op);
		vk::BufferUsageFlags GPUBufferTypeToVulkanBufferUsageFlags(GPUBufferType type);
		vk::CullModeFlags CullModeToVulkan(CullMode cullMode);

		VmaAllocationCreateFlags GPUBufferTypeToVMAAllocCreateFlags(GPUBufferType type);
		VmaMemoryUsage GPUBufferTypeToVMAUsage(GPUBufferType type);
	}
}