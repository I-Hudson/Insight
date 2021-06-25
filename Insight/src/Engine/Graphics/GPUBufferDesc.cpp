
#include "Engine/Graphics/GPUBufferDesc.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Graphics/PixelFormatExtensions.h"

namespace Insight::Graphics
{
    void GPUBufferDesc::Clear()
    {
        Platform::MemClear(this, sizeof(GPUBufferDesc));
    }

    GPUBufferDesc GPUBufferDesc::Buffer(u32 size, GPUBufferFlags flags, PixelFormat format, const void* initData, u32 stride)
    {
        GPUBufferDesc desc;
        desc.Size = size;
        desc.Stride = stride;
        desc.Flags = flags;
        desc.Format = format;
        desc.InitData = initData;
        return desc;
    }

    GPUBufferDesc GPUBufferDesc::SubBuffer(GPUDynamicBuffer* parentBuffer, u64 begin, u64 size, GPUBufferFlags additionalFlags)
    {
        GPUBufferDesc desc = {};
        desc.SubAlloc.Size = size;
        desc.SubAlloc.Begin = begin;
        desc.SubAlloc.ParentBuffer = parentBuffer;
        desc.SubAlloc.State = GPUBufferSubAllocDesc::Used;
        desc.Flags = additionalFlags;
        return desc;
    }


    bool GPUBufferDesc::Equals(const GPUBufferDesc& other) const
    {
        return Size == other.Size
            && Stride == other.Stride
            && Flags == other.Flags
            && Format == other.Format
            && InitData == other.InitData;
    }

    std::string GPUBufferDesc::ToString() const
    {
        // TODO: add tool to Format to string

        std::string flags;
        if (Flags == GPUBufferFlags::NONE)
        {
            flags = "None";
        }
        else
        {
            // TODO: create tool to auto convert flag enums to string

#define CONVERT_FLAGS_FLAGS_2_STR(value) if(Flags & GPUBufferFlags::value) { if (!flags.empty()) flags += '|'; flags += #value; }
            CONVERT_FLAGS_FLAGS_2_STR(TRANSFER_SRC);
            CONVERT_FLAGS_FLAGS_2_STR(TRANSFER_DST);
            CONVERT_FLAGS_FLAGS_2_STR(UNIFORM_TEXEL);
            CONVERT_FLAGS_FLAGS_2_STR(STORAGE_TEXEL);
            CONVERT_FLAGS_FLAGS_2_STR(UNIFORM);
            CONVERT_FLAGS_FLAGS_2_STR(STORAGE);
            CONVERT_FLAGS_FLAGS_2_STR(INDEX);
            CONVERT_FLAGS_FLAGS_2_STR(VERTEX);
            CONVERT_FLAGS_FLAGS_2_STR(INDIRECT);
            CONVERT_FLAGS_FLAGS_2_STR(SHADER_DEVICE_ADDRESS);
#undef CONVERT_FLAGS_FLAGS_2_STR
        }

        return StringFormat("Size: {0}, Stride: {1}, Flags: {2}, Format: {3}, Usage: {4}",
                            Size,
                            Stride,
                            flags,
                            (I32)Format);
    }

    u32 GetHash(const GPUBufferDesc& key)
    {
        u32 hashCode = key.Size;
        hashCode = (hashCode * 397) ^ key.Stride;
        hashCode = (hashCode * 397) ^ (u32)key.Flags;
        hashCode = (hashCode * 397) ^ (u32)key.Format;
        return hashCode;
    }
}