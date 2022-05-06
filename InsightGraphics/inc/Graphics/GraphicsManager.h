#pragma once

#include "Defines.h"
#include "Core/ManagerInterface.h"
#include "Graphics/Renderpass.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI
		{
			namespace Vulkan
			{
				class GPUResource_Vulkan;
			}
			namespace DX12
			{
				class GPUResource_DX12;
			}
		}

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

			virtual bool Init() override;
			virtual void Update(const float deltaTime) override;
			virtual void Destroy() override;

		private:
			static GraphicsManagerData m_sharedData;

			RenderContext* m_renderContext{ nullptr };
			Renderpass m_renderpass;

			friend class RHI::Vulkan::GPUResource_Vulkan;
			friend class RHI::DX12::GPUResource_DX12;
		};
	}
}