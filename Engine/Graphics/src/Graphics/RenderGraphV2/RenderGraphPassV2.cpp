#ifdef RENDERGRAPH_V2_ENABLED

#include "Graphics/RenderGraphV2/RenderGraphPassV2.h"
#include "Graphics/RenderGraphV2/RenderGraphV2.h"
#include "Graphics/PixelFormatExtensions.h"

#include "Algorithm/Vector.h"

namespace Insight
{
	namespace Graphics
	{
		RenderGraphPassBaseV2::RenderGraphPassBaseV2(RenderGraphV2* renderGraph, GPUQueue gpuQueue)
			: RenderGraphTask(renderGraph, gpuQueue)
		{ 
			ASSERT(RenderGraph);
		}

		RenderGraphPassBaseV2::~RenderGraphPassBaseV2()
		{ }

		RGBufferHandle RenderGraphPassBaseV2::CreateBuffer(std::string bufferName, RHI_BufferCreateInfo createInfo)
		{
			//const RGBufferHandle handle = RenderGraph->CreateBuffer(bufferName, createInfo);
			//if (Algorithm::VectorFindIf(m_bufferCreates, [&handle](const std::pair<RGBufferHandle, RHI_BufferCreateInfo>& pair)
			//	{
			//		return handle == pair.first;
			//	}) == m_bufferCreates.end())
			//{
			//	m_bufferCreates.push_back(std::make_pair(handle, createInfo));
			//}
			//return handle;
			return -1;
		}

		RGTextureHandle RenderGraphPassBaseV2::CreateTexture(std::string textureName, RHI_TextureInfo createInfo)
		{
			const RGTextureHandle handle = RenderGraph->CreateTexture(textureName, createInfo);
			if (Algorithm::VectorFindIf(m_textureCreates, [&handle](const std::pair<RGTextureHandle, RHI_TextureInfo>& pair)
				{
					return handle == pair.first;
				}) == m_textureCreates.end())
			{
				m_textureCreates.push_back(std::make_pair(handle, createInfo));
			}
			return handle;
		}

		void RenderGraphPassBaseV2::ReadBuffer(const RGBufferHandle handle)
		{
			if (std::find(m_bufferReads.begin(), m_bufferReads.end(), handle) == m_bufferReads.end())
			{
				m_bufferReads.push_back(handle);
			}
		}

		void RenderGraphPassBaseV2::WriteBuffer(const RGBufferHandle handle)
		{
			if (std::find(m_bufferWrites.begin(), m_bufferWrites.end(), handle) == m_bufferWrites.end())
			{
				m_bufferWrites.push_back(handle);
			}
		}

		void RenderGraphPassBaseV2::ReadTexture(const RGTextureHandle handle)
		{
			if (std::find(m_textureReads.begin(), m_textureReads.end(), handle) == m_textureReads.end())
			{
				m_textureReads.push_back(handle);
			}
		}

		void RenderGraphPassBaseV2::WriteTexture(const RGTextureHandle handle)
		{
			if (std::find(m_textureWrites.begin(),m_textureWrites.end(), handle) == m_textureWrites.end())
			{
				m_textureWrites.push_back(handle);
			}
		}

		RenderGraphGraphicsPassV2::RenderGraphGraphicsPassV2(RenderGraphV2* renderGraph
			, std::string passName, PreExecuteFunc setupFunc, ExecuteFunc executeFunc, PostExecuteFunc postFunc)
			: RenderGraphPassBaseV2(renderGraph, GPUQueue_Graphics)
			, m_preExecuteFunc(std::move(setupFunc))
			, m_executeFunc(std::move(executeFunc))
			, m_postExecuteFunc(std::move(postFunc))
		{
			m_passName = std::move(passName);
		}

		RenderGraphGraphicsPassV2::~RenderGraphGraphicsPassV2()
		{
		}

		void RenderGraphGraphicsPassV2::PreExecute()
		{
			m_preExecuteFunc(*this);
		}

		void RenderGraphGraphicsPassV2::Execute()
		{
			RenderGraphExecuteData data{ *RenderGraph, CmdList };
			m_executeFunc(data);
		}

		void RenderGraphGraphicsPassV2::PostExecute()
		{
			RenderGraphPostData data{ *RenderGraph, CmdList };
			m_postExecuteFunc(data);
		}

		void RenderGraphGraphicsPassV2::SetViewport(const u32 width, const u32 height)
		{
			m_viewport = glm::ivec2(width, height);
		}

		void RenderGraphGraphicsPassV2::SetScissor(const u32 width, const u32 height)
		{
			m_scissor = glm::ivec2(width, height);
		}

		void RenderGraphGraphicsPassV2::SetShader(const ShaderDesc shaderDesc)
		{
			m_shader = shaderDesc;
		}

		void RenderGraphGraphicsPassV2::SetPipeline(const PipelineStateObject pipelineStateObject)
		{
			m_PSO = pipelineStateObject;
		}

		void RenderGraphGraphicsPassV2::SetRenderpass(const RenderpassDescription renderpassDescription)
		{
			m_renderpassDescription = renderpassDescription;
		}

		void RenderGraphGraphicsPassV2::SetAsRenderToSwapchain()
		{
			m_renderOnTopOfSwapchain = true;
		}

		RGTextureHandle RenderGraphGraphicsPassV2::GetDepthSteniclTexture() const
		{
			for (size_t i = 0; i < m_textureReads.size(); ++i)
			{
				const RGTextureHandle handle = m_textureReads.at(i);
				RHI_Texture* texture = RenderGraph->GetRHITexture(handle);
				if (texture && PixelFormatExtensions::IsDepthStencil(texture->GetFormat()))
				{
					return handle;
				}
			}
			return -1;
		}
	}
}
#endif