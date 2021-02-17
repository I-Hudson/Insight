#include "ispch.h"
#include "GraphicsModule.h"
#include "Engine/Module/WindowModule.h"
#include "Engine/Instrumentor/Instrumentor.h"
#include "Engine/Component/MeshComponent.h"
#include "Engine/Graphics/ImGuiRenderer.h"

#include "Engine/GraphicsAPI/Vulkan/GPUDeviceVulkan.h"

#include "Engine/Time/Stopwatch.h"
#include "Engine/Core/Log.h"

// TESTING
#include "Engine/Graphics/Shaders/GPUShader.h"
#include "Engine/Graphics/RenderGraph/RenderGraph.h"
#include "Engine/Graphics/Image/GPUImage.h"
// TESTING

	namespace Module
	{
		CameraComponent* GraphicsModule::m_mainCamera;
		std::vector<MeshComponent*> GraphicsModule::m_meshs;

		GraphicsModule::GraphicsModule()
		{
			// TESTING
			GPUDevice* gpuDevice = GPUDeviceVulkan::New();

			GPUShader* shader = GPUShader::New();
			shader->SetStage(ShaderStage::Vertex, "./data/shaders/vulkan/default.vert", ShaderStageInput::FilePath);
			shader->Compile();
			::Delete(shader);

			GPUImage* image = GPUImage::New();
			image->Init(GPUImageDescription::RenderTarget(1920, 1080, PixelFormat::R8G8B8A8_SNorm));
			GPUImageView* view = GPUImageView::New();
			view->Init(image);

			::Delete(image);
			::Delete(view);

			ResourceDimensions swapchain;
			swapchain.Format = PixelFormat::R8G8B8A8_UNorm_sRGB;
			swapchain.Width = 1920;
			swapchain.Height = 1080;
			swapchain.Samples = 4;
			swapchain.Levels = 1;
			swapchain.Layers = 1;
			swapchain.ImageUsage = (U32)ImageUsageFlagsBits::Color_Attachment;

			ImageAttachmentInfo color, hdrLighting;
			color.Format = PixelFormat::R8G8B8A8_UNorm_sRGB;
			hdrLighting.Format = PixelFormat::R32G32B32A32_SInt;

			RenderGraph* graph = RenderGraph::New();
			graph->SetSwapchainDimensions(swapchain);

			auto& lPass = graph->AddPass("Lightting", RenderGraphQueueFlagsBits::RENDER_GRAPH_QUEUE_GRAPHICS_BIT);
			lPass.AddColorOutput("HDR-Lightting-Image", hdrLighting);
			lPass.AddAttachmentInput("color");

			auto& gPass = graph->AddPass("g-buffer", RenderGraphQueueFlagsBits::RENDER_GRAPH_QUEUE_GRAPHICS_BIT);
			gPass.AddColorOutput("color", color);

			graph->SetbackBufferSource("HDR-Lightting-Image");
			graph->Build();
			graph->SetupAttachments();
			graph->EnqueueRenderPass();

			graph->LogToConsole();
			graph->Reset();

			GPUDevice::Instance()->Dispose();
			::Delete(GPUDevice::Instance());

			// TESTING

			m_renderer = Renderer::New();
			m_renderer->Init();

			m_imguiRenderer = ImGuiRenderer::New();
			ImGuiRenderer::Instance()->Init(m_renderer);
		}

		GraphicsModule::~GraphicsModule()
		{
			ImGuiRenderer* imguiRenderer = ImGuiRenderer::Instance();
			::Delete(m_imguiRenderer);

			::Delete(m_renderer);
		}

		void GraphicsModule::Update(const float& deltaTime)
		{
			IS_PROFILE_FUNCTION();

			if (m_renderer != nullptr)
			{
				m_renderer->Render(m_mainCamera, m_meshs);
			}
			else
			{
				IS_CORE_ERROR("[GraphicsModule::Update] No renderer setup.");
			}
		}

		void GraphicsModule::WaitForIdle()
		{
			m_renderer->WaitForIdle();
		}

		GraphicsAPI GraphicsModule::GetAPI()
		{
			return m_renderer->GetAPI();
		}

		void GraphicsModule::SetMainCamera(CameraComponent* camera)
		{
			m_mainCamera = camera;
		}

		const bool GraphicsModule::HasMainCamera()
		{
			return m_mainCamera != nullptr;
		}

		const bool GraphicsModule::IsThisMainCamera(CameraComponent* camera)
		{
			return camera == m_mainCamera;
		}

		Material* GraphicsModule::GetDefaultMaterial()
		{
			return GraphicsModule::Instance()->m_renderer->GetDefaultMaterial();
		}
	}