#include "Graphics/Renderer.h"
#include "Graphics/Window.h"
#include "Graphics/GPU/GPUDevice.h"
#include "Graphics/GPU/GPUSwapchain.h"
#include "Graphics/GPU/GPUImage.h"
#include "Graphics/GPU/GPUBuffer.h"

#include <iostream>

namespace Insight
{
	namespace Graphics
	{
#define TRI_TEST
		void Renderer::Init(GPUDevice* gpuDevice)
		{
			m_gpuDevice = gpuDevice;
			m_presentCompleteSemaphore = GPUSemaphoreManager::Instance().GetOrCreateSemaphore();
			m_commandListManager.Create();
			GPUShaderManager::Instance().CreateShader("Swapchain", ShaderDesc("Resources/Shaders/Swapchain.vert", "Resources/Shaders/Swapchain.frag"));

#ifdef TRI_TEST
			GPUShaderManager::Instance().CreateShader("GBuffer", ShaderDesc("Resources/Shaders/GBuffer.vert", "Resources/Shaders/GBuffer.frag",
				{
					ShaderInputLayout(0, PixelFormat::R32G32B32A32_Float, 0),
					ShaderInputLayout(1, PixelFormat::R32G32B32A32_Float, 16),
				}
			));

			const int windowHalfWidth = Window::Instance().GetWidth() * 0.5f;
			const int windowHalfHeight = Window::Instance().GetHeight() * 0.5f;
			struct Vertex
			{
				glm::vec4 Pos;
				glm::vec4 Colour;
			};
			const int vertexCount = 3;
			GPUBuffer* vBuffer = GPUBufferManager::Instance().CreateBuffer("TRI_TEST_V", GPUBufferCreateInfo(sizeof(Vertex)* vertexCount, GPUBufferType::Vertex));
			Vertex vertices[vertexCount] =
			{
				{glm::vec4(-1, 1, 0, 1), glm::vec4(1, 0, 0, 1)},
				{glm::vec4(0, -1, 0, 1), glm::vec4(0, 1, 0, 1)},
				{glm::vec4(1, 1, 0, 1), glm::vec4(0, 0, 1, 1)}
			};
			std::cout << "Upload result: " << vBuffer->SetData(vertices, sizeof(Vertex) * vertexCount) << "\n";

			const int indexCount = 3;
			GPUBuffer* iBuffer = GPUBufferManager::Instance().CreateBuffer("TRI_TEST_I", GPUBufferCreateInfo(sizeof(u32) * indexCount, GPUBufferType::Index));
			u32 indices[indexCount] = { 0,1,2 };
			std::cout << "Upload result: " << iBuffer->SetData(indices, sizeof(u32) * indexCount) << "\n";
#endif
		}

		void Renderer::Destroy()
		{
			if (m_presentCompleteSemaphore)
			{
				GPUSemaphoreManager::Instance().ReturnSemaphore(m_presentCompleteSemaphore);
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
			//cmdList->AddRenderTarget(nullptr /*Colour*/);
			//cmdList->AddRenderTarget(nullptr /*Normal*/);
			//cmdList->AddRenderTarget(nullptr /*Depth*/);
			//cmdList->Draw(3, 1, 0, 0);

			// Render straight to swapchain.
			cmdList->SetCullMode(CullMode::None);
			cmdList->ClearRenderTargets();
			cmdList->SetSwapchainSubmit(true);
			cmdList->SetViewport(Window::Instance().GetWidth(), Window::Instance().GetHeight());
#ifdef TRI_TEST
			cmdList->SetShader(GPUShaderManager::Instance().GetShader("GBuffer"));
			cmdList->SetVertexBuffer(GPUBufferManager::Instance().GetBuffer("TRI_TEST_V"));
			cmdList->SetIndexBuffer(GPUBufferManager::Instance().GetBuffer("TRI_TEST_I"));
			cmdList->DrawIndexed(3, 1, 0, 0, 0);
#else

			cmdList->SetShader(GPUShaderManager::Instance().GetShader("Swapchain"));
			cmdList->Draw(3, 1, 0, 0);
#endif

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