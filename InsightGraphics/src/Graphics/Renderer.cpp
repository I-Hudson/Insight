#include "Graphics/Renderer.h"
#include "Graphics/Window.h"
#include "Graphics/GPU/GPUDevice.h"
#include "Graphics/GPU/GPUSwapchain.h"
#include "Graphics/GPU/GPUImage.h"

namespace Insight
{
	namespace Graphics
	{
		void Renderer::Init(GPUDevice* gpuDevice)
		{
			m_gpuDevice = gpuDevice;
			m_presentCompleteSemaphore = m_gpuDevice->GetSemaphoreManager().GetOrCreateSemaphore();
			m_commandListManager.Create();
			m_gpuDevice->GetShaderManager().CreateShader("Swapchain", ShaderDesc("Resources/Shaders/Swapchain.vert", "Resources/Shaders/Swapchain.frag"));
		}

		void Renderer::Destroy()
		{
			if (m_presentCompleteSemaphore)
			{
				m_gpuDevice->GetSemaphoreManager().ReturnSemaphore(m_presentCompleteSemaphore);
			}

			m_commandListManager.Destroy();
			GPUImageManager::Instance().Destroy();
		}

		void Renderer::Render()
		{
			m_commandListManager.ResetCommandPool();
			GPUCommandList* cmdList = m_commandListManager.GetOrCreateCommandList();
			Prepare(cmdList);
			Submit(cmdList);
		}

		void Renderer::Prepare(GPUCommandList* cmdList)
		{
			m_gpuDevice->GetSwapchain()->AcquireNextImage(m_presentCompleteSemaphore);

			cmdList->BeginRecord();

			cmdList->SetPrimitiveTopologyType(PrimitiveTopologyType::TriangleList);
			cmdList->SetPolygonMode(PolygonMode::Fill);
			cmdList->SetCullMode(CullMode::Back);
			cmdList->SetViewport(Window::Instance().GetWidth(), Window::Instance().GetHeight());
			cmdList->SetScissor(Window::Instance().GetWidth(), Window::Instance().GetHeight());

			// Render to a GBuffer.
			//cmdList->SetShader(nullptr);
			//cmdList->AddRenderTarget(nullptr /*Colour*/);
			//cmdList->AddRenderTarget(nullptr /*Normal*/);
			//cmdList->AddRenderTarget(nullptr /*Depth*/);
			//cmdList->Draw(3, 1, 0, 0);

			// Render straight to swapchain.
			cmdList->SetCullMode(CullMode::None);
			cmdList->SetShader(m_gpuDevice->GetShaderManager().GetShader("Swapchain"));
			cmdList->ClearRenderTargets();
			cmdList->SetSwapchainSubmit(true);
			cmdList->SetViewport(Window::Instance().GetWidth(), Window::Instance().GetHeight());
			cmdList->Draw(3, 1, 0, 0);

			cmdList->EndRecord();

			cmdList->SubmitAndWait(GPUQueue_Graphics);
			m_gpuDevice->WaitForGPU();
		}

		void Renderer::Submit(GPUCommandList* cmdList)
		{
			m_gpuDevice->GetSwapchain()->Present(GPUQueue::GPUQueue_Graphics, { m_presentCompleteSemaphore });
		}
	}
}