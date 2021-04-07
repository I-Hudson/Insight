#pragma once

#include "Engine/Core/Compiler.h"
#include "Engine/Core/Log.h"

#include "Engine/Core/Maths/Rect.h"

#include <vulkan/vulkan.hpp>
#include "Engine/Graphics/Enums.h"
#include "Engine/Graphics/PixelFormat.h"
#include "Engine/Graphics/GPUSamplerDescription.h"
#include "spirv_cross.hpp"
#include "Engine/Graphics/Shaders/GPUShader.h"
#include "Engine/Graphics/GPUCommandBuffer.h"

extern VkFormat PixelFormatToVkFormat[static_cast<I32>(PixelFormat::MAX)];
extern VkBlendFactor BlendToVkBlendFactor[static_cast<I32>(BlendingMode::Blend::MAX)];
extern VkBlendOp OperationToVkBlendOp[static_cast<I32>(BlendingMode::Operation::MAX)];
extern VkCompareOp ComparisonFuncToVkCompareOp[static_cast<I32>(ComparisonFunc::MAX)];

namespace
{
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

	std::string PhysicalDeviceTypeString(VkPhysicalDeviceType type)
	{
		switch (type)
		{
#define STR(r) case VK_PHYSICAL_DEVICE_TYPE_ ##r: return #r
			STR(OTHER);
			STR(INTEGRATED_GPU);
			STR(DISCRETE_GPU);
			STR(VIRTUAL_GPU);
#undef STR
		default: return "UNKNOWN_DEVICE_TYPE";
		}
	}

	INLINE void ThrowIfFailed(VkResult errorCode)
	{
		IS_CORE_ASSERT(errorCode == VK_SUCCESS, VkErrorToString(errorCode).c_str());
	}


	/// <summary>
	/// Converts Flax Pixel Format to the Vulkan Format.
	/// </summary>
	/// <param name="value">The Flax Pixel Format.</param>
	/// <returns>The Vulkan Format.</returns>
	FORCE_INLINE VkFormat ToVulkanFormat(const PixelFormat value)
	{
		return PixelFormatToVkFormat[(I32)value];
	}

	/// <summary>
	/// Converts Flax blend mode to the Vulkan blend factor.
	/// </summary>
	/// <param name="value">The Flax blend mode.</param>
	/// <returns>The Vulkan blend factor.</returns>
	FORCE_INLINE VkBlendFactor ToVulkanBlendFactor(const BlendingMode::Blend value)
	{
		return BlendToVkBlendFactor[(I32)value];
	}

	/// <summary>
	/// Converts Flax blend operation to the Vulkan blend operation.
	/// </summary>
	/// <param name="value">The Flax blend operation.</param>
	/// <returns>The Vulkan blend operation.</returns>
	FORCE_INLINE VkBlendOp ToVulkanBlendOp(const BlendingMode::Operation value)
	{
		return OperationToVkBlendOp[(I32)value];
	}

	/// <summary>
	/// Converts Flax comparison function to the Vulkan comparison operation.
	/// </summary>
	/// <param name="value">The Flax comparison function.</param>
	/// <returns>The Vulkan comparison operation.</returns>
	FORCE_INLINE VkCompareOp ToVulkanCompareOp(const ComparisonFunc value)
	{
		return ComparisonFuncToVkCompareOp[(I32)value];
	}

	VkSamplerMipmapMode ToVulkanMipFilterMode(GPUSamplerFilter filter)
	{
		VkSamplerMipmapMode result;
		switch (filter)
		{
		case GPUSamplerFilter::Point:
			result = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			break;
		case GPUSamplerFilter::Bilinear:
			result = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			break;
		case GPUSamplerFilter::Trilinear:
			result = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			break;
		case GPUSamplerFilter::Anisotropic:
			result = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			break;
		default:
			break;
		}
		return result;
	}

	VkFilter ToVulkanMagFilterMode(GPUSamplerFilter filter)
	{
		VkFilter result;
		switch (filter)
		{
		case GPUSamplerFilter::Point:
			result = VK_FILTER_NEAREST;
			break;
		case GPUSamplerFilter::Bilinear:
			result = VK_FILTER_LINEAR;
			break;
		case GPUSamplerFilter::Trilinear:
			result = VK_FILTER_LINEAR;
			break;
		case GPUSamplerFilter::Anisotropic:
			result = VK_FILTER_LINEAR;
			break;
		default:
			break;
		}
		return result;
	}

	VkFilter ToVulkanMinFilterMode(GPUSamplerFilter filter)
	{
		VkFilter result;
		switch (filter)
		{
		case GPUSamplerFilter::Point:
			result = VK_FILTER_NEAREST;
			break;
		case GPUSamplerFilter::Bilinear:
			result = VK_FILTER_LINEAR;
			break;
		case GPUSamplerFilter::Trilinear:
			result = VK_FILTER_LINEAR;
			break;
		case GPUSamplerFilter::Anisotropic:
			result = VK_FILTER_LINEAR;
			break;
		default:
			break;
		}
		return result;
	}

