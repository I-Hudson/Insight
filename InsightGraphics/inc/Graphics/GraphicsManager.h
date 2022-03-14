#pragma once

#include "Defines.h"
#include "Core/ManagerInterface.h"
#include "Renderer.h"

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
			namespace DX
			{
				class GPUResource_DX12;
			}
		}

		class GPUDevice;

		struct GraphicsManagerData
		{
			GPUDevice* GPUDevice;
		};

		class IS_GRAPHICS GraphicsManager : public Insight::Core::ManagerInterface
		{
		public:

			static GraphicsManager& Instance()
			{
				static GraphicsManager instance;
				return instance;
			}

			virtual bool Init() override;
			virtual void Update(const float deltaTime) override;
			virtual void Destroy() override;

		private:
			static GraphicsManagerData m_sharedData;

			Renderer m_renderer;

			friend class RHI::Vulkan::GPUResource_Vulkan;
			friend class RHI::DX::GPUResource_DX12;
		};
	}
}