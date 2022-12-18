#include "Graphics/GraphicsSystem.h"

#include "Graphics/RenderContext.h"
#include "Graphics/PixelFormatExtensions.h"

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
			if (Core::CommandLineArgs::GetCommandLineValue("graphicsapi")->GetString() == "vulkan")
			{
				graphcisAPI = Graphics::GraphicsAPI::Vulkan;
			}
			else if (Core::CommandLineArgs::GetCommandLineValue("graphicsapi")->GetString() == "dx12")
			{
				graphcisAPI = Graphics::GraphicsAPI::DX12;
			}
			else
			{
				graphcisAPI = Graphics::GraphicsAPI::DX12;
			}

			m_window.Init(m_inputSystem);

			m_context = Graphics::RenderContext::New(graphcisAPI);
			ASSERT(m_context);
			ASSERT(m_context->Init());
			m_renderGraph.Init(m_context);

			m_state = Core::SystemStates::Initialised;
		}

		void GraphicsSystem::Shutdown()
		{
			if (m_context)
			{
				m_context->GpuWaitForIdle();
				m_renderGraph.Release();
				m_context->Destroy();
				Delete(m_context);

				m_window.Destroy();
			}

			m_state = Core::SystemStates::Not_Initialised;
		}

		void GraphicsSystem::Update()
		{
			m_window.Update();
		}

		void GraphicsSystem::Render()
		{
			m_renderGraph.Execute();
		}
	}
}