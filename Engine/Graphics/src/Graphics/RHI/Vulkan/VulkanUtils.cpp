#if defined(IS_VULKAN_ENABLED)

#include "Graphics/RHI/Vulkan/VulkanUtils.h"

#ifdef IS_PLATFORM_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             /// Exclude rarely-used stuff from Windows headers.
#endif
#include <Windows.h>
#include "vulkan/vulkan_win32.h"
#endif

#include "VmaUsage.h"

VkFormat PixelFormatToVkFormat[static_cast<uint32_t>(PixelFormat::MAX)] = {

    VK_FORMAT_UNDEFINED,
    VK_FORMAT_R32G32B32A32_SFLOAT,
    VK_FORMAT_R32G32B32A32_SFLOAT,
    VK_FORMAT_R32G32B32A32_UINT,
    VK_FORMAT_R32G32B32A32_SINT,
    VK_FORMAT_R32G32B32_SFLOAT,
    VK_FORMAT_R32G32B32_SFLOAT,
    VK_FORMAT_R32G32B32_UINT,
    VK_FORMAT_R32G32B32_SINT,
    VK_FORMAT_R16G16B16A16_SFLOAT,
    VK_FORMAT_R16G16B16A16_SFLOAT,
    VK_FORMAT_R16G16B16A16_UNORM,
    VK_FORMAT_R16G16B16A16_UINT,
    VK_FORMAT_R16G16B16A16_SNORM,
    VK_FORMAT_R16G16B16A16_SINT,
    VK_FORMAT_R32G32_SFLOAT,
    VK_FORMAT_R32G32_SFLOAT,
    VK_FORMAT_R32G32_UINT,
    VK_FORMAT_R32G32_SINT,
    VK_FORMAT_UNDEFINED,
    // TODO: R32G8X24_Typeless
    VK_FORMAT_D32_SFLOAT_S8_UINT,
    VK_FORMAT_UNDEFINED,
    // TODO: R32_Float_X8X24_Typeless
    VK_FORMAT_UNDEFINED,
    // TODO: X32_Typeless_G8X24_UInt
    VK_FORMAT_A2B10G10R10_UNORM_PACK32,
    VK_FORMAT_A2B10G10R10_UNORM_PACK32,
    VK_FORMAT_A2B10G10R10_UINT_PACK32,
    VK_FORMAT_B10G11R11_UFLOAT_PACK32,
    VK_FORMAT_R8G8B8A8_UNORM,
    VK_FORMAT_R8G8B8A8_UNORM,
    VK_FORMAT_R8G8B8A8_SRGB,
    VK_FORMAT_R8G8B8A8_UINT,
    VK_FORMAT_R8G8B8A8_SNORM,
    VK_FORMAT_R8G8B8A8_SINT,
    VK_FORMAT_R16G16_SFLOAT,
    VK_FORMAT_R16G16_SFLOAT,
    VK_FORMAT_R16G16_UNORM,
    VK_FORMAT_R16G16_UINT,
    VK_FORMAT_R16G16_SNORM,
    VK_FORMAT_R16G16_SINT,
    VK_FORMAT_R32_SFLOAT,
    VK_FORMAT_D32_SFLOAT,
    VK_FORMAT_R32_SFLOAT,
    VK_FORMAT_R32_UINT,
    VK_FORMAT_R32_SINT,
    VK_FORMAT_D24_UNORM_S8_UINT,
    VK_FORMAT_D24_UNORM_S8_UINT,
    VK_FORMAT_X8_D24_UNORM_PACK32,
    VK_FORMAT_D24_UNORM_S8_UINT,
    VK_FORMAT_R8G8_UNORM,
    VK_FORMAT_R8G8_UNORM,
    VK_FORMAT_R8G8_UINT,
    VK_FORMAT_R8G8_SNORM,
    VK_FORMAT_R8G8_SINT,
    VK_FORMAT_R16_SFLOAT,
    VK_FORMAT_R16_SFLOAT,
    VK_FORMAT_D16_UNORM,
    VK_FORMAT_R16_UNORM,
    VK_FORMAT_R16_UINT,
    VK_FORMAT_R16_SNORM,
    VK_FORMAT_R16_SINT,
    VK_FORMAT_R8_UNORM,
    VK_FORMAT_R8_UNORM,
    VK_FORMAT_R8_UINT,
    VK_FORMAT_R8_SNORM,
    VK_FORMAT_R8_SINT,
    VK_FORMAT_UNDEFINED,
    // TODO: A8_UNorm
    VK_FORMAT_UNDEFINED,
    // TODO: R1_UNorm
    VK_FORMAT_E5B9G9R9_UFLOAT_PACK32,
    VK_FORMAT_UNDEFINED,
    // TODO: R8G8_B8G8_UNorm
    VK_FORMAT_UNDEFINED,
    // TODO: G8R8_G8B8_UNorm
    VK_FORMAT_BC1_RGBA_UNORM_BLOCK,
    VK_FORMAT_BC1_RGBA_UNORM_BLOCK,
    VK_FORMAT_BC1_RGBA_SRGB_BLOCK,
    VK_FORMAT_BC2_UNORM_BLOCK,
    VK_FORMAT_BC2_UNORM_BLOCK,
    VK_FORMAT_BC2_SRGB_BLOCK,
    VK_FORMAT_BC3_UNORM_BLOCK,
    VK_FORMAT_BC3_UNORM_BLOCK,
    VK_FORMAT_BC3_SRGB_BLOCK,
    VK_FORMAT_BC4_UNORM_BLOCK,
    VK_FORMAT_BC4_UNORM_BLOCK,
    VK_FORMAT_BC4_SNORM_BLOCK,
    VK_FORMAT_BC5_UNORM_BLOCK,
    VK_FORMAT_BC5_UNORM_BLOCK,
    VK_FORMAT_BC5_SNORM_BLOCK,
    VK_FORMAT_B5G6R5_UNORM_PACK16,
    VK_FORMAT_B5G5R5A1_UNORM_PACK16,
    VK_FORMAT_B8G8R8A8_UNORM,
    VK_FORMAT_B8G8R8A8_UNORM,
    VK_FORMAT_UNDEFINED,
    // TODO: R10G10B10_Xr_Bias_A2_UNorm
    VK_FORMAT_B8G8R8A8_UNORM,
    VK_FORMAT_B8G8R8A8_SRGB,
    VK_FORMAT_B8G8R8A8_UNORM,
    VK_FORMAT_B8G8R8A8_SRGB,
    VK_FORMAT_BC6H_UFLOAT_BLOCK,
    VK_FORMAT_BC6H_UFLOAT_BLOCK,
    VK_FORMAT_BC6H_SFLOAT_BLOCK,
    VK_FORMAT_BC7_UNORM_BLOCK,
    VK_FORMAT_BC7_UNORM_BLOCK,
    VK_FORMAT_BC7_SRGB_BLOCK,
};

