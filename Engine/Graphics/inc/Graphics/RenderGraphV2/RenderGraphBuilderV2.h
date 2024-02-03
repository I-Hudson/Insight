#pragma once

#include "Graphics/RenderTarget.h"
#include "Graphics/RHI/RHI_Texture.h"
#include "Graphics/PipelineStateObject.h"
#include "Graphics/ShaderDesc.h"
#include "Graphics/RHI/RHI_Renderpass.h"

#include <glm/ext/vector_int2.hpp>

namespace Insight
{
	namespace Graphics
	{
		using RGTextureHandle = int;

		class RenderGraphPassBaseV2;
		class RenderGraphV2;

		class IS_GRAPHICS RenderGraphBuilderV2
		{
		public:
			RenderGraphBuilderV2(RenderGraphV2* rg);

			void SetPass(RenderGraphPassBaseV2* pass);
			void SetAsRenderToSwapchain();

			void SetRenderpass(RenderpassDescription description);

			glm::ivec2 GetRenderResolution() const;
			glm::ivec2 GetOutputResolution() const;

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
			RenderGraphPassBaseV2* m_pass = nullptr;
			RenderGraphV2* m_rg = nullptr;
		};
	}
}