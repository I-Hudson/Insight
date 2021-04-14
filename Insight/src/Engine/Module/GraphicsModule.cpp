#include "ispch.h"
#include "Engine/Module/GraphicsModule.h"
#include "Engine/Module/WindowModule.h"
#include "Engine/Instrumentor/Instrumentor.h"
#include "Engine/Component/MeshComponent.h"
#include "Engine/Graphics/ImGuiRenderer.h"
#include "Engine/Config/Config.h"

#include "Engine/GraphicsAPI/Vulkan/GPUDeviceVulkan.h"

#include "Engine/Time/Stopwatch.h"
#include "Engine/Core/Log.h"

// TESTING
#include "Engine/Graphics/Shaders/GPUShader.h"
#include "Engine/Graphics/RenderGraph/RenderGraph.h"
#include "Engine/Graphics/Image/GPUImage.h"
#include "Engine/Graphics/GPUCommandBuffer.h"
#include "Engine/Graphics/GPUDescriptorSet.h"
#include "Engine/Graphics/GPUBuffer.h"
#include "stb_image.h"
#define RENDER_GRAPH_TESTING 1
// TESTING

namespace Module
{
	CameraComponent* GraphicsModule::m_mainCamera;
	std::vector<MeshComponent*> GraphicsModule::m_meshs;

