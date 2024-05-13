#pragma once

#include "Graphics/RenderTarget.h"
#include "Graphics/RHI/RHI_Texture.h"
#include "Graphics/PipelineStateObject.h"
#include "Graphics/ShaderDesc.h"
#include "Graphics/RHI/RHI_Renderpass.h"

#include "Maths/Vector2.h"

namespace Insight
{
	namespace Graphics
	{
		using RGTextureHandle = int;

		class RenderGraphPassBase;
		class RenderGraph;

		class IS_GRAPHICS RenderGraphBuilder
		{
		public:
			RenderGraphBuilder(RenderGraph* rg);

			void SetPass(RenderGraphPassBase* pass);
			void SetAsRenderToSwapchain();

			void SetRenderpass(RenderpassDescription description);

			Maths::Vector2 GetRenderResolution() const;
			Maths::Vector2 GetOutputResolution() const;

			RGTextureHandle CreateTexture(std::string textureName, RHI_TextureInfo desc);
			RGTextureHandle GetTexture(std::string textureName);

			RHI_Texture* GetRHITexture(std::string textureName);
			RHI_Texture* GetRHITexture(RGTextureHandle textureHandle);

			void ReadTexture(RGTextureHandle handle);
			void WriteTexture(RGTextureHandle handle);

			void WriteDepthStencil(RGTextureHandle handle);

			void SetShader(ShaderDesc shaderDesc);
			void SetPipeline(PipelineStateObject pso);

			void SetViewport(u32 width, u32 height);
			void SetScissor(u32 width, u32 height);

			void SkipTextureWriteBarriers();
			void SkipTextureReadBarriers();

		private:
			RenderGraphPassBase* m_pass = nullptr;
			RenderGraph* m_rg = nullptr;
		};
	}
}