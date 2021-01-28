#pragma once

#include "Engine/Core/InsightAlias.h"
#include "VulkanHeaders.h"

enum class SpirvShaderResourceType
{
    Unknown = 0,
    ConstantBuffer = 1,
    Buffer = 2,
    Sampler = 3,
    Texture1D = 4,
    Texture2D = 5,
    Texture3D = 6,
    TextureCube = 7,
    Texture1DArray = 8,
    Texture2DArray = 9,
};

enum class SpirvShaderResourceBindingType : Byte
{
    INVALID = 0,
    CB = 1,
    SAMPLER = 2,
    SRV = 3,
    UAV = 4,
    MAX
};

struct SpirvShaderDescriptorInfo
{
    enum
    {
        MaxDescriptors = 64,
    };

    /// <summary>
    /// A single descriptor data.
    /// </summary>
    struct Descriptor
    {
        /// <summary>
        /// The binding slot (the descriptors slot).
        /// </summary>
        Byte Binding;

        /// <summary>
        /// The layout slot (the descriptors set slot).
        /// </summary>
        Byte Set;

        /// <summary>
        /// The input slot (the pipeline slot).
        /// </summary>
        Byte Slot;

        /// <summary>
        /// The resource binding type (the graphics pipeline abstraction binding layer type).
        /// </summary>
        SpirvShaderResourceBindingType BindingType;

        /// <summary>
        /// The Vulkan descriptor type.
        /// </summary>
        VkDescriptorType DescriptorType;

        /// <summary>
        /// The resource type.
        /// </summary>
        SpirvShaderResourceType ResourceType;
    };

    U64 ImageInfosCount;
    U64 BufferInfosCount;
    U32 DescriptorTypesCount;
    Descriptor DescriptorTypes[MaxDescriptors];
};

struct SpirvShaderHeader
{
    enum class Types
    {
        /// <summary>
        /// The raw SPIR-V Byte code.
        /// </summary>
        Raw = 0,
    };

    /// <summary>
    /// The data type.
    /// </summary>
    Types Type;

    /// <summary>
    /// The shader descriptors usage information.
    /// </summary>
    SpirvShaderDescriptorInfo DescriptorInfo;

    // .. rest is just a actual data array
};
