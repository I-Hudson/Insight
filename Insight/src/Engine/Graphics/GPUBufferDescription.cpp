#include "ispch.h"
#include "Engine/Graphics/GPUBufferDescription.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Graphics/PixelFormatExtensions.h"

void GPUBufferDescription::Clear()
{
    Platform::MemClear(this, sizeof(GPUBufferDescription));
}

GPUBufferDescription GPUBufferDescription::Buffer(U32 size, GPUBufferFlags flags, PixelFormat format, const void* initData, U32 stride, GPUResourceUsage usage)
{
    GPUBufferDescription desc;
    desc.Size = size;
    desc.Stride = stride;
    desc.Flags = flags;
    desc.Format = format;
    desc.InitData = initData;
    desc.Usage = usage;
    return desc;
}

GPUBufferDescription GPUBufferDescription::Typed(I32 count, PixelFormat viewFormat, bool isUnorderedAccess, GPUResourceUsage usage)
{
    auto bufferFlags = GPUBufferFlags::ShaderResource;
    if (isUnorderedAccess)
        bufferFlags |= GPUBufferFlags::UnorderedAccess;
    const auto stride = PixelFormatExtensions::SizeInBytes(viewFormat);
    return Buffer(count * stride, bufferFlags, viewFormat, nullptr, stride, usage);
}

GPUBufferDescription GPUBufferDescription::Typed(const void* data, I32 count, PixelFormat viewFormat, bool isUnorderedAccess, GPUResourceUsage usage)
{
    auto bufferFlags = GPUBufferFlags::ShaderResource;
    if (isUnorderedAccess)
        bufferFlags |= GPUBufferFlags::UnorderedAccess;
    const auto stride = PixelFormatExtensions::SizeInBytes(viewFormat);
    return Buffer(count * stride, bufferFlags, viewFormat, data, stride, usage);
}

bool GPUBufferDescription::Equals(const GPUBufferDescription& other) const
{
    return Size == other.Size
        && Stride == other.Stride
        && Flags == other.Flags
        && Format == other.Format
        && Usage == other.Usage
        && InitData == other.InitData;
}

std::string GPUBufferDescription::ToString() const
{
    // TODO: add tool to Format to string

std::string flags;
    if (Flags == GPUBufferFlags::None)
    {
        flags = "None";
    }
    else
    {
        // TODO: create tool to auto convert flag enums to string

#define CONVERT_FLAGS_FLAGS_2_STR(value) if(Flags & GPUBufferFlags::value) { if (!flags.empty()) flags += '|'; flags += #value; }
        CONVERT_FLAGS_FLAGS_2_STR(ShaderResource);
        CONVERT_FLAGS_FLAGS_2_STR(VertexBuffer);
        CONVERT_FLAGS_FLAGS_2_STR(IndexBuffer);
        CONVERT_FLAGS_FLAGS_2_STR(UnorderedAccess);
        CONVERT_FLAGS_FLAGS_2_STR(Append);
        CONVERT_FLAGS_FLAGS_2_STR(Counter);
        CONVERT_FLAGS_FLAGS_2_STR(Argument);
        CONVERT_FLAGS_FLAGS_2_STR(Structured);
#undef CONVERT_FLAGS_FLAGS_2_STR
    }

    return StringFormat("Size: {0}, Stride: {1}, Flags: {2}, Format: {3}, Usage: {4}",
        Size,
        Stride,
        flags,
        (I32)Format,
        (I32)Usage);
}

U32 GetHash(const GPUBufferDescription& key)
{
    U32 hashCode = key.Size;
    hashCode = (hashCode * 397) ^ key.Stride;
    hashCode = (hashCode * 397) ^ (U32)key.Flags;
    hashCode = (hashCode * 397) ^ (U32)key.Format;
    hashCode = (hashCode * 397) ^ (U32)key.Usage;
    return hashCode;
}