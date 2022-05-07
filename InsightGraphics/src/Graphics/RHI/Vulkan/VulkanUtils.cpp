#include "Graphics/RHI/Vulkan/VulkanUtils.h"

#ifdef IS_PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
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

    vk::Format::eUndefined, // TODO: R32G8X24_Typeless
    vk::Format::eD32SfloatS8Uint,
    vk::Format::eUndefined,  // TODO: R32_Float_X8X24_Typeless
    vk::Format::eUndefined, // TODO: X32_Typeless_G8X24_UInt

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
    
    vk::Format::eUndefined, // TODO: A8_UNorm
    vk::Format::eUndefined, // TODO: R1_UNorm

    vk::Format::eE5B9G9R9UfloatPack32,

    vk::Format::eUndefined, // TODO: R8G8_B8G8_UNorm
    vk::Format::eUndefined, // TODO: G8R8_G8B8_UNorm

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
    vk::Format::eUndefined, // TODO: R10G10B10_Xr_Bias_A2_UNorm

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
    // 10
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
    // 20
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
    // 30
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
    // 40
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

    vk::Format::eUndefined, // TODO: R32G8X24_Typeless
    vk::Format::eD32SfloatS8Uint,
    vk::Format::eUndefined,  // TODO: R32_Float_X8X24_Typeless
    vk::Format::eUndefined, // TODO: X32_Typeless_G8X24_UInt

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
    
    vk::Format::eUndefined, // TODO: A8_UNorm
    vk::Format::eUndefined, // TODO: R1_UNorm

    vk::Format::eE5B9G9R9UfloatPack32,

    vk::Format::eUndefined, // TODO: R8G8_B8G8_UNorm
    vk::Format::eUndefined, // TODO: G8R8_G8B8_UNorm

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
    vk::Format::eUndefined, // TODO: R10G10B10_Xr_Bias_A2_UNorm

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
    // 10
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
    // 20
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
    // 30
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
    // 40
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

        vk::BufferUsageFlags GPUBufferTypeToVulkanBufferUsageFlags(GPUBufferType type)
        {
            switch (type)
            {
            case GPUBufferType::Invalid:    break;
            case GPUBufferType::Vertex:     return vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst;
            case GPUBufferType::Index:      return vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst;
            case GPUBufferType::Uniform:    return vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst;
            case GPUBufferType::Constant:   return vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst;
            case GPUBufferType::Staging:    return vk::BufferUsageFlagBits::eTransferSrc;
            case GPUBufferType::Readback:   return vk::BufferUsageFlagBits::eTransferDst;
            default:
                break;
            }
            return vk::BufferUsageFlags();
        }

        vk::CullModeFlags CullModeToVulkan(CullMode cullMode)
        {
            switch (cullMode)
            {
            case Insight::Graphics::CullMode::None: return vk::CullModeFlagBits::eNone;
            case Insight::Graphics::CullMode::Front: return vk::CullModeFlagBits::eFront;
            case Insight::Graphics::CullMode::Back: return vk::CullModeFlagBits::eBack;
            case Insight::Graphics::CullMode::FrontAndBack: return vk::CullModeFlagBits::eFrontAndBack;
            default:
                break;
            }
            return vk::CullModeFlagBits::eNone;
        }

        VmaAllocationCreateFlags GPUBufferTypeToVMAAllocCreateFlags(GPUBufferType type)
        {
            switch (type)
            {
            case GPUBufferType::Invalid:    break;
            case GPUBufferType::Vertex:     return 0;
            case GPUBufferType::Index:      return 0;
            case GPUBufferType::Uniform:    return VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
            case GPUBufferType::Constant:   return VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
            case GPUBufferType::Staging:    return VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
            case GPUBufferType::Readback:   return VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
            default:
                break;
            }
            return 0;
        }

        VmaMemoryUsage GPUBufferTypeToVMAUsage(GPUBufferType type)
        {
            switch (type)
            {
            case GPUBufferType::Invalid:    break;
            case GPUBufferType::Vertex:     return VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO;
            case GPUBufferType::Index:      return VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO;
            case GPUBufferType::Uniform:    return VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
            case GPUBufferType::Constant:   return VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
            case GPUBufferType::Staging:    return VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
            case GPUBufferType::Readback:   return VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
            default:
                break;
            }
            return VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO;
        }
    }
}