	VkSamplerAddressMode ToVulkanWrapMode(GPUSamplerAddressMode addressMode, const bool supportsMirrorClampToEdge)
	{
		VkSamplerAddressMode result;
		switch (addressMode)
		{
		case GPUSamplerAddressMode::Wrap:
			result = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			break;
		case GPUSamplerAddressMode::Clamp:
			result = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			break;
		case GPUSamplerAddressMode::Mirror:
			result = supportsMirrorClampToEdge ? VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE : VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			break;
		case GPUSamplerAddressMode::Border:
			result = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			break;
		default:
			break;
		}
		return result;
	}

	VkCompareOp ToVulkanSamplerCompareFunction(GPUSamplerCompareFunction samplerComparisonFunction)
	{
		VkCompareOp result;
		switch (samplerComparisonFunction)
		{
		case GPUSamplerCompareFunction::Less:
			result = VK_COMPARE_OP_LESS;
			break;
		case GPUSamplerCompareFunction::Never:
			result = VK_COMPARE_OP_NEVER;
			break;
		default:
			break;
		}
		return result;
	}

	VkFormat ToVulkanFormatFromSPRIV(const spirv_cross::SPIRType& type, const U32& vecSize)
	{
		switch (type.basetype)
		{
		case spirv_cross::SPIRType::Int:
			switch (vecSize)
			{
			case 1: return VK_FORMAT_R32_SINT;
			case 2: return VK_FORMAT_R32G32_SINT;
			case 3: return VK_FORMAT_R32G32B32_SINT;
			case 4: return VK_FORMAT_R32G32B32A32_SINT;
			}
			break;
		case spirv_cross::SPIRType::Float:
			switch (vecSize)
			{
			case 1: return VK_FORMAT_R32_SFLOAT;
			case 2: return VK_FORMAT_R32G32_SFLOAT;
			case 3: return VK_FORMAT_R32G32B32_SFLOAT;
			case 4: return VK_FORMAT_R32G32B32A32_SFLOAT;
			}
			break;

		}
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	}

	VkShaderStageFlagBits ToVulkanShaderStageFlags(const ShaderStage& shaderStage)
	{
		switch (shaderStage)
		{
		case ShaderStage::Vertex: return VK_SHADER_STAGE_VERTEX_BIT;
		case ShaderStage::TessControl: return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		case ShaderStage::TessEvaluation: return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		case ShaderStage::Geometry: return VK_SHADER_STAGE_GEOMETRY_BIT;
		case ShaderStage::Fragment: return VK_SHADER_STAGE_FRAGMENT_BIT;
		case ShaderStage::Compute: return VK_SHADER_STAGE_COMPUTE_BIT;
		}
		return VK_SHADER_STAGE_ALL;
	}

	VkImageType ToVulkanImageType(const ImageType& imageType)
	{
		switch (imageType)
		{
		case ImageType::Image_1D: return VK_IMAGE_TYPE_1D;
		case ImageType::Image_2D: return VK_IMAGE_TYPE_2D;
		case ImageType::Image_3D: return VK_IMAGE_TYPE_3D;
		}
		return VK_IMAGE_TYPE_2D;
	}

	VkImageViewType ToVulkanImageViewType(const ImageType& imageType)
	{
		switch (imageType)
		{
		case ImageType::Image_1D: return VK_IMAGE_VIEW_TYPE_1D;
		case ImageType::Image_2D: return VK_IMAGE_VIEW_TYPE_2D;
		case ImageType::Image_3D: return VK_IMAGE_VIEW_TYPE_3D;
		}
		return VK_IMAGE_VIEW_TYPE_2D;
	}

	VkSampleCountFlagBits ToVulkanSampleCountBits(const U32 sample)
	{
		if (sample & VK_SAMPLE_COUNT_64_BIT)
		{
			return VK_SAMPLE_COUNT_64_BIT;
		}
		if (sample & VK_SAMPLE_COUNT_32_BIT)
		{
			return VK_SAMPLE_COUNT_32_BIT;
		}
		if (sample & VK_SAMPLE_COUNT_16_BIT)
		{
			return VK_SAMPLE_COUNT_16_BIT;
		}
		if (sample & VK_SAMPLE_COUNT_8_BIT)
		{
			return VK_SAMPLE_COUNT_8_BIT;
		}
		if (sample & VK_SAMPLE_COUNT_4_BIT)
		{
			return VK_SAMPLE_COUNT_4_BIT;
		}
		if (sample & VK_SAMPLE_COUNT_2_BIT)
		{
			return VK_SAMPLE_COUNT_2_BIT;
		}
		return VK_SAMPLE_COUNT_1_BIT;
	}