PixelFormat VkFormatToPixelFormat[static_cast<int>(PixelFormat::MAX)] =
{

    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::R8_UNorm,
    /// 10
    PixelFormat::R8_SNorm,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::R8_UInt,
    PixelFormat::R8_SInt,
    PixelFormat::Unknown,
    PixelFormat::R8G8_UNorm,
    PixelFormat::R8G8_SNorm,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    /// 20
    PixelFormat::R8G8_UInt,
    PixelFormat::R8G8_SInt,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    /// 30
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::R8G8B8A8_UNorm,
    PixelFormat::R8G8B8A8_SNorm,
    PixelFormat::Unknown,
    /// 40
    PixelFormat::Unknown,
    PixelFormat::R8G8B8A8_UInt,
    PixelFormat::R8G8B8A8_SInt,
    PixelFormat::Unknown,
    PixelFormat::B8G8R8A8_UNorm,
    PixelFormat::R8G8B8A8_SNorm,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    ///50
    PixelFormat::B8G8R8A8_UNorm_sRGB,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::R16_UNorm,
    PixelFormat::R16_SNorm,
    ///60
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::R16_UInt,
    PixelFormat::R16_SInt,
    PixelFormat::Unknown,
    PixelFormat::R16G16_UNorm,
    PixelFormat::R16G16_UNorm,
    PixelFormat::R16G16_SNorm,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    /// 70
    PixelFormat::R16G16_UInt,
    PixelFormat::R16G16_SInt,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    /// 80
    PixelFormat::Unknown,
    PixelFormat::R16G16B16A16_UNorm,
    PixelFormat::R16G16B16A16_SNorm,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::R16G16B16A16_UInt,
    PixelFormat::R16G16B16A16_SInt,
    PixelFormat::Unknown,
    PixelFormat::R32_UInt,
    PixelFormat::R32_SInt,
    /// 90
    PixelFormat::Unknown,
    PixelFormat::R32G32_UInt,
    PixelFormat::R32G32_SInt,
    PixelFormat::Unknown,
    PixelFormat::R32G32B32_UInt,
    PixelFormat::R32G32B32_SInt,
    PixelFormat::Unknown,
    PixelFormat::R32G32B32A32_UInt,
    PixelFormat::R32G32B32A32_SInt,
    PixelFormat::Unknown,
};

