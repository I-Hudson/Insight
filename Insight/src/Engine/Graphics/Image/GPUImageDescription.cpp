#include "ispch.h"
#include "Engine/Graphics/Image/GPUImageDescription.h"
#include "Engine/Graphics/PixelFormatExtensions.h"

GPUImageDescription GPUImageDescription::Image2D(U32 width, U32 height, U32 depth, U32 levels, U32 samples, U32 layers, ImageLayout initLayout, ImageUsageFlags usage, PixelFormat format)
{
    GPUImageDescription desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.Depth = depth;
    desc.Levels = levels;
    desc.Samples = samples;
    desc.Layers = layers;
    desc.InitLayout = initLayout;
    desc.Usage = usage;
    desc.Format = format;
    desc.Type = ImageType::Image_2D;
    return desc;
}

GPUImageDescription GPUImageDescription::RenderTarget(U32 width, U32 height, PixelFormat format)
{
    GPUImageDescription desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.Depth = 1;
    desc.Depth = 1;
    desc.Levels = 1;
    desc.Samples = 1;
    desc.Layers = 1;
    desc.InitLayout = PixelFormatExtensions::IsDepthStencil(format) ? 
        ImageLayout::Depth_Stencil_Attachment : 
        ImageLayout::Color_Attachment;
    desc.Usage = (PixelFormatExtensions::IsDepthStencil(format) ? (U32)ImageUsageFlagsBits::Depth_Stencil_Attachment :
                 (U32)ImageUsageFlagsBits::Color_Attachment) | 
                 (U32)ImageUsageFlagsBits::Transfer_Src | (U32)ImageUsageFlagsBits::Transfer_Dst;
    desc.Format = format;
    desc.Type = ImageType::Image_2D;
    return desc;
}

GPUImageDescription GPUImageDescription::TransientRenderTarget(U32 width, U32 height, PixelFormat format)
{
    GPUImageDescription desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.Depth = 1;
    desc.Levels = 1;
    desc.Samples = 1;
    desc.Layers = 1;
    desc.Domain = ImageDomain::Transient;
    desc.InitLayout = ImageLayout::Undefined;
    desc.Usage = (PixelFormatExtensions::IsDepthStencil(format) ? (U32)ImageUsageFlagsBits::Depth_Stencil_Attachment :
        (U32)ImageUsageFlagsBits::Color_Attachment) |
        (U32)ImageUsageFlagsBits::Input_Attachment;
    desc.Format = format;
    desc.Type = ImageType::Image_2D;
    return desc;
}
