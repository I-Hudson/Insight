#pragma once
#include "Engine/Core/Common.h"
#include "Enums.h"
#include "PixelFormat.h"

namespace Insight::Graphics
{
    class GPUDynamicBuffer;

    /// <summary>
    /// The GPU buffer usage flags.
    /// </summary>
    enum GPUBufferFlags
    {
        NONE                    = 1 << 0,
        TRANSFER_SRC            = 1 << 1,
        TRANSFER_DST            = 1 << 2,
        UNIFORM_TEXEL           = 1 << 3,
        STORAGE_TEXEL           = 1 << 4,
        UNIFORM                 = 1 << 5,
        STORAGE                 = 1 << 6,
        INDEX                   = 1 << 7,
        VERTEX                  = 1 << 8,
        INDIRECT                = 1 << 9,
        SHADER_DEVICE_ADDRESS   = 1 << 10,
    };

    DECLARE_ENUM_OPERATORS(GPUBufferFlags);

    struct GPUBufferSubAllocDesc
    {
        enum
        {
            Unsed = 1,
            Used = 0
        };

        u64 Size;
        u64 Begin;
        u32 State = Unsed;
        GPUDynamicBuffer* ParentBuffer;
    };

    struct IS_API GPUBufferDesc
    {
        /// <summary>
        /// The buffer total size in bytes.
        /// </summary>
        u32 Size;

        /// <summary>
        /// The buffer structure stride (size in bytes per element).
        /// </summary>
        u32 Stride;

        /// <summary>
        /// The buffer flags.
        /// </summary>
        GPUBufferFlags Flags;

        /// <summary>
        /// The format of the data in a buffer.
        /// </summary>
        PixelFormat Format;

        /// <summary>
        /// The pointer to location of initial resource data. Null if not used.
        /// </summary>
        const void* InitData;

        /// <summary>
        /// Is this buffer a sub allocation of a bigger buffer. Mainly used in rendering.
        /// </summary>
        GPUBufferSubAllocDesc SubAlloc;

    public:

        /// <summary>
        /// Gets the number elements in the buffer.
        /// </summary>
        FORCE_INLINE u32 GetElementsCount() const
        {
            return Stride > 0 ? Size / Stride : 0;
        }

        /// <summary>
        /// Clears description.
        /// </summary>
        void Clear();

        static GPUBufferDesc Buffer(u32 size, GPUBufferFlags flags, PixelFormat format = PixelFormat::Unknown, const void* initData = nullptr, u32 stride = 0);
        static GPUBufferDesc SubBuffer(GPUDynamicBuffer* parentBuffer, u64 begin, u64 size, GPUBufferFlags additionalFlags);

        static GPUBufferDesc Uniform(u32 elementStride, u32 elementsCount, void* data)
        {
            return Buffer(elementsCount * elementStride, GPUBufferFlags::UNIFORM, PixelFormat::Unknown, data, elementStride);
        }

        static GPUBufferDesc Uniform(u32 elementStride, u32 elementsCount)
        {
            return Buffer(elementsCount * elementStride, GPUBufferFlags::UNIFORM, PixelFormat::Unknown, nullptr, elementStride);
        }

        static GPUBufferDesc Vertex(u32 elementStride, u32 elementsCount, void* data)
        {
            return Buffer(elementsCount * elementStride, GPUBufferFlags::VERTEX | GPUBufferFlags::TRANSFER_DST, PixelFormat::Unknown, data, elementStride);
        }

        static GPUBufferDesc Vertex(u32 elementStride, u32 elementsCount)
        {
            return Buffer(elementsCount * elementStride, GPUBufferFlags::VERTEX | GPUBufferFlags::TRANSFER_DST, PixelFormat::Unknown, nullptr, elementStride);
        }

        static GPUBufferDesc Index(u32 elementStride, u32 elementsCount, void* data)
        {
            const auto format = elementStride == 4 ? PixelFormat::R32_UInt : PixelFormat::R16_UInt;
            return Buffer(elementsCount * elementStride, GPUBufferFlags::INDEX | GPUBufferFlags::TRANSFER_DST, format, data, elementStride);
        }

        static GPUBufferDesc Index(u32 elementStride, u32 elementsCount)
        {
            const auto format = elementStride == 4 ? PixelFormat::R32_UInt : PixelFormat::R16_UInt;
            return Buffer(elementsCount * elementStride, GPUBufferFlags::INDEX | GPUBufferFlags::TRANSFER_DST, format, nullptr, elementStride);
        }

        static GPUBufferDesc RawUpload(u32 size, void* data)
        {
            return Buffer(size, GPUBufferFlags::TRANSFER_SRC, PixelFormat::Unknown, data, size);
        }

        static GPUBufferDesc SubAllocation(GPUDynamicBuffer* buffer, u64 begin, u64 size, GPUBufferFlags additionalFlags)
        {
            return SubBuffer(buffer, begin, size, additionalFlags);
        }

        /// <summary>
       /// Gets the staging upload description for this instance.
       /// </summary>
       /// <returns>A staging buffer description</returns>
        GPUBufferDesc ToStagingUpload() const
        {
            auto desc = *this;
            desc.Flags |= GPUBufferFlags::TRANSFER_SRC;
            desc.InitData = nullptr;
            return desc;
        }

        /// <summary>
        /// Gets the staging readback description for this instance.
        /// </summary>
        /// <returns>A staging buffer description</returns>
        GPUBufferDesc ToStagingReadback() const
        {
            auto desc = *this;
            desc.Flags |= GPUBufferFlags::TRANSFER_DST;
            desc.InitData = nullptr;
            return desc;
        }

        /// <summary>
        /// Compares with other instance of GPUBufferDesc
        /// </summary>
        /// <param name="other">The other object to compare.</param>
        /// <returns>True if objects are the same, otherwise false.</returns>
        bool Equals(const GPUBufferDesc& other) const;

        /// <summary>
        /// Implements the operator ==.
        /// </summary>
        /// <param name="other">The other description.</param>
        /// <returns>The result of the operator.</returns>
        FORCE_INLINE bool operator==(const GPUBufferDesc& other) const
        {
            return Equals(other);
        }

        /// <summary>
        /// Implements the operator !=.
        /// </summary>
        /// <param name="other">The other description.</param>
        /// <returns>The result of the operator.</returns>
        FORCE_INLINE bool operator!=(const GPUBufferDesc& other) const
        {
            return !Equals(other);
        }

        std::string ToString() const;
    };

    u32 GetHash(const GPUBufferDesc& key);
}