/*
vk::Format PixelFormatToVkFormat[static_cast<int>(PixelFormat::MAX)] =
{
    vk::Format::eUndefined,
    vk::Format::eR32G32B32A32Sfloat,
    vk::Format::eR32G32B32A32Sfloat,
    vk::Format::eR32G32B32A32Uint,
    vk::Format::eR32G32B32A32Sint,

    vk::Format::eR32G32B32Sfloat,
    vk::Format::eR32G32B32Sfloat,
    vk::Format::eR32G32B32Uint,
    vk::Format::eR32G32B32Sint,

    vk::Format::eR16G16B16A16Sfloat,
    vk::Format::eR16G16B16A16Sfloat,
    vk::Format::eR16G16B16A16Unorm,
    vk::Format::eR16G16B16A16Uint,
    vk::Format::eR16G16B16A16Snorm,
    vk::Format::eR16G16B16A16Sint,

    vk::Format::eR32G32Sfloat,
    vk::Format::eR32G32Sfloat,
    vk::Format::eR32G32Uint,
    vk::Format::eR32G32Sint,

    vk::Format::eUndefined, /// TODO: R32G8X24_Typeless
    vk::Format::eD32SfloatS8Uint,
    vk::Format::eUndefined,  /// TODO: R32_Float_X8X24_Typeless
    vk::Format::eUndefined, /// TODO: X32_Typeless_G8X24_UInt

    vk::Format::eA2R10G10B10UnormPack32,
    vk::Format::eA2R10G10B10UnormPack32,
    vk::Format::eA2R10G10B10UintPack32,

    vk::Format::eB10G11R11UfloatPack32,

    vk::Format::eR8G8B8A8Unorm,
    vk::Format::eR8G8B8A8Unorm,
    vk::Format::eR8G8B8A8Srgb,
    vk::Format::eR8G8B8A8Uint,
    vk::Format::eR8G8B8A8Snorm,
    vk::Format::eR8G8B8A8Sint,

    vk::Format::eR16G16Sfloat,
    vk::Format::eR16G16Sfloat,
    vk::Format::eR16G16Unorm,
    vk::Format::eR16G16Uint,
    vk::Format::eR16G16Snorm,
    vk::Format::eR16G16Sint,

    vk::Format::eD32Sfloat,

    vk::Format::eR32Sfloat,
    vk::Format::eR32Sfloat,
    vk::Format::eR32Uint,
    vk::Format::eR32Sint,

    vk::Format::eD24UnormS8Uint,
    vk::Format::eD24UnormS8Uint,

    vk::Format::eX8D24UnormPack32,
    vk::Format::eD24UnormS8Uint,

    vk::Format::eR8G8Unorm,
    vk::Format::eR8G8Unorm,
    vk::Format::eR8G8Uint,
    vk::Format::eR8G8Snorm,
    vk::Format::eR8G8Sint,

    vk::Format::eD16Unorm,

    vk::Format::eR16Sfloat,
    vk::Format::eR16Sfloat,
    vk::Format::eR16Unorm,
    vk::Format::eR16Uint,
    vk::Format::eR16Snorm,
    vk::Format::eR16Sint,

    vk::Format::eR8Unorm,
    vk::Format::eR8Unorm,
    vk::Format::eR8Uint,
    vk::Format::eR8Snorm,
    vk::Format::eR8Sint,
    
    vk::Format::eUndefined, /// TODO: A8_UNorm
    vk::Format::eUndefined, /// TODO: R1_UNorm

    vk::Format::eE5B9G9R9UfloatPack32,

    vk::Format::eUndefined, /// TODO: R8G8_B8G8_UNorm
    vk::Format::eUndefined, /// TODO: G8R8_G8B8_UNorm

    vk::Format::eBc1RgbaUnormBlock,
    vk::Format::eBc1RgbaUnormBlock,
    vk::Format::eBc1RgbaSrgbBlock,

    vk::Format::eBc2UnormBlock,
    vk::Format::eBc2UnormBlock,
    vk::Format::eBc2SrgbBlock,

    vk::Format::eBc3UnormBlock,
    vk::Format::eBc3UnormBlock,
    vk::Format::eBc3SrgbBlock,

    vk::Format::eBc4UnormBlock,
    vk::Format::eBc4UnormBlock,
    vk::Format::eBc4SnormBlock,

    vk::Format::eBc5UnormBlock,
    vk::Format::eBc5UnormBlock,
    vk::Format::eBc5SnormBlock,

    vk::Format::eB5G6R5UnormPack16,
    vk::Format::eB5G5R5A1UnormPack16,

    vk::Format::eB8G8R8A8Unorm,
    vk::Format::eB8G8R8A8Unorm,
    vk::Format::eUndefined, /// TODO: R10G10B10_Xr_Bias_A2_UNorm

    vk::Format::eB8G8R8A8Unorm,
    vk::Format::eB8G8R8A8Srgb,
    vk::Format::eB8G8R8A8Unorm,
    vk::Format::eB8G8R8A8Srgb,

   vk::Format::eBc6HUfloatBlock,
   vk::Format::eBc6HUfloatBlock,
   vk::Format::eBc6HSfloatBlock,

   vk::Format::eBc7UnormBlock,
   vk::Format::eBc7UnormBlock,
   vk::Format::eBc7SrgbBlock,
};

PixelFormat VkFormatToPixelFormat[static_cast<int>(PixelFormat::MAX)] =
{

    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::R8_UNorm,
    /// 10
    PixelFormat::R8_SNorm,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::R8_UInt,
    PixelFormat::R8_SInt,
    PixelFormat::Unknown,
    PixelFormat::R8G8_UNorm,
    PixelFormat::R8G8_SNorm,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    /// 20
    PixelFormat::R8G8_UInt,
    PixelFormat::R8G8_SInt,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    /// 30
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::R8G8B8A8_UNorm,
    PixelFormat::R8G8B8A8_SNorm,
    PixelFormat::Unknown,
    /// 40
    PixelFormat::Unknown,
    PixelFormat::R8G8B8A8_UInt,
    PixelFormat::R8G8B8A8_SInt,
    PixelFormat::Unknown,
    PixelFormat::B8G8R8A8_UNorm,
    PixelFormat::R8G8B8A8_SNorm,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::B8G8R8A8_UNorm_sRGB,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::R16_UNorm,
    PixelFormat::R16_SNorm,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::R16_UInt,
    PixelFormat::R16_SInt,
    PixelFormat::Unknown,
    PixelFormat::R16G16_UNorm,
    PixelFormat::R16G16_UNorm,
    PixelFormat::R16G16_SNorm,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::R16G16_UInt,
    PixelFormat::R16G16_SInt,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::R16G16B16A16_UNorm,
    PixelFormat::R16G16B16A16_SNorm,
    PixelFormat::Unknown,
    PixelFormat::Unknown,
    PixelFormat::R16G16B16A16_UInt,
    PixelFormat::R16G16B16A16_SInt,
    PixelFormat::Unknown,
    PixelFormat::R32_UInt,
    PixelFormat::R32_SInt,
    PixelFormat::Unknown,
    PixelFormat::R32G32_UInt,
    PixelFormat::R32G32_SInt,
    PixelFormat::Unknown,
    PixelFormat::R32G32B32_UInt,
    PixelFormat::R32G32B32_SInt,
    PixelFormat::Unknown,
    PixelFormat::R32G32B32A32_UInt,
    PixelFormat::R32G32B32A32_SInt,
    PixelFormat::Unknown,
};
*/

