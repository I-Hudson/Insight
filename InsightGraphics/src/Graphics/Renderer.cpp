#include "Graphics/Renderer.h"
#include "Graphics/Window.h"
#include "Graphics/GPU/GPUDevice.h"
#include "Graphics/GPU/GPUSwapchain.h"

namespace Insight
{
	namespace Graphics
	{
		void Renderer::Init(GPUDevice* gpuDevice)
		{
			m_gpuDevice = gpuDevice;
			m_swapchain = GPUSwapchain::Create();
			if (m_swapchain)
			{
				m_swapchain->Prepare();
				m_swapchain->Build(GPUSwapchainDesc(Window::Instance().GetWidth(), Window::Instance().GetHeight()));
				m_swapchain->Build(GPUSwapchainDesc(Window::Instance().GetWidth(), Window::Instance().GetHeight()));
			}
		}

		void Renderer::Destroy()
		{
			if (m_swapchain)
			{
				m_swapchain->Destroy();
				delete m_swapchain;
			}
		}

		void Renderer::Render()
		{
			Prepare();
			Submit();
		}

		void Renderer::Prepare()
		{
			m_swapchain->AcquireNextImage(nullptr);
		}

		void Renderer::Submit()
		{
			m_swapchain->Present(GPUQueue::GPUQueue_Graphics, { });
		}
	}
}