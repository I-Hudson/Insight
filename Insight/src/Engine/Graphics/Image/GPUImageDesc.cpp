#include "ispch.h"
#include "Engine/Graphics/Image/GPUImageDesc.h"


namespace Insight::Graphics
{

    GPUImageDesc GPUImageDesc::Image2D(u32 width, u32 height, u32 depth, u32 levels, SampleLevel samples, u32 layers,
                                       ImageDomain domain, ImageLayout initLayout, ImageLayout finalLayout, ImageUsageFlags usageFlags, ImageMiscFlags miscFlags,
                                       ImageCreateFlags createFlags, PixelFormat format, ImageType type, ImageUsageType imageUsageType,
                                       void* data)
    {
        return GPUImageDesc(width, height, depth, levels, samples, layers, domain, initLayout, finalLayout, usageFlags, miscFlags, createFlags, format, type, imageUsageType, data);
    }
}