namespace Insight
{
    namespace Graphics
    {
#define COMPARE_AND_SET_BIT(value, bitToCompare, result) if (value & bitToCompare) { result |= bitToCompare; }

        std::string DeviceExtensionToVulkan(DeviceExtension extension)
        {
            switch (extension)
            {
            case Insight::Graphics::DeviceExtension::BindlessDescriptors: return VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME;
            case Insight::Graphics::DeviceExtension::ExclusiveFullScreen: return VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME;
            case Insight::Graphics::DeviceExtension::DeviceExtensionCount: return "";
            default:
                break;
            }
            return "";
        }

        VkImageUsageFlags ImageUsageFlagsToVulkan(ImageUsageFlags imageUsageFlags)
        {
            VkImageUsageFlags flags = {};
            if (imageUsageFlags & ImageUsageFlagsBits::TransferSrc)            { flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT; }
            if (imageUsageFlags & ImageUsageFlagsBits::TransferDst)            { flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT; }
            if (imageUsageFlags & ImageUsageFlagsBits::Sampled)                { flags |= VK_IMAGE_USAGE_SAMPLED_BIT; }
            if (imageUsageFlags & ImageUsageFlagsBits::Storage)                { flags |= VK_IMAGE_USAGE_STORAGE_BIT; }
            if (imageUsageFlags & ImageUsageFlagsBits::ColourAttachment)       { flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; }
            if (imageUsageFlags & ImageUsageFlagsBits::DepthStencilAttachment) { flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT; }
            if (imageUsageFlags & ImageUsageFlagsBits::TransientAttachment)    { flags |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT; }
            if (imageUsageFlags & ImageUsageFlagsBits::InputAttachment)        { flags |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT; }
            return flags;
        }

        VkFormat PixelFormatToVulkan(PixelFormat format)
        {
            return PixelFormatToVkFormat[(int)format];
        }

        VkPipelineBindPoint GPUQueueToVulkanBindPoint(GPUQueue queue)
        {
            switch (queue)
            {
            case GPUQueue_Graphics: return VK_PIPELINE_BIND_POINT_GRAPHICS;
            case GPUQueue_Compute: return VK_PIPELINE_BIND_POINT_COMPUTE;
            default:
                break;
            }
            return VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS;
        }

        VkPipelineStageFlags PipelineStageFlagsToVulkan(PipelineStageFlags flags)
        {
            VkPipelineStageFlags result = {};

            if (flags & +PipelineStageFlagBits::TopOfPipe)                       { result |= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT; }
            if (flags & +PipelineStageFlagBits::DrawIndirect)                    { result |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT; }
            if (flags & +PipelineStageFlagBits::VertexInput)                     { result |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT; }
            if (flags & +PipelineStageFlagBits::VertexShader)                    { result |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT; }
            if (flags & +PipelineStageFlagBits::TessesllationControlShader)      { result |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT; }
            if (flags & +PipelineStageFlagBits::TessesllationEvaluationShader)   { result |= VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT; }
            if (flags & +PipelineStageFlagBits::GeometryShader)                  { result |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT; }
            if (flags & +PipelineStageFlagBits::FragmentShader)                  { result |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; }
            if (flags & +PipelineStageFlagBits::EarlyFramgmentShader)            { result |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT; }
            if (flags & +PipelineStageFlagBits::LateFramgmentShader)             { result |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT; }
            if (flags & +PipelineStageFlagBits::ColourAttachmentOutput)          { result |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; }
            if (flags & +PipelineStageFlagBits::ComputeShader)                   { result |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT; }
            if (flags & +PipelineStageFlagBits::Transfer)                        { result |= VK_PIPELINE_STAGE_TRANSFER_BIT; }
            if (flags & +PipelineStageFlagBits::BottomOfPipe)                    { result |= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT; }
            if (flags & +PipelineStageFlagBits::Host)                            { result |= VK_PIPELINE_STAGE_HOST_BIT; }
            if (flags & +PipelineStageFlagBits::AllGraphics)                     { result |= VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT; }
            if (flags & +PipelineStageFlagBits::AllCommands)                     { result |= VK_PIPELINE_STAGE_ALL_COMMANDS_BIT; }

            return result;
        }

        VkShaderStageFlagBits ShaderStageFlagBitsToVulkan(ShaderStageFlagBits stage)
        {
            switch (stage)
            {
            case ShaderStage_Vertex:      return VK_SHADER_STAGE_VERTEX_BIT;
            case ShaderStage_TessControl: return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
            case ShaderStage_TessEval:    return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
            case ShaderStage_Geometry:    return VK_SHADER_STAGE_GEOMETRY_BIT;
            case ShaderStage_Pixel:       return VK_SHADER_STAGE_FRAGMENT_BIT;
            default:
                break;
            }
            return VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;
        }

