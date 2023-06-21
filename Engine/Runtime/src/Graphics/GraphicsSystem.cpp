#include "Graphics/GraphicsSystem.h"

#include "Graphics/RenderContext.h"
#include "Graphics/PixelFormatExtensions.h"

#include "Runtime/CommandLineDefines.h"
#include "Runtime/Engine.h"

#include "Core/CommandLineArgs.h"

namespace Insight
{
	namespace Runtime
	{
		GraphicsSystem::GraphicsSystem()
		{ }

		GraphicsSystem::~GraphicsSystem()
		{ }

		void GraphicsSystem::Initialise(Input::InputSystem* inputSystem)
		{
			m_inputSystem = inputSystem;
			PixelFormatExtensions::Init();

			Graphics::GraphicsAPI graphcisAPI = Graphics::GraphicsAPI::None;
			std::string graphicsAPI_CMD = Core::CommandLineArgs::GetCommandLineValue("graphicsapi")->GetString();

			if (graphicsAPI_CMD == "vulkan"
				|| graphicsAPI_CMD == "v")
			{
				graphcisAPI = Graphics::GraphicsAPI::Vulkan;
			}
			else if (graphicsAPI_CMD == "dx12")
			{
				graphcisAPI = Graphics::GraphicsAPI::DX12;
			}
			else
			{
				graphcisAPI = Graphics::GraphicsAPI::DX12;
			}

			m_window.Init(m_inputSystem, true);

			InitialiseRenderContext(graphcisAPI);

			m_state = Core::SystemStates::Initialised;
		}

		void GraphicsSystem::Shutdown()
		{
			if (m_context)
			{
				m_context->GpuWaitForIdle();

				m_context->Destroy();
				Delete(m_context);

				m_window.Destroy();
			}

			m_state = Core::SystemStates::Not_Initialised;
		}

		void GraphicsSystem::Update()
		{
			//m_window.Update();
		}

		void GraphicsSystem::Render()
		{
			m_context->Render();
		}

		void GraphicsSystem::CreateRenderFrame()
		{
			WorldSystem* worldSystem = App::Engine::Instance().GetSystemRegistry().GetSystem<WorldSystem>();
			if (worldSystem)
			{
				m_renderFrame.CreateRenderFrameFromWorldSystem(worldSystem);
			}
		}

		const RenderFrame& GraphicsSystem::GetRenderFrame() const
		{
			return m_renderFrame;
		}

		void GraphicsSystem::InitialiseRenderContext(Graphics::GraphicsAPI graphicsAPI)
		{
			m_context = Graphics::RenderContext::New(graphicsAPI);

			Graphics::RenderContextDesc renderContextDesc = {};
			if (Core::CommandLineArgs::CommandListExists(CMD_GPU_VALIDATION))
			{
				renderContextDesc.GPUValidation = Core::CommandLineArgs::GetCommandLineValue(CMD_GPU_VALIDATION)->GetBool();
			}

			renderContextDesc.MultithreadContext = true;
			if (!m_context->Init(renderContextDesc))
			{
				m_context->Destroy();
				exit(-1);
			}

			Graphics::SwapchainDesc swapchainDesc = {};
			swapchainDesc.Width = m_window.GetWidth();
			swapchainDesc.Height = m_window.GetHeight();
			swapchainDesc.Format = PixelFormat::R8G8B8A8_UNorm;
			swapchainDesc.PresentMode = Graphics::SwapchainPresentModes::Variable;
			m_context->CreateSwapchain(swapchainDesc);

			m_context->InitImGui();
		}
	}
}