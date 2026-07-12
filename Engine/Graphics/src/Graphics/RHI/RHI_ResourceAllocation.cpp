#include "Graphics/RHI/RHI_ResourceAllocation.h"

#include "Core/Memory.h"
#include "Platforms/Platform.h"

namespace Insight
{
    namespace Graphics
    {
        RHI_ResourceAllocation::RHI_ResourceAllocation()
        { }

        RHI_ResourceAllocation::RHI_ResourceAllocation(u64 offset, u64 size, u64 stride, void* resource, void* memoryAllocation, BufferType bufferType)
            : m_offset(offset)
            , m_size(size)
            , m_stride(stride)
            , m_resource(resource)
            , m_memoryAllocation(memoryAllocation)
            , m_resourceType(ResourceType::Buffer)
            , m_bufferType(bufferType)
        { }

        RHI_ResourceAllocation::RHI_ResourceAllocation(u64 offset, u64 size, u64 stride, void* resource, void* memoryAllocation, TextureType textureType)
            : m_offset(offset)
            , m_size(size)
            , m_stride(stride)
            , m_resource(resource)
            , m_memoryAllocation(memoryAllocation)
            , m_resourceType(ResourceType::Texture)
            , m_textureType(textureType)
        { }

        RHI_ResourceAllocation::~RHI_ResourceAllocation()
        {
            FreeName();
        }

        void RHI_ResourceAllocation::SetName(const char* name)
        {
            FreeName();

            if (m_name == nullptr)
            {
                const u64 nameLength = strlen(name);
                m_name = NewArray<char>(nameLength + 1ull);
                Platform::MemCopy(m_name, name, nameLength);
                m_name[nameLength] = '\0';
            }
        }

        void RHI_ResourceAllocation::FreeName()
        {
            if (m_name != nullptr)
            {
                DeleteArray(m_name);
                m_name = nullptr;
            }
        }
    }
}