        VkShaderStageFlags ShaderStageFlagsToVulkan(ShaderStageFlags flags)
        {
            VkShaderStageFlags result = {};
            for (size_t i = 0; i < ShaderStageCount; ++i)
            {
                ShaderStageFlagBits stageBit = static_cast<ShaderStageFlagBits>(1 << i);
                if (flags & stageBit)
                {
                    result |= ShaderStageFlagBitsToVulkan(stageBit);
                }
            }
            return result;
        }

        VkPrimitiveTopology PrimitiveTopologyTypeToVulkan(PrimitiveTopologyType type)
        {
            switch (type)
            {
            case PrimitiveTopologyType::PointList:                  return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
            case PrimitiveTopologyType::LineList:                   return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
            case PrimitiveTopologyType::LineStrip:                  return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
            case PrimitiveTopologyType::TriangleList:               return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            case PrimitiveTopologyType::TriangleStrip:              return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
            case PrimitiveTopologyType::TriangleFan:                return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
            case PrimitiveTopologyType::LineListWithAdjacency:      return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
            case PrimitiveTopologyType::LineStripWithAdjacency:     return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
            case PrimitiveTopologyType::TriangleListWithAdjacency:  return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
            case PrimitiveTopologyType::TriangleStripWithAdjacency: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
            case PrimitiveTopologyType::PatchList:                  return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
            default:
                break;
            }
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        }

        VkColorComponentFlags ColourComponentFlagsToVulkan(ColourComponentFlags flags)
        {
            VkColorComponentFlags flag = {};
            if (flags & ColourComponentR) { flag |= VK_COLOR_COMPONENT_R_BIT; }
            if (flags & ColourComponentG) { flag |= VK_COLOR_COMPONENT_G_BIT; }
            if (flags & ColourComponentB) { flag |= VK_COLOR_COMPONENT_B_BIT; }
            if (flags & ColourComponentA) { flag |= VK_COLOR_COMPONENT_A_BIT; }
            return flag;
        }

        VkBlendFactor BlendFactorToVulkan(BlendFactor factor)
        {
            switch (factor)
            {
            case Insight::Graphics::BlendFactor::Zero:                   return VK_BLEND_FACTOR_ZERO;
            case Insight::Graphics::BlendFactor::One:                    return VK_BLEND_FACTOR_ONE;
            case Insight::Graphics::BlendFactor::SrcColour:              return VK_BLEND_FACTOR_SRC_COLOR;
            case Insight::Graphics::BlendFactor::OneMinusSrcColour:      return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
            case Insight::Graphics::BlendFactor::DstColour:              return VK_BLEND_FACTOR_DST_COLOR;
            case Insight::Graphics::BlendFactor::OneMinusDstColour:      return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
            case Insight::Graphics::BlendFactor::SrcAlpha:               return VK_BLEND_FACTOR_SRC_ALPHA;
            case Insight::Graphics::BlendFactor::OneMinusSrcAlpha:       return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            case Insight::Graphics::BlendFactor::DstAlpha:               return VK_BLEND_FACTOR_DST_ALPHA;
            case Insight::Graphics::BlendFactor::OneMinusDstAlpha:       return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
            case Insight::Graphics::BlendFactor::ConstantColour:         return VK_BLEND_FACTOR_CONSTANT_COLOR;
            case Insight::Graphics::BlendFactor::OneMinusConstantColour: return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
            case Insight::Graphics::BlendFactor::ConstantAlpha:          return VK_BLEND_FACTOR_CONSTANT_ALPHA;
            case Insight::Graphics::BlendFactor::OneMinusConstantAlpha:  return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
            case Insight::Graphics::BlendFactor::SrcAplhaSaturate:       return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
            case Insight::Graphics::BlendFactor::Src1Colour:             return VK_BLEND_FACTOR_SRC1_COLOR;
            case Insight::Graphics::BlendFactor::OneMinusSrc1Colour:     return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
            case Insight::Graphics::BlendFactor::Src1Alpha:              return VK_BLEND_FACTOR_SRC1_ALPHA;
            case Insight::Graphics::BlendFactor::OneMinusSrc1Alpha:      return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
            default:
                break;
            }
            return VK_BLEND_FACTOR_ZERO;
        }

        VkBlendOp BlendOpToVulkan(BlendOp op)
        {
            switch (op)
            {
            case Insight::Graphics::BlendOp::Add:             return VK_BLEND_OP_ADD;
            case Insight::Graphics::BlendOp::Subtract:        return VK_BLEND_OP_SUBTRACT;
            case Insight::Graphics::BlendOp::ReverseSubtract: return VK_BLEND_OP_REVERSE_SUBTRACT;
            case Insight::Graphics::BlendOp::Min:             return VK_BLEND_OP_MIN;
            case Insight::Graphics::BlendOp::Max:             return VK_BLEND_OP_MAX;
            default:
                break;
            }
            return VK_BLEND_OP_ADD;
        }

