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

        vk::ImageUsageFlags ImageUsageFlagsToVulkan(ImageUsageFlags imageUsageFlags)
        {
            vk::ImageUsageFlags flags;
            if (imageUsageFlags & ImageUsageFlagsBits::TransferSrc) { flags |= vk::ImageUsageFlagBits::eTransferSrc; }
            if (imageUsageFlags & ImageUsageFlagsBits::TransferDst) { flags |= vk::ImageUsageFlagBits::eTransferDst; }
            if (imageUsageFlags & ImageUsageFlagsBits::Sampled) { flags |= vk::ImageUsageFlagBits::eSampled; }
            if (imageUsageFlags & ImageUsageFlagsBits::Storage) { flags |= vk::ImageUsageFlagBits::eStorage; }
            if (imageUsageFlags & ImageUsageFlagsBits::ColourAttachment) { flags |= vk::ImageUsageFlagBits::eColorAttachment; }
            if (imageUsageFlags & ImageUsageFlagsBits::DepthStencilAttachment) { flags |= vk::ImageUsageFlagBits::eDepthStencilAttachment; }
            if (imageUsageFlags & ImageUsageFlagsBits::TransientAttachment) { flags |= vk::ImageUsageFlagBits::eTransientAttachment; }
            if (imageUsageFlags & ImageUsageFlagsBits::InputAttachment) { flags |= vk::ImageUsageFlagBits::eInputAttachment; }
            return flags;
        }

        vk::Format PixelFormatToVulkan(PixelFormat format)
        {
            return PixelFormatToVkFormat[(int)format];
        }

        vk::PipelineBindPoint GPUQueueToVulkanBindPoint(GPUQueue queue)
        {
            switch (queue)
            {
            case GPUQueue_Graphics: return vk::PipelineBindPoint::eGraphics;
            case GPUQueue_Compute: return vk::PipelineBindPoint::eCompute;
            default:
                break;
            }
            return vk::PipelineBindPoint::eGraphics;
        }

        vk::PipelineStageFlags PipelineStageFlagsToVulkan(PipelineStageFlags flags)
        {
            vk::PipelineStageFlags result;

            if (flags & +PipelineStageFlagBits::TopOfPipe)                       { result |= vk::PipelineStageFlagBits::eTopOfPipe; }
            if (flags & +PipelineStageFlagBits::DrawIndirect)                    { result |= vk::PipelineStageFlagBits::eDrawIndirect; }
            if (flags & +PipelineStageFlagBits::VertexInput)                     { result |= vk::PipelineStageFlagBits::eVertexInput; }
            if (flags & +PipelineStageFlagBits::VertexShader)                    { result |= vk::PipelineStageFlagBits::eVertexShader; }
            if (flags & +PipelineStageFlagBits::TessesllationControlShader)      { result |= vk::PipelineStageFlagBits::eTessellationControlShader; }
            if (flags & +PipelineStageFlagBits::TessesllationEvaluationShader)   { result |= vk::PipelineStageFlagBits::eTessellationEvaluationShader; }
            if (flags & +PipelineStageFlagBits::GeometryShader)                  { result |= vk::PipelineStageFlagBits::eGeometryShader; }
            if (flags & +PipelineStageFlagBits::FragmentShader)                  { result |= vk::PipelineStageFlagBits::eFragmentShader; }
            if (flags & +PipelineStageFlagBits::EarlyFramgmentShader)            { result |= vk::PipelineStageFlagBits::eEarlyFragmentTests; }
            if (flags & +PipelineStageFlagBits::LateFramgmentShader)             { result |= vk::PipelineStageFlagBits::eLateFragmentTests; }
            if (flags & +PipelineStageFlagBits::ColourAttachmentOutput)          { result |= vk::PipelineStageFlagBits::eColorAttachmentOutput; }
            if (flags & +PipelineStageFlagBits::ComputeShader)                   { result |= vk::PipelineStageFlagBits::eComputeShader; }
            if (flags & +PipelineStageFlagBits::Transfer)                        { result |= vk::PipelineStageFlagBits::eTransfer; }
            if (flags & +PipelineStageFlagBits::BottomOfPipe)                    { result |= vk::PipelineStageFlagBits::eBottomOfPipe; }
            if (flags & +PipelineStageFlagBits::Host)                            { result |= vk::PipelineStageFlagBits::eHost; }
            if (flags & +PipelineStageFlagBits::AllGraphics)                     { result |= vk::PipelineStageFlagBits::eAllGraphics; }
            if (flags & +PipelineStageFlagBits::AllCommands)                     { result |= vk::PipelineStageFlagBits::eAllCommands; }

            return result;
        }

        vk::ShaderStageFlagBits ShaderStageFlagBitsToVulkan(ShaderStageFlagBits stage)
        {
            switch (stage)
            {
            case ShaderStage_Vertex: return vk::ShaderStageFlagBits::eVertex;
            case ShaderStage_TessControl: return vk::ShaderStageFlagBits::eTessellationControl;
            case ShaderStage_TessEval: return vk::ShaderStageFlagBits::eTessellationEvaluation;
            case ShaderStage_Geometry: return vk::ShaderStageFlagBits::eGeometry;
            case ShaderStage_Pixel: return vk::ShaderStageFlagBits::eFragment;
            default:
                break;
            }
            return vk::ShaderStageFlagBits::eVertex;
        }

        vk::ShaderStageFlags ShaderStageFlagsToVulkan(ShaderStageFlags flags)
        {
            vk::ShaderStageFlags result = {};
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

        vk::PrimitiveTopology PrimitiveTopologyTypeToVulkan(PrimitiveTopologyType type)
        {
            switch (type)
            {
            case PrimitiveTopologyType::PointList: return vk::PrimitiveTopology::ePointList;
            case PrimitiveTopologyType::LineList: return vk::PrimitiveTopology::eLineList;
            case PrimitiveTopologyType::LineStrip: return vk::PrimitiveTopology::eLineStrip;
            case PrimitiveTopologyType::TriangleList: return vk::PrimitiveTopology::eTriangleList;
            case PrimitiveTopologyType::TriangleStrip: return vk::PrimitiveTopology::eTriangleStrip;
            case PrimitiveTopologyType::TriangleFan: return vk::PrimitiveTopology::eTriangleFan;
            case PrimitiveTopologyType::LineListWithAdjacency: return vk::PrimitiveTopology::eLineListWithAdjacency;
            case PrimitiveTopologyType::LineStripWithAdjacency: return vk::PrimitiveTopology::eLineStripWithAdjacency;
            case PrimitiveTopologyType::TriangleListWithAdjacency: return vk::PrimitiveTopology::eTriangleListWithAdjacency;
            case PrimitiveTopologyType::TriangleStripWithAdjacency: return vk::PrimitiveTopology::eTriangleStripWithAdjacency;
            case PrimitiveTopologyType::PatchList: return vk::PrimitiveTopology::ePatchList;
            default:
                break;
            }
            return vk::PrimitiveTopology::eTriangleList;
        }

        vk::ColorComponentFlags ColourComponentFlagsToVulkan(ColourComponentFlags flags)
        {
            vk::ColorComponentFlags flag(0);
            if (flags & ColourComponentR) { flag |= vk::ColorComponentFlagBits::eR; }
            if (flags & ColourComponentG) { flag |= vk::ColorComponentFlagBits::eG; }
            if (flags & ColourComponentB) { flag |= vk::ColorComponentFlagBits::eB; }
            if (flags & ColourComponentA) { flag |= vk::ColorComponentFlagBits::eA; }
            return flag;
        }

        vk::BlendFactor BlendFactorToVulkan(BlendFactor factor)
        {
            switch (factor)
            {
            case Insight::Graphics::BlendFactor::Zero: return vk::BlendFactor::eZero;
            case Insight::Graphics::BlendFactor::One: return vk::BlendFactor::eOne;
            case Insight::Graphics::BlendFactor::SrcColour: return vk::BlendFactor::eSrcColor;
            case Insight::Graphics::BlendFactor::OneMinusSrcColour: return vk::BlendFactor::eOneMinusSrcColor;
            case Insight::Graphics::BlendFactor::DstColour: return vk::BlendFactor::eDstColor;
            case Insight::Graphics::BlendFactor::OneMinusDstColour: return vk::BlendFactor::eOneMinusDstColor;
            case Insight::Graphics::BlendFactor::SrcAlpha: return vk::BlendFactor::eSrcAlpha;
            case Insight::Graphics::BlendFactor::OneMinusSrcAlpha: return vk::BlendFactor::eOneMinusSrcAlpha;
            case Insight::Graphics::BlendFactor::DstAlpha: return vk::BlendFactor::eDstAlpha;
            case Insight::Graphics::BlendFactor::OneMinusDstAlpha: return vk::BlendFactor::eOneMinusDstAlpha;
            case Insight::Graphics::BlendFactor::ConstantColour: return vk::BlendFactor::eConstantColor;
            case Insight::Graphics::BlendFactor::OneMinusConstantColour: return vk::BlendFactor::eOneMinusConstantColor;
            case Insight::Graphics::BlendFactor::ConstantAlpha: return vk::BlendFactor::eConstantAlpha;
            case Insight::Graphics::BlendFactor::OneMinusConstantAlpha: return vk::BlendFactor::eOneMinusConstantAlpha;
            case Insight::Graphics::BlendFactor::SrcAplhaSaturate: return vk::BlendFactor::eSrcAlphaSaturate;
            case Insight::Graphics::BlendFactor::Src1Colour: return vk::BlendFactor::eSrc1Color;
            case Insight::Graphics::BlendFactor::OneMinusSrc1Colour: return vk::BlendFactor::eOneMinusSrc1Color;
            case Insight::Graphics::BlendFactor::Src1Alpha: return vk::BlendFactor::eSrc1Alpha;
            case Insight::Graphics::BlendFactor::OneMinusSrc1Alpha: return vk::BlendFactor::eOneMinusSrc1Alpha;
            default:
                break;
            }
            return vk::BlendFactor::eZero;
        }

        vk::BlendOp BlendOpToVulkan(BlendOp op)
        {
            switch (op)
            {
            case Insight::Graphics::BlendOp::Add: return vk::BlendOp::eAdd;
            case Insight::Graphics::BlendOp::Subtract: return vk::BlendOp::eSubtract;
            case Insight::Graphics::BlendOp::ReverseSubtract: return vk::BlendOp::eReverseSubtract;
            case Insight::Graphics::BlendOp::Min: return vk::BlendOp::eMin;
            case Insight::Graphics::BlendOp::Max: return vk::BlendOp::eMax;
            default:
                break;
            }
            return vk::BlendOp::eAdd;
        }

        vk::CompareOp CompareOpToVulkan(CompareOp op)
        {
            switch (op)
            {
            case CompareOp::Never:          return vk::CompareOp::eNever;
            case CompareOp::Less:           return vk::CompareOp::eLess;
            case CompareOp::Equal:          return vk::CompareOp::eEqual;
            case CompareOp::LessOrEqual:    return vk::CompareOp::eLessOrEqual;
            case CompareOp::Greater:        return vk::CompareOp::eGreater;
            case CompareOp::NotEqual:       return vk::CompareOp::eNotEqual;
            case CompareOp::GreaterOrEqual: return vk::CompareOp::eGreaterOrEqual;
            case CompareOp::Always:         return vk::CompareOp::eAlways;
            default:
                break;
            }
            return vk::CompareOp::eNever;
        }

        vk::Filter FilterToVulkan(Filter filter)
        {
            switch (filter)
            {
            case Filter::Nearest:    return vk::Filter::eNearest;
            case Filter::Linear:     return vk::Filter::eLinear;
            default:
                break;
            }
            assert(false);
            return vk::Filter::eNearest;
        }

        vk::SamplerMipmapMode SamplerMipmapModeToVulkan(SamplerMipmapMode sampler_mipmap_mode)
        {
            switch (sampler_mipmap_mode)
            {
            case SamplerMipmapMode::Nearest:    return vk::SamplerMipmapMode::eNearest;
            case SamplerMipmapMode::Linear:     return vk::SamplerMipmapMode::eLinear;
            default:
                break;
            }
            assert(false);
            return vk::SamplerMipmapMode::eNearest;
        }

        vk::SamplerAddressMode SamplerAddressModeToVulkan(SamplerAddressMode sampler_address_mode)
        {
            switch (sampler_address_mode)
            {
            case SamplerAddressMode::Repeat:                return vk::SamplerAddressMode::eRepeat;
            case SamplerAddressMode::MirroredRepeat:        return vk::SamplerAddressMode::eMirroredRepeat;
            case SamplerAddressMode::ClampToEdge:           return vk::SamplerAddressMode::eClampToEdge;
            case SamplerAddressMode::ClampToBoarder:        return vk::SamplerAddressMode::eClampToBorder;
            case SamplerAddressMode::MirrorClampToEdge:     return vk::SamplerAddressMode::eMirrorClampToEdge;
            default:
                break;
            }
            assert(false);
            return vk::SamplerAddressMode::eMirroredRepeat;
        }

        vk::BorderColor BorderColourToVulkan(BorderColour border_colour)
        {
            switch (border_colour)
            {
            case BorderColour::FloatTransparentBlack:    return vk::BorderColor::eFloatTransparentBlack;
            case BorderColour::IntTransparentBlack:      return vk::BorderColor::eIntTransparentBlack;
            case BorderColour::FloatOpaqueBlack:         return vk::BorderColor::eFloatOpaqueBlack;
            case BorderColour::IntOpaqueBlack:           return vk::BorderColor::eIntOpaqueBlack;
            case BorderColour::FloatOpaqueWhite:         return vk::BorderColor::eFloatOpaqueWhite;
            case BorderColour::IntOpaqueWhite:           return vk::BorderColor::eIntOpaqueWhite;
            default:
                break;
            }
            assert(false);
            return vk::BorderColor::eFloatOpaqueWhite;
        }

        vk::AttachmentLoadOp AttachmentLoadOpToVulkan(AttachmentLoadOp op)
        {
            switch (op)
            {
            case Insight::Graphics::AttachmentLoadOp::Load: return vk::AttachmentLoadOp::eLoad;
            case Insight::Graphics::AttachmentLoadOp::Clear: return vk::AttachmentLoadOp::eClear;
            case Insight::Graphics::AttachmentLoadOp::DontCare: return vk::AttachmentLoadOp::eDontCare;
            default:
                break;
            }
            return vk::AttachmentLoadOp::eClear;
        }

        vk::BufferUsageFlags BufferTypeToVulkanBufferUsageFlags(BufferType type)
        {
            switch (type)
            {
            case BufferType::Vertex:     return vk::BufferUsageFlagBits::eVertexBuffer  | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eTransferSrc;
            case BufferType::Index:      return vk::BufferUsageFlagBits::eIndexBuffer   | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eTransferSrc;
            case BufferType::Uniform:    return vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eTransferSrc;
            case BufferType::Storage:    return vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eTransferSrc;
            case BufferType::Raw:        return vk::BufferUsageFlagBits::eTransferDst   | vk::BufferUsageFlagBits::eTransferSrc;
            case BufferType::Staging:    return vk::BufferUsageFlagBits::eTransferSrc;
            case BufferType::Readback:   return vk::BufferUsageFlagBits::eTransferDst;
            default:
                break;
            }
            return vk::BufferUsageFlags();
        }

        vk::PolygonMode PolygonModeToVulkan(PolygonMode mode)
        {
            switch (mode)
            {
            case PolygonMode::Fill:     return vk::PolygonMode::eFill;
            case PolygonMode::Line:     return vk::PolygonMode::eLine;
            case PolygonMode::Point:    return vk::PolygonMode::ePoint;
            default:
                break;
            }
            return vk::PolygonMode::eFill;
        }

        vk::CullModeFlags CullModeToVulkan(CullMode cullMode)
        {
            switch (cullMode)
            {
            case CullMode::None: return vk::CullModeFlagBits::eNone;
            case CullMode::Front: return vk::CullModeFlagBits::eFront;
            case CullMode::Back: return vk::CullModeFlagBits::eBack;
            case CullMode::FrontAndBack: return vk::CullModeFlagBits::eFrontAndBack;
            default:
                break;
            }
            return vk::CullModeFlagBits::eNone;
        }

        vk::FrontFace FrontFaceToVulkan(FrontFace front_face)
        {
            switch (front_face)
            {
            case FrontFace::CounterClockwise:    return vk::FrontFace::eCounterClockwise;
            case FrontFace::Clockwise:           return vk::FrontFace::eCounterClockwise;
            default:
                break;
            }
            assert(false);
            return vk::FrontFace::eCounterClockwise;
        }

        vk::DescriptorType DescriptorTypeToVulkan(DescriptorType type)
        {
            switch (type)
            {
            case DescriptorType::Sampler:                    return vk::DescriptorType::eSampler;
            case DescriptorType::Combined_Image_Sampler:     return vk::DescriptorType::eCombinedImageSampler;
            case DescriptorType::Sampled_Image:              return vk::DescriptorType::eSampledImage;
            case DescriptorType::Storage_Image:              return vk::DescriptorType::eStorageImage;
            case DescriptorType::Uniform_Texel_Buffer:       return vk::DescriptorType::eUniformTexelBuffer;
            case DescriptorType::Storage_Texel_Buffer:       return vk::DescriptorType::eStorageTexelBuffer;
            case DescriptorType::Unifom_Buffer:              return vk::DescriptorType::eUniformBuffer;
            case DescriptorType::Storage_Buffer:             return vk::DescriptorType::eStorageBuffer;
            case DescriptorType::Uniform_Buffer_Dynamic:     return vk::DescriptorType::eUniformBufferDynamic;
            case DescriptorType::Storage_Buffer_Dyanmic:     return vk::DescriptorType::eStorageBufferDynamic;
            case DescriptorType::Input_Attachment:           return vk::DescriptorType::eInputAttachment;
            case DescriptorType::Unknown:                    return vk::DescriptorType::eCombinedImageSampler;
            default:
                break;
            }
            return vk::DescriptorType::eCombinedImageSampler;
        }

        vk::AccessFlags AccessFlagsToVulkan(AccessFlags flags)
        {
            vk::AccessFlags result;

            if (flags & +vk::AccessFlagBits::eIndirectCommandRead)           { result |= vk::AccessFlagBits::eIndirectCommandRead; }
            if (flags & +vk::AccessFlagBits::eIndexRead)                     { result |= vk::AccessFlagBits::eIndexRead; }
            if (flags & +vk::AccessFlagBits::eVertexAttributeRead)           { result |= vk::AccessFlagBits::eVertexAttributeRead; }
            if (flags & +vk::AccessFlagBits::eUniformRead)                   { result |= vk::AccessFlagBits::eUniformRead; }
            if (flags & +vk::AccessFlagBits::eInputAttachmentRead)           { result |= vk::AccessFlagBits::eInputAttachmentRead; }
            if (flags & +vk::AccessFlagBits::eShaderRead)                    { result |= vk::AccessFlagBits::eShaderRead; }
            if (flags & +vk::AccessFlagBits::eShaderWrite)                   { result |= vk::AccessFlagBits::eShaderWrite; }
            if (flags & +vk::AccessFlagBits::eColorAttachmentRead)           { result |= vk::AccessFlagBits::eColorAttachmentRead; }
            if (flags & +vk::AccessFlagBits::eColorAttachmentWrite)          { result |= vk::AccessFlagBits::eColorAttachmentWrite; }
            if (flags & +vk::AccessFlagBits::eDepthStencilAttachmentRead)    { result |= vk::AccessFlagBits::eDepthStencilAttachmentRead; }
            if (flags & +vk::AccessFlagBits::eDepthStencilAttachmentWrite)   { result |= vk::AccessFlagBits::eDepthStencilAttachmentWrite; }
            if (flags & +vk::AccessFlagBits::eTransferRead)                  { result |= vk::AccessFlagBits::eTransferRead; }
            if (flags & +vk::AccessFlagBits::eTransferWrite)                 { result |= vk::AccessFlagBits::eTransferWrite; }
            if (flags & +vk::AccessFlagBits::eHostRead)                      { result |= vk::AccessFlagBits::eHostRead; }
            if (flags & +vk::AccessFlagBits::eHostWrite)                     { result |= vk::AccessFlagBits::eHostWrite; }
            if (flags & +vk::AccessFlagBits::eMemoryRead)                    { result |= vk::AccessFlagBits::eMemoryRead; }
            if (flags & +vk::AccessFlagBits::eMemoryWrite)                   { result |= vk::AccessFlagBits::eMemoryWrite; }
            if (flags & +vk::AccessFlagBits::eNoneKHR)                       { result |= vk::AccessFlagBits::eNoneKHR; }

            return result;
        }

        vk::ImageLayout ImageLayoutToVulkan(ImageLayout layout)
        {
            switch (layout)
            {
            case ImageLayout::Undefined: return vk::ImageLayout::eUndefined;
            case ImageLayout::General: return vk::ImageLayout::eGeneral;
            case ImageLayout::ColourAttachment: return vk::ImageLayout::eColorAttachmentOptimal;
            case ImageLayout::DepthStencilAttachment: return vk::ImageLayout::eDepthStencilAttachmentOptimal;
            case ImageLayout::DepthStencilAttachmentReadOnly: return vk::ImageLayout::eDepthStencilReadOnlyOptimal;
            case ImageLayout::ShaderReadOnly: return vk::ImageLayout::eShaderReadOnlyOptimal;
            case ImageLayout::TransforSrc: return vk::ImageLayout::eTransferSrcOptimal;
            case ImageLayout::TransforDst: return vk::ImageLayout::eTransferDstOptimal;
            case ImageLayout::Preinitialised: return vk::ImageLayout::ePreinitialized;
            case ImageLayout::DepthReadOnlyStencilAttacment: return vk::ImageLayout::eDepthReadOnlyStencilAttachmentOptimal;
            case ImageLayout::DepthAttachmentStencilReadOnly: return vk::ImageLayout::eDepthAttachmentStencilReadOnlyOptimal;
            case ImageLayout::DepthAttachmentOnly: return vk::ImageLayout::eDepthAttachmentOptimal;
            case ImageLayout::DepthReadOnly: return vk::ImageLayout::eDepthReadOnlyOptimal;
            case ImageLayout::StencilAttacment: return vk::ImageLayout::eStencilAttachmentOptimal;
            case ImageLayout::StencilReadOnly: return vk::ImageLayout::eStencilReadOnlyOptimal;
            case ImageLayout::PresentSrc: return vk::ImageLayout::ePresentSrcKHR;
            default:
                break;
            }
            return vk::ImageLayout::eUndefined;
        }

        vk::ImageAspectFlags ImageAspectFlagsToVulkan(ImageAspectFlags flags)
        {
            vk::ImageAspectFlags result;

            if (flags & +vk::ImageAspectFlagBits::eColor) { result |= vk::ImageAspectFlagBits::eColor; }
            if (flags & +vk::ImageAspectFlagBits::eDepth) { result |= vk::ImageAspectFlagBits::eDepth; }
            if (flags & +vk::ImageAspectFlagBits::eStencil) { result |= vk::ImageAspectFlagBits::eStencil; }
                
            return result;
        }

        vk::IndexType IndexTypeToVulkan(IndexType index_type)
        {
            switch (index_type)
            {
            case IndexType::Uint16: return vk::IndexType::eUint16;
            case IndexType::Uint32: return vk::IndexType::eUint32;
            default:
                break;
            }
            assert(false);
            return vk::IndexType::eUint16;
        }

        vk::ImageType TextureTypeToVulkan(TextureType type)
        {
            switch (type)
            {
            case TextureType::Unknown:       return vk::ImageType::e2D;
            case TextureType::Tex1D:         return vk::ImageType::e1D;
            case TextureType::Tex2D:         return vk::ImageType::e2D;
            case TextureType::Tex3D:         return vk::ImageType::e3D;
            case TextureType::TexCube:       return vk::ImageType::e2D;
            case TextureType::Tex2DArray:    return vk::ImageType::e2D;
            case TextureType::Tex3DArray:    return vk::ImageType::e3D;
            default:
                break;
            }
            assert(false);
            return vk::ImageType::e2D;
        }

        vk::ImageViewType TextureViewTypeToVulkan(TextureType type)
        {
            switch (type)
            {
            case TextureType::Unknown:       return vk::ImageViewType::e2D;
            case TextureType::Tex1D:         return vk::ImageViewType::e1D;
            case TextureType::Tex2D:         return vk::ImageViewType::e2D;
            case TextureType::Tex3D:         return vk::ImageViewType::e3D;
            case TextureType::TexCube:       return vk::ImageViewType::eCube;
            case TextureType::Tex2DArray:    return vk::ImageViewType::e2DArray;
            case TextureType::Tex3DArray:
            default:
                break;
            }
            assert(false);
            return vk::ImageViewType::e2D;
        }

        vk::DynamicState DynamicStateToVulkan(DynamicState dynamic_state)
        {
            switch (dynamic_state)
            {
            case Insight::Graphics::DynamicState::Viewport:     return vk::DynamicState::eViewport;
            case Insight::Graphics::DynamicState::Scissor:      return vk::DynamicState::eScissor;
            case Insight::Graphics::DynamicState::DepthBias:    return vk::DynamicState::eDepthBias;
            case Insight::Graphics::DynamicState::LineWidth:    return vk::DynamicState::eLineWidth;
            default:
                break;
            }
            assert(false);
            return vk::DynamicState::eViewport;
        }

        std::vector<vk::DynamicState> DynamicStatesToVulkan(std::vector<DynamicState> dynamic_states)
        {
            std::vector<vk::DynamicState> states;
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


        ImageLayout VkToImageLayout(vk::ImageLayout layout)
        {
            switch (layout)
            {
            case vk::ImageLayout::eUndefined:                               return ImageLayout::Undefined;
            case vk::ImageLayout::eGeneral:                                 return ImageLayout::General;
            case vk::ImageLayout::eColorAttachmentOptimal:                  return ImageLayout::ColourAttachment;
            case vk::ImageLayout::eDepthStencilAttachmentOptimal:           return ImageLayout::DepthStencilAttachment;
            case vk::ImageLayout::eDepthStencilReadOnlyOptimal:             return ImageLayout::DepthStencilAttachmentReadOnly;
            case vk::ImageLayout::eShaderReadOnlyOptimal:                   return ImageLayout::ShaderReadOnly;
            case vk::ImageLayout::eTransferSrcOptimal:                      return ImageLayout::TransforSrc;
            case vk::ImageLayout::eTransferDstOptimal:                      return ImageLayout::TransforDst;
            case vk::ImageLayout::ePreinitialized:                          return ImageLayout::Preinitialised;
            case vk::ImageLayout::eDepthReadOnlyStencilAttachmentOptimal:   return ImageLayout::DepthReadOnlyStencilAttacment;
            case vk::ImageLayout::eDepthAttachmentStencilReadOnlyOptimal:   return ImageLayout::DepthAttachmentStencilReadOnly;
            case vk::ImageLayout::eDepthAttachmentOptimal:                  return ImageLayout::DepthAttachmentOnly;
            case vk::ImageLayout::eDepthReadOnlyOptimal:                    return ImageLayout::DepthReadOnly;
            case vk::ImageLayout::eStencilAttachmentOptimal:                return ImageLayout::StencilAttacment;
            case vk::ImageLayout::eStencilReadOnlyOptimal:                  return ImageLayout::StencilReadOnly;
            case vk::ImageLayout::ePresentSrcKHR:                           return ImageLayout::PresentSrc;
            default:
                break;
            }
            assert(false); 
            return ImageLayout::Undefined;
        }

        AttachmentLoadOp VkToAttachmentLoadOp(vk::AttachmentLoadOp op)
        {
            switch (op)
            {
            case vk::AttachmentLoadOp::eLoad:       return AttachmentLoadOp::Load;
            case vk::AttachmentLoadOp::eClear:      return AttachmentLoadOp::Clear;
            case vk::AttachmentLoadOp::eDontCare:   return AttachmentLoadOp::DontCare;
            default:
                break;
            }
            assert(false);
            return AttachmentLoadOp::Clear;
        }
    }
}

#endif ///#if defined(IS_VULKAN_ENABLED)