#pragma once

#ifdef RENDER_GRAPH_ENABLED

#include "Graphics/Enums.h"
#include "Graphics/RenderTarget.h"
#include "Graphics/ShaderDesc.h"
#include "Graphics/PipelineStateObject.h"
#include "Graphics/RHI/RHI_Texture.h"
#include "Graphics/RHI/RHI_Renderpass.h"

#include <functional>
#include <vector>


namespace Insight
{
	namespace Graphics
	{
		using RGTextureHandle = int;

		class RenderContext;
		class RHI_CommandList;

		class RenderGraph;
		class RenderGraphBuilder;

		struct BufferBarrier
		{ };

		struct ImageSubresourceRange
		{
			ImageAspectFlags AspectMask;
			u32 BaseMipLevel;
			u32 LevelCount;
			u32 BaseArrayLayer;
			u32 LayerCount;

			static ImageSubresourceRange SingleMipAndLayer(ImageAspectFlags aspectMask)
			{
				return ImageSubresourceRange
				{
					aspectMask, 
					0, 
					1, 
					0, 
					1
				};
			}
		};

		struct ImageBarrier
		{
			AccessFlags SrcAccessFlags;
			AccessFlags DstAccessFlags;
			ImageLayout OldLayout;
			ImageLayout NewLayout;
			RHI_Texture* Image;
			ImageSubresourceRange SubresourceRange;
		};

		struct PipelineBarrier
		{
			PipelineStageFlags SrcStage;
			PipelineStageFlags DstStage;

			std::vector<BufferBarrier> BufferBarriers;
			std::vector<ImageBarrier> ImageBarriers;
		};

		/// <summary>
		/// 
		/// </summary>
		class RenderGraphPassBase
		{
		public:

		protected:
			virtual void Setup(RenderGraphBuilder& builder) = 0;
			virtual void Execute(RHI_CommandList* cmdList) = 0;

		public:
			std::vector<std::pair<RGTextureHandle, RHI_TextureCreateInfo>> m_textureCreates;
			std::vector<RGTextureHandle> m_textureReads;
			std::vector<RGTextureHandle> m_textureWrites;
			RGTextureHandle m_depthStencilWrite = -1;
		
			// Optional, define a custom render pass. Otherwise we create it and/or fill in the blanks.
			RenderpassDescription m_renderpassDescription = { };

			bool m_swapchainPass = false; // Does this render straight to the swap chain.

			ShaderDesc m_shader = { };
			PipelineStateObject m_pso = { };

			glm::ivec2 m_viewport;
			glm::ivec2 m_scissor;

			std::vector<PipelineBarrier> m_textureIncomingBarriers; // These should be done before rendering.

			friend class RenderGraphBuilder;
			friend class RenderGraph;
		};

		template<typename TPassData>
		class RenderGraphPass : public RenderGraphPassBase
		{
		public:
			using SetupFunc = std::function<void(TPassData&, RenderGraphBuilder&)>;
			using ExecuteFunc = std::function<void(TPassData&, RenderGraphPassBase&, RHI_CommandList*)>;

			RenderGraphPass(std::string passName, SetupFunc setupFunc, ExecuteFunc executeFunc, TPassData initalData)
				: m_passName(std::move(passName))
				, m_setupFunc(setupFunc)
				, m_executeFunc(executeFunc)
				, m_passData(std::move(initalData))
			{ }

			TPassData& GetData() const { return m_passData; }

		protected:
			virtual void Setup(RenderGraphBuilder& builder) override
			{
				m_setupFunc(m_passData, builder);
			}
			virtual void Execute(RHI_CommandList* cmdList) override
			{
				m_executeFunc(m_passData, *this, cmdList);
			}

		private:
			TPassData m_passData;

			std::string m_passName;
			SetupFunc m_setupFunc;
			ExecuteFunc m_executeFunc;
		};
	}
}
#endif // RENDER_GRAPH_ENABLED