        VkCompareOp CompareOpToVulkan(CompareOp op)
        {
            switch (op)
            {
            case CompareOp::Never:          return VK_COMPARE_OP_NEVER;
            case CompareOp::Less:           return VK_COMPARE_OP_LESS;
            case CompareOp::Equal:          return VK_COMPARE_OP_EQUAL;
            case CompareOp::LessOrEqual:    return VK_COMPARE_OP_LESS_OR_EQUAL;
            case CompareOp::Greater:        return VK_COMPARE_OP_GREATER;
            case CompareOp::NotEqual:       return VK_COMPARE_OP_NOT_EQUAL;
            case CompareOp::GreaterOrEqual: return VK_COMPARE_OP_GREATER_OR_EQUAL;
            case CompareOp::Always:         return VK_COMPARE_OP_ALWAYS;
            default:
                break;
            }
            return VK_COMPARE_OP_NEVER;
        }

        VkFilter FilterToVulkan(Filter filter)
        {
            switch (filter)
            {
            case Filter::Nearest:    return VK_FILTER_NEAREST;
            case Filter::Linear:     return VK_FILTER_LINEAR;
            default:
                break;
            }
            assert(false);
            return VK_FILTER_NEAREST;
        }

        VkSamplerMipmapMode SamplerMipmapModeToVulkan(SamplerMipmapMode sampler_mipmap_mode)
        {
            switch (sampler_mipmap_mode)
            {
            case SamplerMipmapMode::Nearest:    return VK_SAMPLER_MIPMAP_MODE_NEAREST;
            case SamplerMipmapMode::Linear:     return VK_SAMPLER_MIPMAP_MODE_LINEAR;
            default:
                break;
            }
            assert(false);
            return VK_SAMPLER_MIPMAP_MODE_NEAREST;
        }

        VkSamplerAddressMode SamplerAddressModeToVulkan(SamplerAddressMode sampler_address_mode)
        {
            switch (sampler_address_mode)
            {
            case SamplerAddressMode::Repeat:                return VK_SAMPLER_ADDRESS_MODE_REPEAT;
            case SamplerAddressMode::MirroredRepeat:        return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
            case SamplerAddressMode::ClampToEdge:           return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            case SamplerAddressMode::ClampToBoarder:        return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
            case SamplerAddressMode::MirrorClampToEdge:     return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
            default:
                break;
            }
            assert(false);
            return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        }

        VkBorderColor BorderColourToVulkan(BorderColour border_colour)
        {
            switch (border_colour)
            {
            case BorderColour::FloatTransparentBlack:    return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
            case BorderColour::IntTransparentBlack:      return VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
            case BorderColour::FloatOpaqueBlack:         return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
            case BorderColour::IntOpaqueBlack:           return VK_BORDER_COLOR_INT_OPAQUE_BLACK;
            case BorderColour::FloatOpaqueWhite:         return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
            case BorderColour::IntOpaqueWhite:           return VK_BORDER_COLOR_INT_OPAQUE_WHITE;
            default:
                break;
            }
            assert(false);
            return VkBorderColor::VK_BORDER_COLOR_INT_OPAQUE_WHITE;
        }

        VkAttachmentLoadOp AttachmentLoadOpToVulkan(AttachmentLoadOp op)
        {
            switch (op)
            {
            case Insight::Graphics::AttachmentLoadOp::Load:     return VK_ATTACHMENT_LOAD_OP_LOAD;
            case Insight::Graphics::AttachmentLoadOp::Clear:    return VK_ATTACHMENT_LOAD_OP_CLEAR;
            case Insight::Graphics::AttachmentLoadOp::DontCare: return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            default:
                break;
            }
            return VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
        }

        VkBufferUsageFlags BufferTypeToVulkanBufferUsageFlags(BufferType type)
        {
            switch (type)
            {
            case BufferType::Vertex:     return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            case BufferType::Index:      return VK_BUFFER_USAGE_INDEX_BUFFER_BIT   | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            case BufferType::Uniform:    return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            case BufferType::Storage:    return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            case BufferType::Raw:        return VK_BUFFER_USAGE_TRANSFER_DST_BIT   | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            case BufferType::Staging:    return VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            case BufferType::Readback:   return VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            default:
                break;
            }
            return VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        }

        VkPolygonMode PolygonModeToVulkan(PolygonMode mode)
        {
            switch (mode)
            {
            case PolygonMode::Fill:     return VK_POLYGON_MODE_FILL;
            case PolygonMode::Line:     return VK_POLYGON_MODE_LINE;
            case PolygonMode::Point:    return VK_POLYGON_MODE_POINT;
            default:
                break;
            }
            return VkPolygonMode::VK_POLYGON_MODE_FILL;
        }

        VkCullModeFlags CullModeToVulkan(CullMode cullMode)
        {
            switch (cullMode)
            {
            case CullMode::None:         return VK_CULL_MODE_NONE;
            case CullMode::Front:        return VK_CULL_MODE_FRONT_BIT;
            case CullMode::Back:         return VK_CULL_MODE_BACK_BIT;
            case CullMode::FrontAndBack: return VK_CULL_MODE_FRONT_AND_BACK;
            default:
                break;
            }
            return VK_CULL_MODE_NONE;
        }

        VkFrontFace FrontFaceToVulkan(FrontFace front_face)
        {
            switch (front_face)
            {
            case FrontFace::CounterClockwise:    return VK_FRONT_FACE_COUNTER_CLOCKWISE;
            case FrontFace::Clockwise:           return VK_FRONT_FACE_CLOCKWISE;
            default:
                break;
            }
            assert(false);
            return VK_FRONT_FACE_COUNTER_CLOCKWISE;
        }

