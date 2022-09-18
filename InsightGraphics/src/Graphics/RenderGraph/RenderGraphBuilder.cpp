#ifdef RENDER_GRAPH_ENABLED

#include "Graphics/RenderGraph/RenderGraphBuilder.h"
#include "Graphics/RenderGraph/RenderGraphPass.h"
#include "Graphics/RenderGraph/RenderGraph.h"
#include "Platform/Platform.h"

namespace Insight
{
	namespace Graphics
	{
		RenderGraphBuilder::RenderGraphBuilder(RenderGraph* rg)
			: m_rg(rg)
		{ }

		void RenderGraphBuilder::SetPass(RenderGraphPassBase* pass)
		{
			m_pass = pass;
			ASSERT(m_pass);
		}

		void RenderGraphBuilder::SetAsRenderToSwapchain()
		{
			m_pass->m_swapchainPass = true;
		}

		void RenderGraphBuilder::SetRenderpass(RenderpassDescription description)
		{
			m_pass->m_renderpassDescription = std::move(description);
		}

		RGTextureHandle RenderGraphBuilder::CreateTexture(std::wstring textureName, RHI_TextureCreateInfo desc)
		{
			RGTextureHandle handle = m_rg->CreateTexture(textureName, desc);
			if (std::find_if(m_pass->m_textureCreates.begin(), m_pass->m_textureCreates.end(),
				[handle](const std::pair<RGTextureHandle, RHI_TextureCreateInfo>& pair)
				{
					return handle == pair.first;
				}) == m_pass->m_textureCreates.end())
			{
				m_pass->m_textureCreates.push_back(std::make_pair(handle, desc));
			}
			return handle;
		}

		RGTextureHandle RenderGraphBuilder::GetTexture(std::wstring textureName)
		{
			return m_rg->GetTexture(textureName);
		}

		void RenderGraphBuilder::ReadTexture(RGTextureHandle handle)
		{
			if (std::find(m_pass->m_textureReads.begin(), m_pass->m_textureReads.end(), handle) == m_pass->m_textureReads.end())
			{
				m_pass->m_textureReads.push_back(handle);
			}
		}

		void RenderGraphBuilder::WriteTexture(RGTextureHandle handle)
		{
			if (std::find(m_pass->m_textureWrites.begin(), m_pass->m_textureWrites.end(), handle) == m_pass->m_textureWrites.end())
			{
				m_pass->m_textureWrites.push_back(handle);
			}
		}

		void RenderGraphBuilder::WriteDepthStencil(RGTextureHandle handle)
		{
			m_pass->m_depthStencilWrite = handle;
		}

		void RenderGraphBuilder::SetShader(ShaderDesc shaderDesc)
		{
			m_pass->m_shader = shaderDesc;
		}

		void RenderGraphBuilder::SetPipeline(PipelineStateObject pso)
		{
			m_pass->m_pso = std::move(pso);
		}

		void RenderGraphBuilder::SetViewport(u32 width, u32 height)
		{
			m_pass->m_viewport = glm::ivec2(width, height);
		}

		void RenderGraphBuilder::SetScissor(u32 width, u32 height)
		{
			m_pass->m_scissor = glm::ivec2(width, height);
		}
	}
}
#endif /// RENDER_GRAPH_ENABLED