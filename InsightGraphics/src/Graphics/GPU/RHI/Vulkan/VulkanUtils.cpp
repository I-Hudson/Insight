#include "Graphics/GPU/RHI/Vulkan/VulkanUtils.h"

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

//vk::Format PixelFormatToVkFormat[static_cast<int>(PixelFormat::MAX)] =
//{
//    vk::Format::eUndefined,
//    vk::Format::eR32G32B32A32Sfloat,
//    vk::Format::eR32G32B32A32Sfloat,
//    vk::Format::eR32G32B32A32Uint,
//    vk::Format::eR32G32B32A32Sint,
//
//    vk::Format::eR32G32B32Sfloat,
//    vk::Format::eR32G32B32Sfloat,
//    vk::Format::eR32G32B32Uint,
//    vk::Format::eR32G32B32Sint,
//
//    vk::Format::eR16G16B16A16Sfloat,
//    vk::Format::eR16G16B16A16Sfloat,
//    vk::Format::eR16G16B16A16Unorm,
//    vk::Format::eR16G16B16A16Uint,
//    vk::Format::eR16G16B16A16Snorm,
//    vk::Format::eR16G16B16A16Sint,
//
//    vk::Format::eR32G32Sfloat,
//    vk::Format::eR32G32Sfloat,
//    vk::Format::eR32G32Uint,
//    vk::Format::eR32G32Sint,
//
//    vk::Format::eUndefined, // TODO: R32G8X24_Typeless
//    vk::Format::eD32SfloatS8Uint,
//    vk::Format::eUndefined,  // TODO: R32_Float_X8X24_Typeless
//    vk::Format::eUndefined, // TODO: X32_Typeless_G8X24_UInt
//
//    vk::Format::eA2R10G10B10UnormPack32,
//    vk::Format::eA2R10G10B10UnormPack32,
//    vk::Format::eA2R10G10B10UintPack32,
//
//    vk::Format::eB10G11R11UfloatPack32,
//
//    vk::Format::eR8G8B8A8Unorm,
//    vk::Format::eR8G8B8A8Unorm,
//    vk::Format::eR8G8B8A8Srgb,
//    vk::Format::eR8G8B8A8Uint,
//    vk::Format::eR8G8B8A8Snorm,
//    vk::Format::eR8G8B8A8Sint,
//
//    vk::Format::eR16G16Sfloat,
//    vk::Format::eR16G16Sfloat,
//    vk::Format::eR16G16Unorm,
//    vk::Format::eR16G16Uint,
//    vk::Format::eR16G16Snorm,
//    vk::Format::eR16G16Sint,
//
//    vk::Format::eD32Sfloat,
//
//    vk::Format::eR32Sfloat,
//    vk::Format::eR32Sfloat,
//    vk::Format::eR32Uint,
//    vk::Format::eR32Sint,
//
//    vk::Format::eD24UnormS8Uint,
//    vk::Format::eD24UnormS8Uint,
//
//    vk::Format::eX8D24UnormPack32,
//    vk::Format::eD24UnormS8Uint,
//
//    vk::Format::eR8G8Unorm,
//    vk::Format::eR8G8Unorm,
//    vk::Format::eR8G8Uint,
//    vk::Format::eR8G8Snorm,
//    vk::Format::eR8G8Sint,
//
//    vk::Format::eD16Unorm,
//
//    vk::Format::eR16Sfloat,
//    vk::Format::eR16Sfloat,
//    vk::Format::eR16Unorm,
//    vk::Format::eR16Uint,
//    vk::Format::eR16Snorm,
//    vk::Format::eR16Sint,
//
//    vk::Format::eR8Unorm,
//    vk::Format::eR8Unorm,
//    vk::Format::eR8Uint,
//    vk::Format::eR8Snorm,
//    vk::Format::eR8Sint,
//    
//    vk::Format::eUndefined, // TODO: A8_UNorm
//    vk::Format::eUndefined, // TODO: R1_UNorm
//
//    vk::Format::eE5B9G9R9UfloatPack32,
//
//    vk::Format::eUndefined, // TODO: R8G8_B8G8_UNorm
//    vk::Format::eUndefined, // TODO: G8R8_G8B8_UNorm
//
//    vk::Format::eBc1RgbaUnormBlock,
//    vk::Format::eBc1RgbaUnormBlock,
//    vk::Format::eBc1RgbaSrgbBlock,
//
//    vk::Format::eBc2UnormBlock,
//    vk::Format::eBc2UnormBlock,
//    vk::Format::eBc2SrgbBlock,
//
//    vk::Format::eBc3UnormBlock,
//    vk::Format::eBc3UnormBlock,
//    vk::Format::eBc3SrgbBlock,
//
//    vk::Format::eBc4UnormBlock,
//    vk::Format::eBc4UnormBlock,
//    vk::Format::eBc4SnormBlock,
//
//    vk::Format::eBc5UnormBlock,
//    vk::Format::eBc5UnormBlock,
//    vk::Format::eBc5SnormBlock,
//
//    vk::Format::eB5G6R5UnormPack16,
//    vk::Format::eB5G5R5A1UnormPack16,
//
//    vk::Format::eB8G8R8A8Unorm,
//    vk::Format::eB8G8R8A8Unorm,
//    vk::Format::eUndefined, // TODO: R10G10B10_Xr_Bias_A2_UNorm
//
//    vk::Format::eB8G8R8A8Unorm,
//    vk::Format::eB8G8R8A8Srgb,
//    vk::Format::eB8G8R8A8Unorm,
//    vk::Format::eB8G8R8A8Srgb,
//
//   vk::Format::eBc6HUfloatBlock,
//   vk::Format::eBc6HUfloatBlock,
//   vk::Format::eBc6HSfloatBlock,
//
//   vk::Format::eBc7UnormBlock,
//   vk::Format::eBc7UnormBlock,
//   vk::Format::eBc7SrgbBlock,
//};
//
//PixelFormat VkFormatToPixelFormat[static_cast<int>(PixelFormat::MAX)] =
//{
//
//    PixelFormat::Unknown,
//    PixelFormat::Unknown,
//    PixelFormat::Unknown,
//    PixelFormat::Unknown,
//    PixelFormat::Unknown,
//    PixelFormat::Unknown,
//    PixelFormat::Unknown,
//    PixelFormat::Unknown,
//    PixelFormat::Unknown,
//    PixelFormat::R8_UNorm,
//    // 10
//    PixelFormat::R8_SNorm,
//    PixelFormat::Unknown,
//    PixelFormat::Unknown,
//    PixelFormat::R8_UInt,
//    PixelFormat::R8_SInt,
//    PixelFormat::Unknown,
//    PixelFormat::R8G8_UNorm,
//    PixelFormat::R8G8_SNorm,
//    PixelFormat::Unknown,
//    PixelFormat::Unknown,
//    // 20
//    PixelFormat::R8G8_UInt,
//    PixelFormat::R8G8_SInt,
//    PixelFormat::Unknown,
//    PixelFormat::Unknown,
//    PixelFormat::Unknown,
//    PixelFormat::Unknown,
//    PixelFormat::Unknown,
//    PixelFormat::Unknown,
//    PixelFormat::Unknown,
//    PixelFormat::Unknown,
//    // 30
//    PixelFormat::Unknown,
//    PixelFormat::Unknown,
//    PixelFormat::Unknown,
//    PixelFormat::Unknown,
//    PixelFormat::Unknown,
//    PixelFormat::Unknown,
//    PixelFormat::Unknown,
//    PixelFormat::R8G8B8A8_UNorm,
//    PixelFormat::R8G8B8A8_SNorm,
//    PixelFormat::Unknown,
//    // 40
//    PixelFormat::Unknown,
//    PixelFormat::R8G8B8A8_UInt,
//    PixelFormat::R8G8B8A8_SInt,
//    PixelFormat::Unknown,
//    PixelFormat::B8G8R8A8_UNorm,
//    PixelFormat::R8G8B8A8_SNorm,
//    PixelFormat::Unknown,
//    PixelFormat::Unknown,
//    PixelFormat::Unknown,
//    PixelFormat::Unknown,
//    PixelFormat::B8G8R8A8_UNorm_sRGB,
//    PixelFormat::Unknown,
//    PixelFormat::Unknown,
//    PixelFormat::Unknown,
//    PixelFormat::Unknown,
//    PixelFormat::Unknown,
//    PixelFormat::Unknown,
//    PixelFormat::Unknown,
//    PixelFormat::R16_UNorm,
//    PixelFormat::R16_SNorm,
//    PixelFormat::Unknown,
//    PixelFormat::Unknown,
//    PixelFormat::R16_UInt,
//    PixelFormat::R16_SInt,
//    PixelFormat::Unknown,
//    PixelFormat::R16G16_UNorm,
//    PixelFormat::R16G16_UNorm,
//    PixelFormat::R16G16_SNorm,
//    PixelFormat::Unknown,
//    PixelFormat::Unknown,
//    PixelFormat::R16G16_UInt,
//    PixelFormat::R16G16_SInt,
//    PixelFormat::Unknown,
//    PixelFormat::Unknown,
//    PixelFormat::Unknown,
//    PixelFormat::Unknown,
//    PixelFormat::Unknown,
//    PixelFormat::Unknown,
//    PixelFormat::Unknown,
//    PixelFormat::Unknown,
//    PixelFormat::Unknown,
//    PixelFormat::R16G16B16A16_UNorm,
//    PixelFormat::R16G16B16A16_SNorm,
//    PixelFormat::Unknown,
//    PixelFormat::Unknown,
//    PixelFormat::R16G16B16A16_UInt,
//    PixelFormat::R16G16B16A16_SInt,
//    PixelFormat::Unknown,
//    PixelFormat::R32_UInt,
//    PixelFormat::R32_SInt,
//    PixelFormat::Unknown,
//    PixelFormat::R32G32_UInt,
//    PixelFormat::R32G32_SInt,
//    PixelFormat::Unknown,
//    PixelFormat::R32G32B32_UInt,
//    PixelFormat::R32G32B32_SInt,
//    PixelFormat::Unknown,
//    PixelFormat::R32G32B32A32_UInt,
//    PixelFormat::R32G32B32A32_SInt,
//    PixelFormat::Unknown,
//};