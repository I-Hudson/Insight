#pragma once

#include "Engine/Platform/Platform.h"
#include "GPUBufferDesc.h"
#include "GPUResource.h"
#include "glm/glm.hpp"

struct Vertex
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec4 Colour;
	glm::vec2 UV1;
	glm::ivec4 JointIndices;
	glm::vec4 JointWeight;
#ifdef IS_MESH_BATCHING_EXT
	u32 VIndex;
#endif
};

namespace Insight::Graphics
{
	class GPUBuffer : public GPUResource
	{
	public:

		GPUBuffer();
		virtual ~GPUBuffer() override;

		static GPUBuffer* New();
		virtual void Init(const GPUBufferDesc& desc) = 0;

		void SetData(void const* data, const u64& size);
		void GetData(std::vector<Byte>& data);

		FORCE_INLINE bool IsAllocated() const { return m_desc.Size > 0; }
		FORCE_INLINE u32 GetSize() const { return m_desc.Size; }
		FORCE_INLINE u32 GetStride() const { return m_desc.Stride; }
		FORCE_INLINE GPUBufferFlags GetFlags() const { return m_desc.Flags; }
		FORCE_INLINE u32 GetElementsCount() const { ASSERT(GetStride() > 0); return m_desc.Size / m_desc.Stride; }
		FORCE_INLINE PixelFormat GetFromat() const { return m_desc.Format; }

		FORCE_INLINE bool IsStaging() const { return m_desc.Flags == GPUBufferFlags::TRANSFER_SRC || m_desc.Flags == GPUBufferFlags::TRANSFER_DST; }
		FORCE_INLINE bool IsSubAllocation() const { return m_desc.SubAlloc.State != GPUBufferSubAllocDesc::Unsed; }
		FORCE_INLINE const GPUBufferDesc& GetDesc() const { return m_desc; }

		void Resize(u32 newSize);

		// [GPUResource]
		virtual ResourceType GetResourceType() const { return ResourceType::Buffer; }
		virtual ObjectType GetObjectType() const { return ObjectType::Buffer; }

	protected:
		virtual void* Map() = 0;
		virtual void UnMap() = 0;

		bool IsMapped() const { return m_mappedData != nullptr; }

		/// <summary>
		/// Upload the bufffer data to the GPU.
		/// </summary>
		virtual void Upload(void* mapped, void const* data, u64 const& size) = 0;
		/// <summary>
		/// Get the buffer data from the GPU. This is slow and should not really but used.
		/// </summary>
		virtual void Download(std::vector<u8>& data, void const* mapped) = 0;

	protected:
		GPUBufferDesc m_desc;
		void* m_mappedData;
		bool m_mustBeMapped;
	};
}