#pragma once

#ifdef RENDERGRAPH_V2_ENABLED

#include "Core/Singleton.h"
#include "Graphics/RenderGraphV2/RenderGraphPassV2.h"

#include "Graphics/RenderContext.h"

#include <functional>

#ifdef RENDER_GRAPH_RENDER_THREAD
#include <ppltasks.h>
#endif

namespace Insight
{
	namespace Graphics
	{
		class RHI_Texture;
		class RHI_CommandList;
		class RenderContext;

		class IS_GRAPHICS RenderGraphV2 : public Core::Singleton<RenderGraphV2>
		{
			using RenderGraphSetPreRenderFunc = std::function<void(RenderGraphV2&, RHI_CommandList*)>;
			using RenderGraphSetPostRenderFunc = std::function<void(RenderGraphV2&, RHI_CommandList*)>;
			using RenderGraphSyncFunc = std::function<void()>;

		public:
			RenderGraphV2();

			void Init(RenderContext* context);
			void Swap();
			void Execute(RHI_CommandList* cmdList);

			RGBufferHandle CreateBuffer(std::string bufferName);
			RGTextureHandle CreateTexture(std::string bufferName);

			RGBufferHandle GetBuffer(std::string bufferName) const;
			RHI_Buffer* GetRHIBuffer(std::string bufferName) const;
			RHI_Buffer* GetRHIBuffer(RGTextureHandle handle) const;

			RGTextureHandle GetTexture(std::string textureName) const;
			RHI_Texture* GetRHITexture(std::string textureName) const;
			RHI_Texture* GetRHITexture(RGTextureHandle handle) const;

			RHI_Texture* GetRenderCompletedRHITexture(std::string textureName) const;

			RenderpassDescription GetRenderpassDescription(std::string_view passName) const;
			PipelineStateObject GetPipelineStateObject(std::string_view passName) const;

			void Release();

			/// @brief Add a lambda which will be called when the render graph is synced between the update and render.
			/// Before the render thread is signalled for execution.
			/// @param func 
			void AddSyncPoint(RenderGraphSyncFunc func);

			void AddPreRender(RenderGraphSetPreRenderFunc func);
			void AddPostRender(RenderGraphSetPreRenderFunc func);

			void AddGraphicsPass(std::string passName
				, RenderGraphGraphicsPassV2::PreExecuteFunc preExecuteFunc
				, RenderGraphGraphicsPassV2::ExecuteFunc executeFunc
				, RenderGraphGraphicsPassV2::PostExecuteFunc postExecuteFunc)
			{
				std::lock_guard lock(m_mutex);
				GetGraphicsPendingPasses().emplace_back(
					RenderGraphGraphicsPassV2(
						this
						, std::move(passName)
						, std::move(preExecuteFunc)
						, std::move(executeFunc)
						, std::move(postExecuteFunc)));
			}

			/// @brief Set the render resolution size.
			/// @param render_resolution 
			void SetRenderResolution(Maths::Vector2 render_resolution);
			/// @brief Return the render resoltuion size.
			/// @return glm::ivec2
			Maths::Vector2 GetRenderResolution() const { return m_render_resolution; }

			/// @brief Attempt to set the output resolution. This is restricted by the window size (Best to leave alone).
			/// @param output_resolution 
			void SetOutputResolution(glm::ivec2 output_resolution);
			/// @brief Return then output size.
			/// @return glm::ivec2
			Maths::Vector2 GetOutputResolution() const { return m_output_resolution; }

		private:
			void Build();
			void PlaceBarriers();
			void Render(RHI_CommandList* cmdList);
			void Clear();

			void PlaceBarriersInToPipeline(RenderGraphPassBaseV2* pass, RHI_CommandList* cmdList);

			std::vector<RenderGraphGraphicsPassV2>& GetGraphicsPendingPasses();
			std::vector<RenderGraphGraphicsPassV2>& GetGraphicsRenderPasses();

			const std::vector<RenderGraphGraphicsPassV2>& GetGraphicsPendingPasses() const;
			const std::vector<RenderGraphGraphicsPassV2>& GetGraphicsRenderPasses() const;

		private:
			RenderContext* m_context = nullptr;
			std::mutex m_mutex;

			u32 m_passesUpdateIndex = 0;
			u32 m_passesRenderIndex = 1;

			std::vector<RenderGraphSyncFunc> m_syncFuncs;
			std::vector<RenderGraphSyncFunc> m_renderSyncFuncs;

			std::vector<RenderGraphSetPreRenderFunc> m_preRenderFunc;
			std::vector<RenderGraphSetPostRenderFunc> m_postRenderFunc;

			std::vector<RenderGraphSetPreRenderFunc> m_renderPreRenderFunc;
			std::vector<RenderGraphSetPostRenderFunc> m_renderPostRenderFunc;

			std::vector<std::vector<RenderGraphGraphicsPassV2>> m_graphicsPasses;
			//std::vector<std::vector<UPtr<RenderGraphComputePassV2>>> m_computePasses;
			std::vector<RenderGraphPassBaseV2*> m_orderedPasses;


			/// @brief General render resolution to be used for all render passes. Can be overwritten.
			Maths::Vector2 m_render_resolution = {};
			bool m_render_resolution_has_changed = false;
			/// @brief Set the render resolution to the window resolution when the window resolution has changed automaticity.
			bool m_set_render_resolution_to_window_resolution_auto = true;
			/// @brief General ouput resolution to be used for all render passes. Can be overwritten.
			Maths::Vector2 m_output_resolution = {};

			FrameResource<RHI_ResourceCache<RHI_Buffer>*> m_bufferCaches;
			FrameResource<RHI_ResourceCache<RHI_Texture>*> m_textureCaches;
		};
	}
}
#endif