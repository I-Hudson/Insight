#pragma once

#ifdef RENDERGRAPH_V2_ENABLED

#include "Graphics/Enums.h"

#include "Graphics/ShaderDesc.h"
#include "Graphics/PipelineStateObject.h"
#include "Graphics/RHI/RHI_Buffer.h"
#include "Graphics/RHI/RHI_Texture.h"
#include "Graphics/RHI/RHI_Renderpass.h"


#include "Graphics/PipelineBarrier.h"

#include <glm/ext/vector_int2.hpp>

#include <functional>
#include <vector>


namespace Insight
{
	namespace Graphics
	{
		class RenderContext;
		class RHI_CommandList;

		class RenderGraphV2;

		using RGResourceHandle = u32;

		class IS_GRAPHICS RenderGraphPassV2
		{
			using ExecuteFunc = std::function<void(RenderGraphV2*, RHI_CommandList*)>;
		public:
			RenderGraphPassV2(RenderGraphV2* renderGraph, GPUQueue gpuQueue, const std::string name);
			~RenderGraphPassV2();

			RenderGraphPassV2& AddBufferWrite(const std::string_view name, const RHI_BufferCreateInfo createInfo);
			RenderGraphPassV2& AddTextureWrite(const std::string_view name, const RHI_TextureInfo createInfo);

			RenderGraphPassV2& AddBufferRead(const std::string_view name);
			RenderGraphPassV2& AddTextureRead(const std::string_view name);

			RenderGraphPassV2& SetExecuteFunc(ExecuteFunc executeFunc);

			std::string_view GetPassName() const { return PassName; }

			bool IsBufferWritten(const std::string_view name) const;
			bool IsBufferWritten(const RGResourceHandle handle) const;
			bool IsTextureWritten(const std::string_view name) const;
			bool IsTextureWritten(const RGResourceHandle handle) const;

			bool IsBufferRead(const std::string_view name) const;
			bool IsBufferRead(const RGResourceHandle handle) const;
			bool IsTextureRead(const std::string_view name) const;
			bool IsTextureRead(const RGResourceHandle handle) const;

		public:
			ExecuteFunc ExecuteFuncCallback;
			std::string PassName;
			RenderGraphV2* RenderGraph = nullptr;
			GPUQueue GpuQueue;

			std::vector<std::pair<RGResourceHandle, RHI_BufferCreateInfo>> BufferWrites;
			std::vector<std::pair<RGResourceHandle, RHI_TextureInfo>> TextureWrites;

			std::vector<RGResourceHandle> BufferReads;
			std::vector<RGResourceHandle> TextureReads;

			std::vector<PipelineBarrier> PipelineBarriers;

			friend class RenderGraphV2;
		};
	}
}
#endif