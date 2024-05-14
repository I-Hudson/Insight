#pragma once

#include "Core/Memory.h"
#include "Core/Singleton.h"
#include "Graphics/RenderGraph/RenderGraphPass.h"
#include "Graphics/RenderGraph/RenderGraphBuilder.h"
#include "Graphics/RHI/RHI_Descriptor.h"

#include "Graphics/RenderContext.h"

#include <type_traits>
#include <unordered_map>
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

		class IS_GRAPHICS RenderGraph : public Core::Singleton<RenderGraph>
		{
			using RenderGraphSetPreRenderFunc = std::function<void(RenderGraph&, RHI_CommandList*)>;
			using RenderGraphSetPostRenderFunc = std::function<void(RenderGraph&, RHI_CommandList*)>;
			using RenderGraphSyncFunc = std::function<void()>;

		public:
			RenderGraph();

			void Init(RenderContext* context);
			void Swap();
			void Execute(RHI_CommandList* cmdList);

			RGTextureHandle CreateTexture(std::string textureName, RHI_TextureInfo info);

			RGTextureHandle GetTexture(std::string textureName) const;
			RHI_Texture* GetRHITexture(std::string textureName) const;
			RHI_Texture* GetRHITexture(RGTextureHandle handle) const;

			bool HasTexture(RHI_Texture* texture) const
			{
				return m_textureCaches.Get()->HasValue(texture);
			}

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

			template<typename TData>
			void AddPass(std::string passName, typename RenderGraphPass<TData>::SetupFunc setupFunc
				, typename RenderGraphPass<TData>::ExecuteFunc executeFunc, TData initalData = { })
			{
				std::lock_guard lock(m_mutex);
				GetUpdatePasses().emplace_back(
					MakeUPtr<RenderGraphPass<TData>>(
						std::move(passName)
						, std::move(setupFunc)
						, std::move(executeFunc)
						, std::move([](TData&, RenderGraph&, RHI_CommandList*) {})
						, std::move(initalData)));
			}

			template<typename TData>
			void AddPass(std::string passName
				, typename RenderGraphPass<TData>::SetupFunc setupFunc
				, typename RenderGraphPass<TData>::ExecuteFunc executeFunc
				, typename RenderGraphPass<TData>::PostFunc postFunc
				, TData initalData = { })
			{
				std::lock_guard lock(m_mutex);
				GetUpdatePasses().emplace_back(
					MakeUPtr<RenderGraphPass<TData>>(
						std::move(passName)
						, std::move(setupFunc)
						, std::move(executeFunc)
						, std::move(postFunc)
						, std::move(initalData)));
			}

			/// @brief Set the render resolution size.
			/// @param render_resolution 
			void SetRenderResolution(Maths::Vector2 render_resolution);
			/// @brief Return the render resoltuion size.
			/// @return glm::ivec2
			Maths::Vector2 GetRenderResolution() const { return m_render_resolution; }

			/// @brief Attempt to set the output resolution. This is restricted by the window size (Best to leave alone).
			/// @param output_resolution 
			void SetOutputResolution(Maths::Vector2 output_resolution);
			/// @brief Return then output size.
			/// @return glm::ivec2
			Maths::Vector2 GetOutputResolution() const { return m_output_resolution; }

		private:
			void Build();
			void PlaceBarriers();
			void Render(RHI_CommandList* cmdList);
			void Clear();

			void PlaceBarriersInToPipeline(RenderGraphPassBase* pass, RHI_CommandList* cmdList);

			std::vector<UPtr<RenderGraphPassBase>>& GetUpdatePasses();
			std::vector<UPtr<RenderGraphPassBase>>& GetRenderPasses();

			const std::vector<UPtr<RenderGraphPassBase>>& GetUpdatePasses() const;
			const std::vector<UPtr<RenderGraphPassBase>>& GetRenderPasses() const;

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

			std::vector<std::vector<UPtr<RenderGraphPassBase>>> m_passes;

			/// @brief General render resolution to be used for all render passes. Can be overwritten.
			Maths::Vector2 m_render_resolution = {};
			bool m_render_resolution_has_changed = false;
			
			// Hack for ImGui. Currently there is an issue with ImGui and textures where if frame 1 resizes the window then all RenderGraph textures
			// are destroyed, while this is happening on the RenderThread the GameThread has already updated ImGui to include pending/destroyed textures.
			// To composate for this we just skip the frame after the window resize.
			bool m_render_resolution_has_changedSkip2ndFrame = false;
			
			/// @brief Set the render resolution to the window resolution when the window resolution has changed automaticly.
			bool m_set_render_resolution_to_window_resolution_auto = true;
			/// @brief General ouput resolution to be used for all render passes. Can be overwritten.
			Maths::Vector2 m_output_resolution = {};

			FrameResource<RHI_ResourceCache<RHI_Texture>*> m_textureCaches;
		};
	}
}