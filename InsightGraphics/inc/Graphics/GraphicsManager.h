#pragma once

#include "Defines.h"
#include "Core/ManagerInterface.h"
#include "Graphics/Renderpass.h"

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

			static GraphicsManager& Instance()
			{
				static GraphicsManager instance;
				return instance;
			}
			static bool IsVulkan() { return Instance().m_sharedData.GraphicsAPI == GraphicsAPI::Vulkan; }
			static bool IsDX12() { return Instance().m_sharedData.GraphicsAPI == GraphicsAPI::DX12; }

			RenderContext* GetRenderContext() const { return m_renderContext; }

			virtual bool Init() override;
			virtual void Update(const float deltaTime) override;
			virtual void Destroy() override;

		private:
			static GraphicsManagerData m_sharedData;

			RenderContext* m_renderContext{ nullptr };
			Renderpass m_renderpass;
		};
	}
}