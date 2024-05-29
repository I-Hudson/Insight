#pragma once

#if defined(IS_VULKAN_ENABLED)

#include "Graphics/Enums.h"
#include "Graphics/PixelFormat.h"
#include "Core/Asserts.h"

#include "VmaUsage.h"

#include <vector>
#include <string>

extern VkFormat PixelFormatToVkFormat[static_cast<int>(PixelFormat::MAX)];
extern PixelFormat VkFormatToPixelFormat[static_cast<int>(PixelFormat::MAX)];
using VmaAllocationCreateFlags = uint32_t;
enum VmaMemoryUsage;

namespace Insight
{
	namespace Graphics
	{
		std::string						DeviceExtensionToVulkan(DeviceExtension extension);
		VkImageUsageFlags				ImageUsageFlagsToVulkan(ImageUsageFlags imageUsageFlags);
		VkFormat						PixelFormatToVulkan(PixelFormat format);
		VkPipelineBindPoint				GPUQueueToVulkanBindPoint(GPUQueue queue);
		VkPipelineStageFlags			PipelineStageFlagsToVulkan(PipelineStageFlags flags);
		VkShaderStageFlagBits			ShaderStageFlagBitsToVulkan(ShaderStageFlagBits stage);
		VkShaderStageFlags				ShaderStageFlagsToVulkan(ShaderStageFlags flags);
		VkPrimitiveTopology				PrimitiveTopologyTypeToVulkan(PrimitiveTopologyType type);
		VkColorComponentFlags			ColourComponentFlagsToVulkan(ColourComponentFlags flags);
		VkBlendFactor					BlendFactorToVulkan(BlendFactor factor);
		VkBlendOp						BlendOpToVulkan(BlendOp op);
		VkCompareOp						CompareOpToVulkan(CompareOp op);
		VkFilter						FilterToVulkan(Filter filter);
		VkSamplerMipmapMode				SamplerMipmapModeToVulkan(SamplerMipmapMode sampler_mipmap_mode);
		VkSamplerAddressMode			SamplerAddressModeToVulkan(SamplerAddressMode sampler_address_mode);
		VkBorderColor					BorderColourToVulkan(BorderColour border_colour);
		VkAttachmentLoadOp				AttachmentLoadOpToVulkan(AttachmentLoadOp op);
		VkBufferUsageFlags				BufferTypeToVulkanBufferUsageFlags(BufferType type);
		VkPolygonMode					PolygonModeToVulkan(PolygonMode mode);
		VkCullModeFlags					CullModeToVulkan(CullMode cullMode);
		VkFrontFace						FrontFaceToVulkan(FrontFace front_face);
		VkDescriptorType				DescriptorTypeToVulkan(DescriptorType type);
		VkAccessFlags					AccessFlagsToVulkan(AccessFlags flags);
		VkImageLayout					ImageLayoutToVulkan(ImageLayout layout);
		VkImageAspectFlags				ImageAspectFlagsToVulkan(ImageAspectFlags flags);
		VkIndexType						IndexTypeToVulkan(IndexType index_type);
		VkImageType						TextureTypeToVulkan(TextureType type);
		VkImageViewType					TextureViewTypeToVulkan(TextureType type);
		VkDynamicState					DynamicStateToVulkan(DynamicState dynamic_state);
		std::vector<VkDynamicState>		DynamicStatesToVulkan(std::vector<DynamicState> dynamic_states);

		VmaAllocationCreateFlags		BufferTypeToVMAAllocCreateFlags(BufferType type);
		VmaMemoryUsage					BufferTypeToVMAUsage(BufferType type);

		ImageLayout						VkToImageLayout(VkImageLayout layout);
		AttachmentLoadOp				VkToAttachmentLoadOp(VkAttachmentLoadOp op);
		
		VkPresentModeKHR				SwapchainPresentModeToVulkan(SwapchainPresentModes presentMode);

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
			ASSERT(errorCode == VK_SUCCESS);///&& VkErrorToString(errorCode).c_str());
		}
	}
}

#endif ///#if defined(IS_VULKAN_ENABLED)