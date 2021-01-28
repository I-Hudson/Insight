#pragma once

#include "Engine/Core/Compiler.h"
#include "Engine/Core/Log.h"
#include <vulkan/vulkan.hpp>
#include "Engine/Graphics/Enums.h"
#include "Engine/Graphics/PixelFormat.h"
#include "Engine/Graphics/GPUSamplerDescription.h"

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

}