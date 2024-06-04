#ifdef RENDERGRAPH_V2_ENABLED

#include "Graphics/RenderGraphV2/RenderGraphPassV2.h"
#include "Graphics/RenderGraphV2/RenderGraphV2.h"

#include "Algorithm/Vector.h"

namespace Insight
{
	namespace Graphics
	{
		RenderGraphPassV2::RenderGraphPassV2(RenderGraphV2* renderGraph, GPUQueue gpuQueue, const std::string name)
			: RenderGraph(renderGraph)
			, GpuQueue(gpuQueue)
			, PassName(name)
		{ }

		RenderGraphPassV2::~RenderGraphPassV2()
		{ }

		RenderGraphPassV2& RenderGraphPassV2::AddBufferWrite(const std::string_view name, const RHI_BufferCreateInfo createInfo)
		{
			const RGResourceHandle handle = RenderGraph->CreateBuffer(name.data());
			if (std::find(BufferWrites.begin(), BufferWrites.end(), handle) == BufferWrites.end())
			{
				BufferWrites.push_back(std::make_pair(handle, createInfo));
			}
			return *this;
		}
		RenderGraphPassV2& RenderGraphPassV2::AddTextureWrite(const std::string_view name, const RHI_TextureInfo createInfo)
		{
			const RGResourceHandle handle = RenderGraph->CreateTexture(name.data());
			if (std::find(TextureWrites.begin(), TextureWrites.end(), handle) == TextureWrites.end())
			{
				TextureWrites.push_back(std::make_pair(handle, createInfo));
			}
			return *this;
		}

		RenderGraphPassV2& RenderGraphPassV2::AddBufferRead(const std::string_view name)
		{
			const RGResourceHandle handle = RenderGraph->GetBuffer(name.data());
			if (std::find(BufferReads.begin(), BufferReads.end(), handle) == BufferReads.end())
			{
				BufferReads.push_back(handle);
			}
			return *this;
		}
		RenderGraphPassV2& RenderGraphPassV2::AddTextureRead(const std::string_view name)
		{
			const RGResourceHandle handle = RenderGraph->GetBuffer(name.data());
			if (std::find(TextureReads.begin(), TextureReads.end(), handle) == TextureReads.end())
			{
				TextureReads.push_back(handle);
			}
			return *this;
		}

		RenderGraphPassV2& RenderGraphPassV2::SetExecuteFunc(RenderGraphPassV2::ExecuteFunc executeFunc)
		{
			ExecuteFuncCallback = std::move(executeFunc);
		}

		bool RenderGraphPassV2::IsBufferWritten(const std::string_view name) const
		{
			const RGResourceHandle handle = RenderGraph->GetBuffer(name.data());
			return IsBufferWritten(handle);
		}
		bool RenderGraphPassV2::IsBufferWritten(const RGResourceHandle handle) const
		{
			return Algorithm::VectorFindIf(BufferWrites, [handle](const std::pair<RGResourceHandle, RHI_BufferCreateInfo>& pair)
				{
					return handle == pair.first;
				}) != BufferWrites.end();
		}

		bool RenderGraphPassV2::IsTextureWritten(const std::string_view name) const
		{
			const RGResourceHandle handle = RenderGraph->GetTexture(name.data());
			return IsTextureWritten(handle);
		}
		bool RenderGraphPassV2::IsTextureWritten(const RGResourceHandle handle) const
		{
			return Algorithm::VectorFindIf(TextureWrites, [handle](const std::pair<RGResourceHandle, RHI_TextureInfo>& pair)
				{
					return handle == pair.first;
				}) != TextureWrites.end();
		}

		bool RenderGraphPassV2::IsBufferRead(const std::string_view name) const
		{
			const RGResourceHandle handle = RenderGraph->GetBuffer(name.data());
			return IsBufferRead(handle);
		}
		bool RenderGraphPassV2::IsBufferRead(const RGResourceHandle handle) const
		{
			return Algorithm::VectorFind(BufferReads, handle) != BufferReads.end();
		}

		bool RenderGraphPassV2::IsTextureRead(const std::string_view name) const
		{
			const RGResourceHandle handle = RenderGraph->GetTexture(name.data());
			return IsTextureRead(handle);
		}
		bool RenderGraphPassV2::IsTextureRead(const RGResourceHandle handle) const
		{
			return Algorithm::VectorFind(TextureReads, handle) != TextureReads.end();
		}
	}
}
#endif