#pragma once

#ifdef RENDER_GRAPH_ENABLED

#include "Graphics/RenderTarget.h"
#include "Graphics/RHI/RHI_Texture.h"
#include "Graphics/PipelineStateObject.h"
#include "Graphics/ShaderDesc.h"

namespace Insight
{
	namespace Graphics
	{
		using RGTextureHandle = int;

		class RenderGraphPassBase;
		class RenderGraph;

		class RenderGraphBuilder
		{
		public:
			RenderGraphBuilder(RenderGraph* rg);

			void SetPass(RenderGraphPassBase* pass);

			RGTextureHandle CreateTexture(std::string textureName, RHI_TextureCreateInfo desc);
			RGTextureHandle GetTexture(std::string textureName);

			void ReadTexture(RGTextureHandle handle);
			void WriteTexture(RGTextureHandle handle);

			void WriteDepthStencil(RGTextureHandle handle);

			void SetShader(ShaderDesc shaderDesc);
			void SetPipeline(PipelineStateObject pso);

			void SetViewport(u32 width, u32 height);
			void SetScissor(u32 width, u32 height);

		private:
			RenderGraphPassBase* m_pass = nullptr;
			RenderGraph* m_rg = nullptr;
		};
	}
}
#endif // RENDER_GRAPH_ENABLED