	VkImageUsageFlags ToVulkanImageUsage(const ImageUsageFlags imageUsageFlags)
	{
		VkImageUsageFlags flags = 0;
		if (imageUsageFlags & (u32)ImageUsageFlagsBits::Transfer_Src)				{ flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT; }
		if (imageUsageFlags & (u32)ImageUsageFlagsBits::Transfer_Dst)				{ flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT; }
		if (imageUsageFlags & (u32)ImageUsageFlagsBits::Sampled)					{ flags |= VK_IMAGE_USAGE_SAMPLED_BIT; }
		if (imageUsageFlags & (u32)ImageUsageFlagsBits::Storage)					{ flags |= VK_IMAGE_USAGE_STORAGE_BIT; }
		if (imageUsageFlags & (u32)ImageUsageFlagsBits::Color_Attachment)			{ flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; }
		if (imageUsageFlags & (u32)ImageUsageFlagsBits::Depth_Stencil_Attachment)	{ flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT; }
		if (imageUsageFlags & (u32)ImageUsageFlagsBits::Transient_Attachment)		{ flags |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT; }
		if (imageUsageFlags & (u32)ImageUsageFlagsBits::Input_Attachment)			{ flags |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT; }
	
		return flags;
	}

	VkImageLayout ToVulkanImageLayout(const ImageLayout& imageLayout)
	{
		switch (imageLayout)
		{
		case ImageLayout::Undefined: return VK_IMAGE_LAYOUT_UNDEFINED;
		case ImageLayout::General: return VK_IMAGE_LAYOUT_GENERAL;
		case ImageLayout::Color_Attachment: return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		case ImageLayout::Depth_Stencil_Attachment: return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		case ImageLayout::Depth_Stencil_Read_Only: return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
		case ImageLayout::Shader_Read_Only: return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		case ImageLayout::Transfer_Src: return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		case ImageLayout::Transfer_Dst: return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		case ImageLayout::Preinitialized: return VK_IMAGE_LAYOUT_PREINITIALIZED;
		}
		return VK_IMAGE_LAYOUT_UNDEFINED;
	}

	VkImageCreateFlags ToVulkanImageCreateFlags(const ImageCreateFlags& createFlags)
	{
		VkImageCreateFlags flags = 0;
		if (createFlags & (u32)ImageCreate::Sparse_Binding)		{ flags |= VK_IMAGE_CREATE_SPARSE_BINDING_BIT; }
		if (createFlags & (u32)ImageCreate::Sparse_Residency)	{ flags |= VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT; }
		if (createFlags & (u32)ImageCreate::Sparse_Aliased)		{ flags |= VK_IMAGE_CREATE_SPARSE_ALIASED_BIT; }
		if (createFlags & (u32)ImageCreate::Mutable_Format)		{ flags |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT; }
		if (createFlags & (u32)ImageCreate::Cube_Compatible)	{ flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT; }
		return flags;
	}
	
	VkCommandBufferResetFlags ToVulkanCommandBufferUsageFlags(Insight::Graphics::GPUCommandBufferUsageFlags const& flags)
	{
		VkCommandBufferResetFlags vFlags = 0;
		if (flags == Insight::Graphics::GPUCommandBufferUsageFlags::ONE_TIME_SUBMIT) { vFlags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; }
		else if (flags == Insight::Graphics::GPUCommandBufferUsageFlags::RENDER_PASS_CONTINUE) { vFlags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT; }
		else if (flags == Insight::Graphics::GPUCommandBufferUsageFlags::SIMULATANEOUS_USE) { vFlags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT; }
		return vFlags;
	}

	VkCommandPoolCreateFlags ToVulkanCommandPoolUsageFlgas(Insight::Graphics::GPUCommandPoolFlags const& flags)
	{
		VkCommandPoolCreateFlags vFlags = 0;
		if (flags == Insight::Graphics::GPUCommandPoolFlags::TRANSIENT) { vFlags |= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT; }
		else if (flags == Insight::Graphics::GPUCommandPoolFlags::RESET_COMMAND_BUFFER) { vFlags |= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; }
		else if (flags == Insight::Graphics::GPUCommandPoolFlags::PROTECTED) { vFlags |= VK_COMMAND_POOL_CREATE_PROTECTED_BIT; }
		return vFlags;
	}

	VkViewport ToVulkanViewPort(Insight::Maths::Rect const& rect)
	{
		VkViewport viewport;
		viewport.width = rect.GetX();
		viewport.height = rect.GetY();
		viewport.minDepth = rect.GetWidth();
		viewport.maxDepth = rect.GetHeight();
		return viewport;
	}
}