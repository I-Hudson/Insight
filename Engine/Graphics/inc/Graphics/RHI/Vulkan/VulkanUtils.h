#pragma once

#if defined(IS_VULKAN_ENABLED)

#include "Graphics/Enums.h"
#include "Graphics/PixelFormat.h"
#include <vulkan/vulkan.hpp>
#include "VmaUsage.h"

extern vk::Format PixelFormatToVkFormat[static_cast<int>(PixelFormat::MAX)];
extern PixelFormat VkFormatToPixelFormat[static_cast<int>(PixelFormat::MAX)];
using VmaAllocationCreateFlags = uint32_t;
enum VmaMemoryUsage;

namespace Insight
{
	namespace Graphics
	{
		std::string						DeviceExtensionToVulkan(DeviceExtension extension);
		vk::ImageUsageFlags				ImageUsageFlagsToVulkan(ImageUsageFlags imageUsageFlags);
		vk::Format						PixelFormatToVulkan(PixelFormat format);
		vk::PipelineBindPoint			GPUQueueToVulkanBindPoint(GPUQueue queue);
		vk::PipelineStageFlags			PipelineStageFlagsToVulkan(PipelineStageFlags flags);
		vk::ShaderStageFlagBits			ShaderStageFlagBitsToVulkan(ShaderStageFlagBits stage);
		vk::ShaderStageFlags			ShaderStageFlagsToVulkan(ShaderStageFlags flags);
		vk::PrimitiveTopology			PrimitiveTopologyTypeToVulkan(PrimitiveTopologyType type);
		vk::ColorComponentFlags			ColourComponentFlagsToVulkan(ColourComponentFlags flags);
		vk::BlendFactor					BlendFactorToVulkan(BlendFactor factor);
		vk::BlendOp						BlendOpToVulkan(BlendOp op);
		vk::CompareOp					CompareOpToVulkan(CompareOp op);
		vk::Filter						FilterToVulkan(Filter filter);
		vk::SamplerMipmapMode			SamplerMipmapModeToVulkan(SamplerMipmapMode sampler_mipmap_mode);
		vk::SamplerAddressMode			SamplerAddressModeToVulkan(SamplerAddressMode sampler_address_mode);
		vk::BorderColor					BorderColourToVulkan(BorderColour border_colour);
		vk::AttachmentLoadOp			AttachmentLoadOpToVulkan(AttachmentLoadOp op);
		vk::BufferUsageFlags			BufferTypeToVulkanBufferUsageFlags(BufferType type);
		vk::PolygonMode					PolygonModeToVulkan(PolygonMode mode);
		vk::CullModeFlags				CullModeToVulkan(CullMode cullMode);
		vk::FrontFace					FrontFaceToVulkan(FrontFace front_face);
		vk::DescriptorType				DescriptorTypeToVulkan(DescriptorType type);
		vk::AccessFlags					AccessFlagsToVulkan(AccessFlags flags);
		vk::ImageLayout					ImageLayoutToVulkan(ImageLayout layout);
		vk::ImageAspectFlags			ImageAspectFlagsToVulkan(ImageAspectFlags flags);
		vk::IndexType					IndexTypeToVulkan(IndexType index_type);
		vk::ImageType					TextureTypeToVulkan(TextureType type);
		vk::ImageViewType				TextureViewTypeToVulkan(TextureType type);
		vk::DynamicState				DynamicStateToVulkan(DynamicState dynamic_state);
		std::vector<vk::DynamicState>	DynamicStatesToVulkan(std::vector<DynamicState> dynamic_states);

		VmaAllocationCreateFlags	BufferTypeToVMAAllocCreateFlags(BufferType type);
		VmaMemoryUsage				BufferTypeToVMAUsage(BufferType type);

		ImageLayout					VkToImageLayout(vk::ImageLayout layout);
		AttachmentLoadOp			VkToAttachmentLoadOp(vk::AttachmentLoadOp op);
		

		/*
		std::string VkErrorToString(VkResult errorCode)
		{
			switch (errorCode)
			{
#define STR(r) case VK_ ##r: return #r
				STR(NOT_READY);
				STR(TIMEOUT);
				STR(EVENT_SET);
				STR(EVENT_RESET);
				STR(INCOMPLETE);
				STR(ERROR_OUT_OF_HOST_MEMORY);
				STR(ERROR_OUT_OF_DEVICE_MEMORY);
				STR(ERROR_INITIALIZATION_FAILED);
				STR(ERROR_DEVICE_LOST);
				STR(ERROR_MEMORY_MAP_FAILED);
				STR(ERROR_LAYER_NOT_PRESENT);
				STR(ERROR_EXTENSION_NOT_PRESENT);
				STR(ERROR_FEATURE_NOT_PRESENT);
				STR(ERROR_INCOMPATIBLE_DRIVER);
				STR(ERROR_TOO_MANY_OBJECTS);
				STR(ERROR_FORMAT_NOT_SUPPORTED);
				STR(ERROR_SURFACE_LOST_KHR);
				STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
				STR(SUBOPTIMAL_KHR);
				STR(ERROR_OUT_OF_DATE_KHR);
				STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
				STR(ERROR_VALIDATION_FAILED_EXT);
				STR(ERROR_INVALID_SHADER_NV);
#undef STR
			default:
				return "UNKNOWN_ERROR";
			}
		}
		*/

		inline void ThrowIfFailed(VkResult errorCode)
		{
			assert(errorCode == VK_SUCCESS);///&& VkErrorToString(errorCode).c_str());
		}
	}
}

#endif ///#if defined(IS_VULKAN_ENABLED)