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

		void SetData(void const* data, const U64& size);
		void GetData(std::vector<Byte>& data);

		FORCE_INLINE bool IsAllocated() const { return m_desc.Size > 0; }
		FORCE_INLINE U32 GetSize() const { return m_desc.Size; }
		FORCE_INLINE U32 GetStride() const { return m_desc.Stride; }
		FORCE_INLINE GPUBufferFlags GetFlags() const { return m_desc.Flags; }
		FORCE_INLINE U32 GetElementsCount() const { ASSERT(GetStride() > 0); return m_desc.Size / m_desc.Stride; }
		FORCE_INLINE PixelFormat GetFromat() const { return m_desc.Format; }

		FORCE_INLINE bool IsStaging() const { return m_desc.Flags == GPUBufferFlags::TRANSFER_SRC || m_desc.Flags == GPUBufferFlags::TRANSFER_DST; }
		FORCE_INLINE const GPUBufferDesc& GetDesc() const { return m_desc; }

		void Resize(U32 newSize);
		bool IsMapped() const { return m_mappedData != nullptr; }

		// [GPUResource]
		virtual ResourceType GetResourceType() const { return ResourceType::Buffer; }
		virtual ObjectType GetObjectType() const { return ObjectType::Buffer; }

	protected:
		virtual void* Map(GPUResourceMapMode mapMode) = 0;
		virtual void UnMap() = 0;

		/// <summary>
		/// Upload the bufffer data to the GPU.
		/// </summary>
		virtual void Upload() = 0;
		/// <summary>
		/// Get the buffer data from the GPU. This is slow and should not really but used.
		/// </summary>
		virtual void Download() = 0;

	protected:
		GPUBufferDesc m_desc;
		void* m_mappedData;
	};
}