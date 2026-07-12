#pragma once

#include "Core/TypeAlias.h"
#include "Graphics/Enums.h"

namespace Insight
{
    namespace Graphics
    {
			/// @brief Track the allocation within the RHI layer for this resource. This will contain information about the on device resource and its allocation,
			/// not the engine RHI_Resouce.
			class RHI_ResourceAllocation
			{
			public:
				RHI_ResourceAllocation();
				RHI_ResourceAllocation(u64 offset, u64 size, u64 stride, void* resource, void* memoryAllocation, BufferType bufferType);
				RHI_ResourceAllocation(u64 offset, u64 size, u64 stride, void* resource, void* memoryAllocation, TextureType textureType);
				~RHI_ResourceAllocation();

				u64 GetOffset() const	{ return m_offset; }
				u64 GetSize() const		{ return m_size; }
				u64 GetStride() const	{ return m_stride; }

				void* GetMemoryAllocation() const	{ return m_memoryAllocation; }
				void* GetResource() const			{ return m_resource; }
				const char* GetName() const			{ return m_name; }

				ResourceType GetResourceType() const	{ return m_resourceType; }
				BufferType GetBufferType() const		{ return m_bufferType; }
				TextureType GetTextureType() const		{ return m_textureType; }

				void SetName(const char* name);

			private:
				void FreeName();

			private:
				u64 m_offset = 0;
				u64 m_size = 0;
				u64 m_stride = 0;

				/// @brief Store a pointer to a plugin library struct/class for an allocation.
				void* m_resource = nullptr;
				char* m_name = nullptr;
				void* m_memoryAllocation = nullptr;

				ResourceType m_resourceType;

				union
				{
					struct
					{
						BufferType m_bufferType;
					};

					struct
					{
						TextureType m_textureType;
					};
				};
			};
        }
}