        VkDescriptorType DescriptorTypeToVulkan(DescriptorType type)
        {
            switch (type)
            {
            case DescriptorType::Sampler:                    return VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLER;
            case DescriptorType::Combined_Image_Sampler:     return VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            case DescriptorType::Sampled_Image:              return VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            case DescriptorType::Storage_Image:              return VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            case DescriptorType::Uniform_Texel_Buffer:       return VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
            case DescriptorType::Storage_Texel_Buffer:       return VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
            case DescriptorType::Unifom_Buffer:              return VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            case DescriptorType::Storage_Buffer:             return VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            case DescriptorType::Uniform_Buffer_Dynamic:     return VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
            case DescriptorType::Storage_Buffer_Dyanmic:     return VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
            case DescriptorType::Input_Attachment:           return VkDescriptorType::VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
            case DescriptorType::Unknown:                    return VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            default:
                break;
            }
            assert(false);
            return (VkDescriptorType)0;
        }

        VkAccessFlags AccessFlagsToVulkan(AccessFlags flags)
        {
            VkAccessFlags result = {};
            COMPARE_AND_SET_BIT(flags, VK_ACCESS_INDIRECT_COMMAND_READ_BIT, result);
            COMPARE_AND_SET_BIT(flags, VK_ACCESS_INDEX_READ_BIT, result);
            COMPARE_AND_SET_BIT(flags, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, result);
            COMPARE_AND_SET_BIT(flags, VK_ACCESS_UNIFORM_READ_BIT, result);
            COMPARE_AND_SET_BIT(flags, VK_ACCESS_INPUT_ATTACHMENT_READ_BIT, result);
            COMPARE_AND_SET_BIT(flags, VK_ACCESS_SHADER_READ_BIT, result);
            COMPARE_AND_SET_BIT(flags, VK_ACCESS_SHADER_WRITE_BIT, result);
            COMPARE_AND_SET_BIT(flags, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT, result);
            COMPARE_AND_SET_BIT(flags, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, result);
            COMPARE_AND_SET_BIT(flags, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT, result);
            COMPARE_AND_SET_BIT(flags, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, result);
            COMPARE_AND_SET_BIT(flags, VK_ACCESS_TRANSFER_READ_BIT, result);
            COMPARE_AND_SET_BIT(flags, VK_ACCESS_TRANSFER_WRITE_BIT, result);
            COMPARE_AND_SET_BIT(flags, VK_ACCESS_HOST_READ_BIT, result);
            COMPARE_AND_SET_BIT(flags, VK_ACCESS_HOST_WRITE_BIT, result);
            COMPARE_AND_SET_BIT(flags, VK_ACCESS_MEMORY_READ_BIT, result);
            COMPARE_AND_SET_BIT(flags, VK_ACCESS_MEMORY_WRITE_BIT, result);
            COMPARE_AND_SET_BIT(flags, VK_ACCESS_NONE, result);
            return result;
        }

        VkImageLayout ImageLayoutToVulkan(ImageLayout layout)
        {
            switch (layout)
            {
            case ImageLayout::Undefined:                      return VK_IMAGE_LAYOUT_UNDEFINED;
            case ImageLayout::General:                        return VK_IMAGE_LAYOUT_GENERAL;
            case ImageLayout::ColourAttachment:               return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            case ImageLayout::DepthStencilAttachment:         return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            case ImageLayout::DepthStencilAttachmentReadOnly: return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
            case ImageLayout::ShaderReadOnly:                 return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            case ImageLayout::TransforSrc:                    return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            case ImageLayout::TransforDst:                    return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            case ImageLayout::Preinitialised:                 return VK_IMAGE_LAYOUT_PREINITIALIZED;
            case ImageLayout::DepthReadOnlyStencilAttacment:  return VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL;
            case ImageLayout::DepthAttachmentStencilReadOnly: return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL;
            case ImageLayout::DepthAttachmentOnly:            return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
            case ImageLayout::DepthReadOnly:                  return VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
            case ImageLayout::StencilAttacment:               return VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
            case ImageLayout::StencilReadOnly:                return VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL;
            case ImageLayout::PresentSrc:                     return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            default:
                break;
            }
            return VK_IMAGE_LAYOUT_UNDEFINED;
        }

        VkImageAspectFlags ImageAspectFlagsToVulkan(ImageAspectFlags flags)
        {
            VkImageAspectFlags result = {};
            COMPARE_AND_SET_BIT(flags, VK_IMAGE_ASPECT_COLOR_BIT, result);
            COMPARE_AND_SET_BIT(flags, VK_IMAGE_ASPECT_DEPTH_BIT, result);
            COMPARE_AND_SET_BIT(flags, VK_IMAGE_ASPECT_STENCIL_BIT, result);                
            return result;
        }

        VkIndexType IndexTypeToVulkan(IndexType index_type)
        {
            switch (index_type)
            {
            case IndexType::Uint16: return VK_INDEX_TYPE_UINT16;
            case IndexType::Uint32: return VK_INDEX_TYPE_UINT32;
            default:
                break;
            }
            assert(false);
            return VK_INDEX_TYPE_UINT32;
        }

