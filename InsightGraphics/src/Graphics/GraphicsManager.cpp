#include "Graphics/GraphicsManager.h"
#include "Graphics/GPU/GPUDevice.h"
#include "Graphics/PixelFormatExtensions.h"

#include "Graphics/GPU/RHI/Vulkan/GPUDevice_Vulkan.h"
#include "Graphics/GPU/RHI/DX12/GPUDevice_DX12.h"

namespace Insight
{
	namespace Graphics
	{
		GraphicsManagerData GraphicsManager::m_sharedData;

		bool GraphicsManager::Init()
		{
			PixelFormatExtensions::Init();

			m_sharedData.GraphicsAPI = GraphicsAPI::Vulkan;
			m_sharedData.GPUDevice = GPUDevice::New();
			if (!m_sharedData.GPUDevice->Init())
			{
				// Error message.
				return false;
			}
			m_renderer.Init(m_sharedData.GPUDevice);

			return true;
		}

		void GraphicsManager::Update(const float deltaTime)
		{
			m_renderer.Render();
		}

		void GraphicsManager::Destroy()
		{
			m_sharedData.GPUDevice->WaitForGPU();

			m_renderer.Destroy();

			if (m_sharedData.GPUDevice)
			{
				m_sharedData.GPUDevice->Destroy();
				delete m_sharedData.GPUDevice;
			}
		}
	}
}