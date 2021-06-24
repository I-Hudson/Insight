#pragma once

#include "Engine/Graphics/GPUResource.h"
#include "Engine/Graphics/GPUBuffer.h"

namespace Insight::Graphics
{
	struct GPUDynamicBufferDesc
	{
		GPUDynamicBufferDesc()
			: AligmentSize(0), MinBlockSize(0), Size(0)
			, Flags(GPUBufferFlags::NONE)
		{ }

		GPUDynamicBufferDesc(u64 aligment, u64 minBlockSize, u64 size, GPUBufferFlags flags)
			: AligmentSize(aligment), MinBlockSize(minBlockSize), Size(size)
			, Flags(flags)
		{ }
		u64 AligmentSize;
		u64 MinBlockSize;
		u64 Size;
		GPUBufferFlags Flags;

		static GPUDynamicBufferDesc Uniform(u64 aligment, u64 minUboSize, u64 size)
		{
			return GPUDynamicBufferDesc(aligment, minUboSize, size, GPUBufferFlags::UNIFORM);
		}

		static GPUDynamicBufferDesc Vertex(u64 size)
		{
			return GPUDynamicBufferDesc(0, 0, size, GPUBufferFlags::VERTEX | GPUBufferFlags::TRANSFER_DST);
		}

		static GPUDynamicBufferDesc Index(u64 size)
		{
			return GPUDynamicBufferDesc(0, 0, size, GPUBufferFlags::INDEX | GPUBufferFlags::TRANSFER_DST);
		}
	};

	/// <summary>
	/// A buffer which allows sub allocations.
	/// </summary>
	class GPUDynamicBuffer : public GPUResource
	{
	public:
		GPUDynamicBuffer();
		virtual ~GPUDynamicBuffer();

		static GPUDynamicBuffer* New();

		virtual void Init(GPUDynamicBufferDesc& desc) = 0;
		virtual GPUBuffer* Upload(void* data, u64 size) = 0;
		void Reset() 
		{
			m_currentOffset = 0;
			for (auto* b : m_subBuffers)
			{
				::Delete(b);
				b = nullptr;
			}
			m_subBuffers.clear();
		}

		//[GPUResource]
		virtual ResourceType GetResourceType() const override { return ResourceType::Buffer; }
		virtual ObjectType GetObjectType() const override { return ObjectType::Buffer; }
	
	protected:
		u64 m_currentOffset;
		GPUDynamicBufferDesc m_desc;
		std::vector<GPUBuffer*> m_subBuffers;
		void* m_mapped;
	};
}