        VkImageType TextureTypeToVulkan(TextureType type)
        {
            switch (type)
            {
            case TextureType::Unknown:       return VK_IMAGE_TYPE_2D;
            case TextureType::Tex1D:         return VK_IMAGE_TYPE_1D;
            case TextureType::Tex2D:         return VK_IMAGE_TYPE_2D;
            case TextureType::Tex3D:         return VK_IMAGE_TYPE_3D;
            case TextureType::TexCube:       return VK_IMAGE_TYPE_2D;
            case TextureType::Tex2DArray:    return VK_IMAGE_TYPE_2D;
            case TextureType::Tex3DArray:    return VK_IMAGE_TYPE_3D;
            default:
                break;
            }
            assert(false);
            return VK_IMAGE_TYPE_2D;
        }

        VkImageViewType TextureViewTypeToVulkan(TextureType type)
        {
            switch (type)
            {
            case TextureType::Unknown:       return VK_IMAGE_VIEW_TYPE_2D;
            case TextureType::Tex1D:         return VK_IMAGE_VIEW_TYPE_1D;
            case TextureType::Tex2D:         return VK_IMAGE_VIEW_TYPE_2D;
            case TextureType::Tex3D:         return VK_IMAGE_VIEW_TYPE_3D;
            case TextureType::TexCube:       return VK_IMAGE_VIEW_TYPE_CUBE;
            case TextureType::Tex2DArray:    return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
            default:
                break;
            }
            assert(false);
            return VK_IMAGE_VIEW_TYPE_2D;
        }

        VkDynamicState DynamicStateToVulkan(DynamicState dynamic_state)
        {
            switch (dynamic_state)
            {
            case Insight::Graphics::DynamicState::Viewport:     return VK_DYNAMIC_STATE_VIEWPORT;
            case Insight::Graphics::DynamicState::Scissor:      return VK_DYNAMIC_STATE_SCISSOR;
            case Insight::Graphics::DynamicState::DepthBias:    return VK_DYNAMIC_STATE_DEPTH_BIAS;
            case Insight::Graphics::DynamicState::LineWidth:    return VK_DYNAMIC_STATE_LINE_WIDTH;
            default:
                break;
            }
            assert(false);
            return VK_DYNAMIC_STATE_VIEWPORT;
        }

        std::vector<VkDynamicState> DynamicStatesToVulkan(std::vector<DynamicState> dynamic_states)
        {
            std::vector<VkDynamicState> states;
            for (const DynamicState ds : dynamic_states)
            {
                states.push_back(DynamicStateToVulkan(ds));
            }
            return states;
        }

        VmaAllocationCreateFlags BufferTypeToVMAAllocCreateFlags(BufferType type)
        {
            switch (type)
            {
            case BufferType::Vertex:     return 0;
            case BufferType::Index:      return 0;
            case BufferType::Uniform:    return VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
            case BufferType::Storage:    return VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
            case BufferType::Raw:        return 0;
            case BufferType::Staging:    return VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
            case BufferType::Readback:   return VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
            default:
                break;
            }
            return 0;
        }

        VmaMemoryUsage BufferTypeToVMAUsage(BufferType type)
        {
            switch (type)
            {
            case BufferType::Vertex:     return VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO;
            case BufferType::Index:      return VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO;
            case BufferType::Uniform:    return VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
            case BufferType::Storage:    return VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
            case BufferType::Raw:        return VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO;
            case BufferType::Staging:    return VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
            case BufferType::Readback:   return VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
            default:
                break;
            }
            return VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO;
        }


        ImageLayout VkToImageLayout(VkImageLayout layout)
        {
            switch (layout)
            {
            case VK_IMAGE_LAYOUT_UNDEFINED:                                     return ImageLayout::Undefined;
            case VK_IMAGE_LAYOUT_GENERAL:                                       return ImageLayout::General;
            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:                      return ImageLayout::ColourAttachment;
            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:              return ImageLayout::DepthStencilAttachment;
            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:               return ImageLayout::DepthStencilAttachmentReadOnly;
            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:                      return ImageLayout::ShaderReadOnly;
            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:                          return ImageLayout::TransforSrc;
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:                          return ImageLayout::TransforDst;
            case VK_IMAGE_LAYOUT_PREINITIALIZED:                                return ImageLayout::Preinitialised;
            case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:    return ImageLayout::DepthReadOnlyStencilAttacment;
            case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:    return ImageLayout::DepthAttachmentStencilReadOnly;
            case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:                      return ImageLayout::DepthAttachmentOnly;
            case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:                       return ImageLayout::DepthReadOnly;
            case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL:                    return ImageLayout::StencilAttacment;
            case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL:                     return ImageLayout::StencilReadOnly;
            case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:                               return ImageLayout::PresentSrc;
            default:
                break;
            }
            assert(false);
            return ImageLayout::Undefined;
        }

        AttachmentLoadOp VkToAttachmentLoadOp(VkAttachmentLoadOp op)
        {
            switch (op)
            {
            case VK_ATTACHMENT_LOAD_OP_LOAD:        return AttachmentLoadOp::Load;
            case VK_ATTACHMENT_LOAD_OP_CLEAR:       return AttachmentLoadOp::Clear;
            case VK_ATTACHMENT_LOAD_OP_DONT_CARE:   return AttachmentLoadOp::DontCare;
            default:
                break;
            }
            assert(false);
            return AttachmentLoadOp::Clear;
        }
    
#undef COMPARE_AND_SET_BIT
    }
}

#endif ///#if defined(IS_VULKAN_ENABLED)