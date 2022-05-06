#include "Graphics/Renderpass.h"
#include "Graphics/RenderContext.h"
#include "Graphics/Window.h"
#include "Graphics/GraphicsManager.h"

#include "Tracy.hpp"

namespace Insight
{
	namespace Graphics
	{
		void VulkanTest()
		{
			ShaderDesc shaderDesc;
			shaderDesc.VertexFilePath = L"Resources/Shaders/hlsl/Swapchain.hlsl";
			shaderDesc.PixelFilePath = L"Resources/Shaders/hlsl/Swapchain.hlsl";
			RHI_Shader* shader = Renderer::GetShader(shaderDesc);

			PipelineStateObject pso{};
			pso.Name = L"Swapchain_PSO";
			pso.Shader = shader;
			pso.CullMode = CullMode::None;
			pso.RenderTargets.clear();
			pso.Swapchain = true;
			Renderer::SetPipelineStateObject(pso);

			Renderer::SetViewport(Window::Instance().GetWidth(), Window::Instance().GetHeight());
			Renderer::SetScissor(Window::Instance().GetWidth(), Window::Instance().GetHeight());

			Renderer::Draw(3, 1, 0, 0);
		}

		void Renderpass::Render()
		{
			ZoneScoped;
			if (GraphicsManager::IsVulkan())
			{
				VulkanTest();
			}
			else
			{
				Sample();
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}

		void Renderpass::Sample()
		{
			RHI_Buffer* vBuffer = Renderer::CreateVertexBuffer(128);

			ShaderDesc shaderDesc;
			shaderDesc.VertexFilePath = L"Resources/Shaders/hlsl/Swapchain.hlsl";
			shaderDesc.PixelFilePath = L"Resources/Shaders/hlsl/Swapchain.hlsl";
			RHI_Shader* shader = Renderer::GetShader(shaderDesc);

			PipelineStateObject pso{};
			pso.Name = L"Swapchain_PSO";
			pso.Shader = shader;
			pso.CullMode = CullMode::None;
			pso.RenderTargets.clear();
			pso.Swapchain = true;
			Renderer::SetPipelineStateObject(pso);

			Renderer::SetViewport(Window::Instance().GetWidth(), Window::Instance().GetHeight());
			Renderer::SetScissor(Window::Instance().GetWidth(), Window::Instance().GetHeight());

			glm::vec4 swapchainColour = { 0,0,1,1 };
			Renderer::SetUniform(0, 0, &swapchainColour, sizeof(swapchainColour));

			Renderer::Draw(3, 1, 0, 0);

			Renderer::FreeVertexBuffer(vBuffer);
		}
	}
}