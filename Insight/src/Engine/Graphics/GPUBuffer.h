#pragma once

#include "Engine/Platform/Platform.h"
#include "GPUBufferDescription.h"
#include "GPUResource.h"
#include "glm/glm.hpp"

struct Vertex
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec4 Colour;
	glm::vec2 UV1;
};


class IS_API GPUBuffer : public GPUResource
{
public:
	SharedPtr<GPUBuffer> Create();

	GPUBuffer();
	virtual ~GPUBuffer() override;

	virtual ResourceType GetResourceType() const { return ResourceType::Buffer; }

	bool Init(const GPUBufferDescription& desc);

	void SetData(void* data, const U64& size);
	void GetData(std::vector<Byte>& data);

	FORCE_INLINE bool IsAllocated() const { return m_desc.Size > 0; }
	FORCEINLINE U32 GetSize() const { return m_desc.Size; }
	FORCEINLINE U32 GetStride() const { return m_desc.Stride; }
	FORCEINLINE GPUBufferFlags GetFlags() const { return m_desc.Flags; }
	FORCEINLINE U32 GetElementsCount() const { ASSERT(GetStride() > 0); return m_desc.Size / m_desc.Stride; }
	FORCEINLINE PixelFormat GetFromat() const { return m_desc.Format; }
	FORCEINLINE GPUResourceUsage GetUsage() const { return m_desc.Usage; }

	FORCE_INLINE bool IsStaging() const { return m_desc.Usage == GPUResourceUsage::StagingUpload || m_desc.Usage == GPUResourceUsage::StagingReadback; }
	FORCE_INLINE bool IsDynamic() const { return m_desc.Usage == GPUResourceUsage::Dynamic; }
	FORCE_INLINE bool IsShaderResource() const { return m_desc.IsShaderResource(); }
	FORCE_INLINE bool IsUnorderedAccess() const { return m_desc.IsUnorderedAccess(); }
	FORCE_INLINE const GPUBufferDescription& GetDescription() const { return m_desc; }

	bool Resize(U32 newSize);

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
	GPUBufferDescription m_desc;
};