	GraphicsModule::GraphicsModule()
	{
#if RENDER_GRAPH_TESTING
		{
			PixelFormatExtensions::Init();

			// TESTING
			GPUDevice* gpuDevice = GPUDeviceVulkan::New();

			GPUShader* shader = GPUShader::New();
			shader->SetStage(ShaderStage::Vertex, "./data/shaders/vulkan/default.vert", ShaderStageInput::FilePath);
			shader->Compile();
			::Delete(shader);

			Insight::Graphics::GPUImage* image = Insight::Graphics::GPUImage::New();
			image->Init(Insight::Graphics::GPUImageDesc::Texture(1, 0, PixelFormat::R8G8B8A8_UNorm, "./data/embed2.jpg"));
			Insight::Graphics::GPUImageView* view = Insight::Graphics::GPUImageView::New();
			view->Init(image);
			::Delete(image);
			::Delete(view);

			{
				using namespace Insight;
				Graphics::GPUCommandPool* cmdPool = Graphics::GPUCommandPool::New();
				cmdPool->Init(Graphics::GPUCommandPoolDesc(Graphics::GPUCommandPoolFlags::TRANSIENT, GPUQueue::GRAPHICS));
				auto* cmdBuffer = cmdPool->AllocateCommandBuffer(Graphics::GPUCommandBufferDesc::CreateOneTimeCmdBuffer());

				Graphics::GPUBuffer* gpuBuffer = Graphics::GPUBuffer::New();
				Graphics::GPUBuffer* gpuBuffer1 = Graphics::GPUBuffer::New();
				glm::vec4 vec4 = glm::vec4(1.0f, 2.0f, 3.0f, 4.0f);
				Graphics::GPUBufferDesc desc = Graphics::GPUBufferDesc::Vertex(16, 1, &vec4);
				gpuBuffer->Init(desc);
				desc.Flags = Graphics::GPUBufferFlags::VERTEX | Graphics::GPUBufferFlags::TRANSFER_SRC;
				gpuBuffer1->Init(desc);

				///
				// Get data from a GPU buffer
				///
				std::vector<Byte> gpuBufferData;
				gpuBuffer->GetData(gpuBufferData);
				void* dataPtr = gpuBufferData.data();
				glm::vec4 returnedValue = *static_cast<glm::vec4*>(dataPtr);

				///
				// Set data to a GPU buffer
				///
				vec4 = glm::vec4(10, 20,30,40);
				gpuBuffer->SetData(&vec4, sizeof(glm::vec4));

				///
				// Get data from a GPU buffer
				///
				gpuBuffer->GetData(gpuBufferData);
				dataPtr = gpuBufferData.data();
				returnedValue = *static_cast<glm::vec4*>(dataPtr);

				cmdBuffer->BeginRecord();
				cmdBuffer->SetViewPort(Maths::Rect(1920, 1080, 0, 1));
				std::vector<Graphics::GPUBuffer*> vBuffers = { gpuBuffer};
				std::vector<u32> vOffsets = { 0 };
				cmdBuffer->BindVertexBuffers(0, 1, vBuffers.data(), vOffsets.data());

				vBuffers = { gpuBuffer, gpuBuffer1 };
				vOffsets = { 0, 0 };
				cmdBuffer->BindVertexBuffers(0, 2, vBuffers.data() , vOffsets.data());

				cmdBuffer->EndRecord();

				::Delete(gpuBuffer);
				::Delete(gpuBuffer1);
				::Delete(cmdBuffer);
				::Delete(cmdPool);


			}

			{
				using namespace Insight::Graphics;
				RenderGraph* graph = RenderGraph::New();
				
				//while (appRunning)
				//{
					graph->Reset();

					ImageAttachmentInfo mainPassOutput = { };
					mainPassOutput.Width = 2560;
					mainPassOutput.Height = 1440;
					mainPassOutput.Name = "mainPass-Color";
					mainPassOutput.Format = PixelFormat::R8G8B8A8_UNorm;

					ImageAttachmentInfo depthOutput = {};
					depthOutput.Width = 1920;
					depthOutput.Height = 1080;
					depthOutput.Name = "depth";
					depthOutput.Format = PixelFormat::D24_UNorm_S8_UInt;

					auto& lightingPass = graph->AddPass("LightingPass", RenderGraphQueueFlagsBits::RENDER_GRAPH_QUEUE_GRAPHICS_BIT);
					lightingPass.AddColorInput("colour");
					lightingPass.AddColorInput("normal");
					lightingPass.AddColorInput("position");
					lightingPass.AddColorInput("pointLights");
					lightingPass.SetDepthStencilInput("shaderDepthStencil");
					lightingPass.AddColorOutput("lightingOutput", mainPassOutput);

					auto& shadowPass = graph->AddPass("ShaderPass", RenderGraphQueueFlagsBits::RENDER_GRAPH_QUEUE_GRAPHICS_BIT);
					shadowPass.SetDepthStencilOutput("shaderDepthStencil", depthOutput);

					// Add all my passes at runtime.
					auto& mainPass = graph->AddPass("MainPass", RenderGraphQueueFlagsBits::RENDER_GRAPH_QUEUE_GRAPHICS_BIT);
					mainPass.AddColorOutput("colour", mainPassOutput);
					mainPass.AddColorOutput("normal", mainPassOutput);
					mainPass.AddColorOutput("position", mainPassOutput);
					mainPass.SetDepthStencilInput("shaderDepthStencil");

					mainPass.SetClearColour(glm::vec4());
					mainPass.SetRenderFunc([&]()
					{
						// bind material
						// bind buffers
						// Draw mesn, 
					});

					auto& pointLights = graph->AddPass("pointLights", RenderGraphQueueFlagsBits::RENDER_GRAPH_QUEUE_GRAPHICS_BIT);
					pointLights.SetDepthStencilInput("shaderDepthStencil");
					pointLights.AddColorOutput("pointLights", mainPassOutput);

					graph->SetbackBufferSource("lightingOutput");
					graph->Build();
					graph->Execute();

					::Delete(graph);

				//}
			}


			//ResourceDimensions swapchain;
			//swapchain.Format = PixelFormat::R8G8B8A8_UNorm_sRGB;
			//swapchain.Width = 1920;
			//swapchain.Height = 1080;
			//swapchain.Samples = 4;
			//swapchain.Levels = 1;
			//swapchain.Layers = 1;
			//swapchain.ImageUsage = (U32)ImageUsageFlagsBits::Color_Attachment;

			//ImageAttachmentInfo color, hdrLighting;
			//color.Format = PixelFormat::R8G8B8A8_UNorm_sRGB;
			//hdrLighting.Format = PixelFormat::R32G32B32A32_SInt;

			//RenderGraph* graph = RenderGraph::New();
			//graph->SetSwapchainDimensions(swapchain);

			//auto& lPass = graph->AddPass("Lightting", RenderGraphQueueFlagsBits::RENDER_GRAPH_QUEUE_GRAPHICS_BIT);
			//lPass.AddColorOutput("HDR-Lightting-Image", hdrLighting);
			//lPass.AddAttachmentInput("color");

			//auto& gPass = graph->AddPass("g-buffer", RenderGraphQueueFlagsBits::RENDER_GRAPH_QUEUE_GRAPHICS_BIT);
			//gPass.AddColorOutput("color", color);

			//graph->SetbackBufferSource("HDR-Lightting-Image");
			//graph->Build();
			//graph->SetupAttachments();
			//graph->EnqueueRenderPass();

			//graph->LogToConsole();
			//::Delete(graph);
		}
#endif
		m_renderer = Renderer::New();
		m_renderer->Init();

		m_imguiRenderer = ImGuiRenderer::New();
		ImGuiRenderer::Instance()->Init(m_renderer);

	}

	GraphicsModule::~GraphicsModule()
	{
#if RENDER_GRAPH_TESTING
		// TESTING
		GPUDevice::Instance()->Dispose();
		::Delete(GPUDevice::Instance());
#endif
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
		if (m_renderer)
		{
			m_renderer->WaitForIdle();
		}
	}

	GraphicsRendererAPI GraphicsModule::GetAPI()
	{
		return (GraphicsRendererAPI)CONFIG_VAL(GraphicsConfig.GraphicsAPI);
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