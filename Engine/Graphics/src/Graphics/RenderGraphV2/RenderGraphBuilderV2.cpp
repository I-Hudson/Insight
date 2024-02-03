#include "Graphics/RenderGraphV2/RenderGraphBuilderV2.h"
#include "Graphics/RenderGraphV2/RenderGraphPassV2.h"
#include "Graphics/RenderGraphV2/RenderGraphV2.h"
#include "Platforms/Platform.h"

namespace Insight
{
	namespace Graphics
	{
		RenderGraphBuilderV2::RenderGraphBuilderV2(RenderGraphV2* rg)
			: m_rg(rg)
		{ }

		void RenderGraphBuilderV2::SetPass(RenderGraphPassBaseV2* pass)
		{
			m_pass = pass;
			ASSERT(m_pass);
		}

		void RenderGraphBuilderV2::SetAsRenderToSwapchain()
		{
			m_pass->m_swapchainPass = true;
		}

		void RenderGraphBuilderV2::SetRenderpass(RenderpassDescription description)
		{
			m_pass->m_renderpassDescription = std::move(description);
		}

		glm::ivec2 RenderGraphBuilderV2::GetRenderResolution() const
		{
			return m_rg->GetRenderResolution();
		}

		glm::ivec2 RenderGraphBuilderV2::GetOutputResolution() const
		{
			return m_rg->GetOutputResolution();
		}

		RGTextureHandle RenderGraphBuilderV2::CreateTexture(std::string textureName, RHI_TextureInfo desc)
		{
			RGTextureHandle handle = m_rg->CreateTexture(textureName, desc);
			if (std::find_if(m_pass->m_textureCreates.begin(), m_pass->m_textureCreates.end(),
				[handle](const std::pair<RGTextureHandle, RHI_TextureInfo>& pair)
				{
					return handle == pair.first;
				}) == m_pass->m_textureCreates.end())
			{
				m_pass->m_textureCreates.push_back(std::make_pair(handle, desc));
			}
			return handle;
		}

		RGTextureHandle RenderGraphBuilderV2::GetTexture(std::string textureName)
		{
			return m_rg->GetTexture(textureName);
		}

		RHI_Texture* RenderGraphBuilderV2::GetRHITexture(std::string textureName)
		{
			return GetRHITexture(GetTexture(textureName));
		}

		RHI_Texture* RenderGraphBuilderV2::GetRHITexture(RGTextureHandle textureHandle)
		{
			return m_rg->GetRHITexture(textureHandle);
		}

		void RenderGraphBuilderV2::ReadTexture(RGTextureHandle handle)
		{
			if (std::find(m_pass->TextureReads.begin(), m_pass->TextureReads.end(), handle) == m_pass->TextureReads.end())
			{
				m_pass->TextureReads.push_back(handle);
			}
		}

		void RenderGraphBuilderV2::WriteTexture(RGTextureHandle handle)
		{
			if (std::find(m_pass->TextureWrites.begin(), m_pass->TextureWrites.end(), handle) == m_pass->TextureWrites.end())
			{
				m_pass->TextureWrites.push_back(handle);
			}
		}

		void RenderGraphBuilderV2::WriteDepthStencil(RGTextureHandle handle)
		{
			m_pass->m_depthStencilWrite = handle;
		}

		void RenderGraphBuilderV2::SetShader(ShaderDesc shaderDesc)
		{
			m_pass->m_shader = shaderDesc;
		}

		void RenderGraphBuilderV2::SetPipeline(PipelineStateObject pso)
		{
			m_pass->m_PSO = std::move(pso);
		}

		void RenderGraphBuilderV2::SetViewport(u32 width, u32 height)
		{
			m_pass->m_viewport = glm::ivec2(width, height);
		}

		void RenderGraphBuilderV2::SetScissor(u32 width, u32 height)
		{
			m_pass->m_scissor = glm::ivec2(width, height);
		}

		void RenderGraphBuilderV2::SkipTextureWriteBarriers()
		{
			//m_pass->m_skipTextureWriteBarriers = true;
		}

		void RenderGraphBuilderV2::SkipTextureReadBarriers()
		{
			//m_pass->m_skipTextureReadBarriers = true;
		}
	}
}