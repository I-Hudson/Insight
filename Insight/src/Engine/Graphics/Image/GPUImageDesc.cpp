#include "ispch.h"
#include "Engine/Graphics/Image/GPUImageDesc.h"


namespace Insight::Graphics
{

    GPUImageDesc GPUImageDesc::Image2D(u32 width, u32 height, u32 depth, u32 levels, u32 samples, u32 layers,
                                       ImageDomain domain, ImageLayout initLayout, ImageUsageFlags usageFlags, ImageMiscFlags miscFlags,
                                       ImageCreateFlags createFlags, PixelFormat format, ImageType type, ImageUsageType imageUsageType,
                                       void* data)
    {
        return GPUImageDesc(width, height, depth, levels, samples, layers, domain, initLayout, usageFlags, miscFlags, createFlags, format, type, imageUsageType, data);
    }
}