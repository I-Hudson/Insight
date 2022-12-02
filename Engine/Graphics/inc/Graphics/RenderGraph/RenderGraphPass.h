#pragma once

#include "Graphics/Enums.h"
#include "Graphics/RenderTarget.h"
#include "Graphics/ShaderDesc.h"
#include "Graphics/PipelineStateObject.h"
#include "Graphics/RHI/RHI_Texture.h"
#include "Graphics/RHI/RHI_Renderpass.h"

#include "Graphics/PipelineBarrier.h"

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


		//// <summary>
		//// 
		//// </summary>
		class RenderGraphPassBase
		{
		public:

		protected:
			virtual void Setup(RenderGraphBuilder& builder) = 0;
			virtual void Execute(RenderGraph& renderGraph, RHI_CommandList* cmdList) = 0;

		public:
			std::string m_passName;

			std::vector<std::pair<RGTextureHandle, RHI_TextureInfo>> m_textureCreates;
			std::vector<RGTextureHandle> m_textureReads;
			std::vector<RGTextureHandle> m_textureWrites;

			bool m_skipTextureWriteBarriers = false; // HACK:
			bool m_skipTextureReadBarriers = false; // HACK:

			RGTextureHandle m_depthStencilWrite = -1;
		
			/// Optional, define a custom render pass. Otherwise we create it and/or fill in the blanks.
			RenderpassDescription m_renderpassDescription = { };

			bool m_swapchainPass = false; /// Does this render straight to the swap chain.

			ShaderDesc m_shader = { };
			PipelineStateObject m_pso = { };

			glm::ivec2 m_viewport;
			glm::ivec2 m_scissor;

			std::vector<PipelineBarrier> m_textureIncomingBarriers; /// These should be done before rendering.

			friend class RenderGraphBuilder;
			friend class RenderGraph;
		};

		template<typename TPassData>
		class RenderGraphPass : public RenderGraphPassBase
		{
		public:
			using SetupFunc = std::function<void(TPassData&, RenderGraphBuilder&)>;
			using ExecuteFunc = std::function<void(TPassData&, RenderGraph&, RHI_CommandList*)>;

			RenderGraphPass(std::string passName, SetupFunc setupFunc, ExecuteFunc executeFunc, TPassData initalData)
				: m_setupFunc(std::move(setupFunc))
				, m_executeFunc(std::move(executeFunc))
				, m_passData(std::move(initalData))
			{
				m_passName = std::move(passName);
			}

			TPassData& GetData() const { return m_passData; }

		protected:
			virtual void Setup(RenderGraphBuilder& builder) override
			{
				m_setupFunc(m_passData, builder);
			}
			virtual void Execute(RenderGraph& renderGraph, RHI_CommandList* cmdList) override
			{
				m_executeFunc(m_passData, renderGraph, cmdList);
			}

		private:
			TPassData m_passData;

			SetupFunc m_setupFunc;
			ExecuteFunc m_executeFunc;
		};
	}
}