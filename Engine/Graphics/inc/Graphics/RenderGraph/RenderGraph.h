#pragma once

#include "Core/Memory.h"
#include "Core/Singleton.h"
#include "Graphics/RenderGraph/RenderGraphPass.h"
#include "Graphics/RenderGraph/RenderGraphBuilder.h"
#include "Graphics/RHI/RHI_CommandList.h"
#include "Graphics/RHI/RHI_Descriptor.h"

#include <type_traits>
#include <unordered_map>
#include <functional>
#include <ppltasks.h>

namespace Insight
{
	namespace Graphics
	{
		class RHI_Texture;
		class RenderContext;

		template<typename TValue>
		class FrameResource
		{
		public:
			void Setup()
			{
				m_values.clear();
				m_values.resize(RenderGraph::s_MaxFarmeCount);
			}

			TValue* operator->() const
			{
				return const_cast<TValue*>(&m_values.at(RenderGraph::Instance().GetFrameIndex()));
			}

			TValue& Get()
			{
				ASSERT(!m_values.empty());
				return m_values.at(RenderGraph::Instance().GetFrameIndex());
			}

			u64 Size() const
			{
				return m_values.size();
			}

			void ForEach(std::function<void(TValue& value)> func)
			{
				for (TValue& v : m_values)
				{
					func(v);
				}
			}

		private:
			std::vector<TValue> m_values;
		};

		class IS_GRAPHICS RenderGraph : public Core::Singleton<RenderGraph>
		{
			using RenderGraphSetPreRenderFunc = std::function<void(RenderGraph&, RHI_CommandList*)>;
		public:
			RenderGraph();

			void Init(RenderContext* context);
			void Execute();

			RGTextureHandle CreateTexture(std::wstring textureName, RHI_TextureCreateInfo info);

			RGTextureHandle GetTexture(std::wstring textureName) const;
			RHI_Texture* GetRHITexture(RGTextureHandle handle) const;

			RenderpassDescription GetRenderpassDescription(std::wstring_view passName) const;
			PipelineStateObject GetPipelineStateObject(std::wstring_view passName) const;

			void Release();

			u32 GetFrameIndex() const { return m_frameIndex; }
			u64 GetFrameCount() const { return m_frame_count; }

			void SetPreRender(RenderGraphSetPreRenderFunc func) { m_pre_render_func = std::move(func); }

			template<typename TData>
			void AddPass(std::wstring passName, typename RenderGraphPass<TData>::SetupFunc setupFunc
				, typename RenderGraphPass<TData>::ExecuteFunc executeFunc, TData initalData = { })
			{
				IS_PROFILE_FUNCTION();
				m_pending_passes.emplace_back(MakeUPtr<RenderGraphPass<TData>>(std::move(passName), std::move(setupFunc), std::move(executeFunc), std::move(initalData)));
			}

			void SetRenderResolution(glm::ivec2 render_resolution) { m_render_resolution = render_resolution; }
			glm::ivec2 GetRenderResolution() const { return m_render_resolution; }

			void SetOutputResolution(glm::ivec2 output_resolution) { m_output_resolution = output_resolution; }
			glm::ivec2 GetOutputResolution() const { return m_render_resolution; }

			static u32 s_MaxFarmeCount;

		private:
			void Build();
			void PlaceBarriers();
			void Render(RHI_CommandList* cmdList);
			void Clear();

			void PlaceBarriersInToPipeline(RenderGraphPassBase* pass, RHI_CommandList* cmdList);

		private:
			RenderContext* m_context = nullptr;
			RenderGraphSetPreRenderFunc m_pre_render_func = nullptr;
			std::vector<UPtr<RenderGraphPassBase>> m_pending_passes;
			std::vector<UPtr<RenderGraphPassBase>> m_passes;

#ifdef RENDER_GRAPH_RENDER_THREAD
			concurrency::task<void>* m_render_task = nullptr;

			std::mutex m_trigger_render_thread_lock;
			std::condition_variable m_trigger_render_thread_cv;
			bool m_trigger_render_thread_ready = false;

			std::mutex m_render_thread_finished_lock;
			std::condition_variable m_render_thread_finished_cv;
			bool m_render_thread_finished_ready = false;

			std::thread m_render_thread;
			std::atomic<bool> m_shutdown_render_thread = false;
			const bool m_render_thread_enabled = false;
#endif
			/// @brief General render resolution to be used for all render passes. Can be overwritten.
			glm::ivec2 m_render_resolution = {};
			/// @brief General ouput resolution to be used for all render passes. Can be overwritten.
			glm::ivec2 m_output_resolution = {};

			u32 m_frameIndex = 0;
			/// @brief Current frame count for the whole life time of the app (Only incremented when a render frame has happened).
			u64 m_frame_count = 0;

			RHI_ResourceCache<RHI_Texture>* m_textureCaches;
			std::unordered_map<RHI_Texture*, std::vector<ImageBarrier>> m_texture_barrier_history;

			FrameResource<CommandListManager> m_commandListManager;
			FrameResource<DescriptorAllocator> m_descriptorManagers;
		};
	}
}