#pragma once

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
		using RGTextureHandle = int;
		using RGBufferHandle = int;

		class RenderContext;
		class RHI_CommandList;

		class RenderGraphV2;
		class RenderGraphBuilderV2;

		struct RenderGraphSetupData
		{
			RenderGraphBuilderV2& Builder;
		};
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

		class RenderGraphPassBaseV2 : public RenderGraphTask
		{
		public:
			template<typename TPassData>
			using SetupFunc = std::function<void(TPassData& passData, const RenderGraphBuilderV2& buildData)>;
			template<typename TPassData>
			using ExecuteFunc = std::function<void(TPassData& passData, const RenderGraphExecuteData& excuteData)>;
			template<typename TPassData>
			using PostFunc = std::function<void(TPassData& passData, const RenderGraphPostData& postData)>;

			RenderGraphPassBaseV2(GPUQueue gpuQueue) : RenderGraphTask(gpuQueue) { }
			virtual ~RenderGraphPassBaseV2() { }

		protected:
			virtual void Setup(const RenderGraphSetupData& data) = 0;
			virtual void Execute(const RenderGraphExecuteData& data) = 0;
			virtual void Post(const RenderGraphPostData& data) = 0;

		public:
			std::string m_passName;

			std::vector<std::pair<RGBufferHandle, RHI_BufferCreateInfo>> m_bufferCreates;
			std::vector<std::pair<RGTextureHandle, RHI_TextureInfo>> m_textureCreates;

			RGTextureHandle m_depthStencilWrite = -1;

			/// Optional, define a custom render pass. Otherwise we create it and/or fill in the blanks.
			RenderpassDescription m_renderpassDescription = { };

			bool m_swapchainPass = false; // Does this render straight to the swap chain.

			ShaderDesc m_shader = { }; // Shader to be used for this pass.
			PipelineStateObject m_PSO = { }; // PSO to be used for this pass.

			glm::ivec2 m_viewport;
			glm::ivec2 m_scissor;

			std::vector<PipelineBarrier> m_incomingBarriers; /// These should be done before rendering.

			friend class RenderGraphBuilderV2;
			friend class RenderGraphV2;
		};

		template<typename TPassData>
		class RenderGraphGraphicsPassV2 : public RenderGraphPassBaseV2
		{
		public:
			RenderGraphGraphicsPassV2(std::string passName, SetupFunc<TPassData> setupFunc, ExecuteFunc<TPassData> executeFunc, PostFunc<TPassData> postFunc, TPassData initalData)
				: RenderGraphPassBaseV2(GPUQueue_Graphics)
				, m_setupFunc(std::move(setupFunc))
				, m_executeFunc(std::move(executeFunc))
				, m_postFunc(std::move(postFunc))
				, m_passData(std::move(initalData))
			{
				m_passName = std::move(passName);
			}

			virtual ~RenderGraphGraphicsPassV2() override
			{
				m_setupFunc = {};
				m_executeFunc = {};
				m_postFunc = {};
				m_passData = {};
			}

			TPassData& GetData() const { return m_passData; }

			virtual const char* GetTaskName() const override { return "RenderGraphGraphicsPassV2"; }

		protected:
			virtual void Setup(const RenderGraphSetupData& data) override
			{
				m_setupFunc(m_passData, data.Builder);
			}
			virtual void Execute(const RenderGraphExecuteData& data) override
			{
				m_executeFunc(m_passData, data.RenderGraph, data.CmdList);
			}
			virtual void Post(const RenderGraphPostData& data) override
			{
				m_postFunc(m_passData, data.RenderGraph, data.CmdList);
			}

		private:
			TPassData m_passData;

			SetupFunc<TPassData> m_setupFunc;
			ExecuteFunc<TPassData> m_executeFunc;
			ExecuteFunc<TPassData> m_postFunc;
		};
	}
}