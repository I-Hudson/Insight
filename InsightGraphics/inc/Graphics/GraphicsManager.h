#pragma once

#include "Defines.h"
#include "Core/ManagerInterface.h"
#include "Graphics/Renderpass.h"
#include "Graphics/RenderGraph/RenderGraph.h"

#include <thread>
#include <mutex>
#include <atomic>

//#define IS_EXP_ENABLE_THREADED_RENDERING

namespace Insight
{
	namespace Graphics
	{
		class GPUDevice;
		class RenderContext;

		enum class GraphicsAPI
		{
			Vulkan,
			DX12,

			None,
		};

		struct GraphicsManagerData
		{
			GPUDevice* GPUDevice;
			GraphicsAPI GraphicsAPI = GraphicsAPI::None;
		};

		class IS_GRAPHICS GraphicsManager : public Insight::Core::ManagerInterface
		{
		public:
			GraphicsManager();
			~GraphicsManager();

			static GraphicsManager& Instance()
			{
				return *s_instance;
			}
			static bool IsVulkan() { return Instance().m_sharedData.GraphicsAPI == GraphicsAPI::Vulkan; }
			static bool IsDX12() { return Instance().m_sharedData.GraphicsAPI == GraphicsAPI::DX12; }

			RenderContext* GetRenderContext() const { return m_renderContext; }

			virtual bool Init() override;
			virtual void Update(const float deltaTime) override;
			virtual void Destroy() override;

		private:
			static GraphicsManagerData m_sharedData;
			static GraphicsManager* s_instance;

#ifdef IS_EXP_ENABLE_THREADED_RENDERING
			std::thread m_renderThread;
			std::atomic<bool> m_triggerRender = false;
			std::atomic<bool> m_multiThreadRender = true;
			std::atomic<int> m_frameOffset = 0;
			CommandList m_renderCommandList;
			std::mutex m_renderCommandListMutex;
#endif
#ifdef RENDER_GRAPH_ENABLED
			RenderGraph m_renderGraph;
#endif

			RenderContext* m_renderContext{ nullptr };
			Renderpass m_renderpass;
		};
	}
}