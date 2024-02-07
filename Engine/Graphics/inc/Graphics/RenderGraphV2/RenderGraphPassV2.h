#pragma once

#ifdef RENDERGRAPH_V2_ENABLED

#include "Graphics/Enums.h"
#include "Graphics/RenderGraphV2/RenderGraphTask.h"

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

		struct RenderGraphExecuteData
		{
			RenderGraphV2& RenderGraph;
			RHI_CommandList* CmdList;
		};
		struct RenderGraphPostData
		{
			RenderGraphV2& RenderGraph;
			RHI_CommandList* CmdList;
		};

		class IS_GRAPHICS RenderGraphPassBaseV2 : public RenderGraphTask
		{
		public:
			RenderGraphPassBaseV2(RenderGraphV2* renderGraph, GPUQueue gpuQueue);
			virtual ~RenderGraphPassBaseV2();

			RGBufferHandle CreateBuffer(std::string bufferName, RHI_BufferCreateInfo createInfo);
			RGTextureHandle CreateTexture(std::string textureName, RHI_TextureInfo createInfo);

			void ReadBuffer(const RGBufferHandle handle);
			void WriteBuffer(const RGBufferHandle handle);

			void ReadTexture(const RGTextureHandle handle);
			void WriteTexture(const RGTextureHandle handle);

		public:
			std::string m_passName;

			std::vector<std::pair<RGBufferHandle, RHI_BufferCreateInfo>> m_bufferCreates;
			std::vector<RGBufferHandle> m_bufferReads;
			std::vector<RGBufferHandle> m_bufferWrites;

			std::vector<std::pair<RGTextureHandle, RHI_TextureInfo>> m_textureCreates;
			std::vector<RGTextureHandle> m_textureReads;
			std::vector<RGTextureHandle> m_textureWrites;

			friend class RenderGraphV2;
		};

		class IS_GRAPHICS RenderGraphGraphicsPassV2 : public RenderGraphPassBaseV2
		{
		public:
			using PreExecuteFunc = std::function<void(RenderGraphGraphicsPassV2& graphPass)>;
			using ExecuteFunc = std::function<void(const RenderGraphExecuteData& excuteData)>;
			using PostExecuteFunc = std::function<void(const RenderGraphPostData& postExecuteData)>;

			RenderGraphGraphicsPassV2(RenderGraphV2* renderGraph
				, std::string passName
				, PreExecuteFunc setupFunc
				, ExecuteFunc executeFunc
				, PostExecuteFunc postFunc);

			virtual ~RenderGraphGraphicsPassV2() override;

			virtual const char* GetTaskName() const override { return "RenderGraphGraphicsPassV2"; }

			virtual void PreExecute() override;
			virtual void Execute() override;
			virtual void PostExecute() override;

			void SetViewport(const u32 width, const u32 height);
			void SetScissor(const u32 width, const u32 height);

			void SetShader(const ShaderDesc shaderDesc);
			void SetPipeline(const PipelineStateObject pipelineStateObject);
			void SetRenderpass(const RenderpassDescription renderpassDescription);

			/// @brief Does this pass render directly on top of the swapchain image.
			void SetAsRenderToSwapchain();

			RGTextureHandle GetDepthSteniclTexture() const;

		private:
			/// Optional, define a custom render pass. Otherwise we create it and/or fill in the blanks.
			RenderpassDescription m_renderpassDescription = { };

			bool m_swapchainPass = false; // Does this render straight to the swap chain.

			ShaderDesc m_shader = { }; // Shader to be used for this pass.
			PipelineStateObject m_PSO = { }; // PSO to be used for this pass.

			glm::ivec2 m_viewport = glm::ivec2(0, 0);
			glm::ivec2 m_scissor = glm::ivec2(0, 0);

			bool m_renderOnTopOfSwapchain = false;

			PreExecuteFunc m_preExecuteFunc;
			ExecuteFunc m_executeFunc;
			PostExecuteFunc m_postExecuteFunc;

			friend class RenderGraphV2;
		};
	}
}
#endif