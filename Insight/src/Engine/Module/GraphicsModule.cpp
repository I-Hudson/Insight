#include "ispch.h"
#include "Engine/Module/GraphicsModule.h"
#include "Engine/Module/WindowModule.h"
#include "Engine/Instrumentor/Instrumentor.h"
#include "Engine/Component/MeshComponent.h"
#include "Engine/Component/CameraComponent.h"
#include "Engine/Graphics/ImGuiRenderer.h"
#include "Engine/Config/Config.h"
#include "Engine/Scene/Scene.h"

#include "Engine/GraphicsAPI/Vulkan/GPUDeviceVulkan.h"

#include "Engine/Time/Stopwatch.h"
#include "Engine/Core/Log.h"

#include "Engine/Graphics/Shaders/GPUShader.h"
#include "Engine/Graphics/RenderGraph/RenderGraph.h"
#include "Engine/Graphics/Image/GPUImage.h"
#include "Engine/Graphics/GPUCommandBuffer.h"
#include "Engine/Graphics/GPUDescriptorSet.h"
#include "Engine/Graphics/GPUBuffer.h"
#include "Engine/Graphics/GPUDynamicBuffer.h"
#include "Engine/Graphics/GPUSync.h"
#include "stb_image.h"

namespace Module
{
	CameraComponent* GraphicsModule::m_mainCamera;
	std::vector<MeshComponent*> GraphicsModule::m_meshs;

	GraphicsModule::GraphicsModule()
	{
		PixelFormatExtensions::Init();
		GPUDevice* gpuDevice = GPUDeviceVulkan::New();

		{
			using namespace Insight;

			Graphics::GPUShader* shader = Graphics::GPUShader::New();
			shader->SetStage(ShaderStage::Vertex, "./data/shaders/vulkan/default.vert", Graphics::ShaderStageInput::FilePath);
			shader->Compile();
			::Delete(shader);

			Insight::Graphics::GPUImage* image = Insight::Graphics::GPUImage::New();
			image->Init(Insight::Graphics::GPUImageDesc::Texture(1, SampleLevel::None, PixelFormat::R8G8B8A8_UNorm, "./data/embed2.jpg"));
			Insight::Graphics::GPUImageView* view = Insight::Graphics::GPUImageView::New();
			view->Init(image);
			::Delete(image);
			::Delete(view);

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
			vec4 = glm::vec4(10, 20, 30, 40);
			gpuBuffer->SetData(&vec4, sizeof(glm::vec4));

			///
			// Get data from a GPU buffer
			///
			gpuBuffer->GetData(gpuBufferData);
			dataPtr = gpuBufferData.data();
			returnedValue = *static_cast<glm::vec4*>(dataPtr);

			cmdBuffer->BeginRecord();
			std::vector<Graphics::GPUBuffer*> vBuffers = { gpuBuffer };
			std::vector<u32> vOffsets = { 0 };
			cmdBuffer->BindVertexBuffers(0, 1, vBuffers.data(), vOffsets.data());

			vBuffers = { gpuBuffer, gpuBuffer1 };
			vOffsets = { 0, 0 };
			cmdBuffer->BindVertexBuffers(0, 2, vBuffers.data(), vOffsets.data());

			cmdBuffer->EndRecord();

			Graphics::GPUDynamicBuffer* dyanmicBuffer = Graphics::GPUDynamicBuffer::New();
			dyanmicBuffer->Init(Graphics::GPUDynamicBufferDesc::Uniform(64, 256, 1000));
			dyanmicBuffer->SetName("Dynamic Uniform Buffer");

			Graphics::GPUBuffer* sub1 = dyanmicBuffer->Upload(&vec4, 16);
			std::vector<u8> vector; vector.resize(128);
			Graphics::GPUBuffer* sub2 = dyanmicBuffer->Upload(vector.data(), sizeof(u8) * vector.size());
			dyanmicBuffer->Reset();

			::Delete(dyanmicBuffer);
			::Delete(gpuBuffer);
			::Delete(gpuBuffer1);
			::Delete(cmdBuffer);
			::Delete(cmdPool);


		}
		Insight::Graphics::RenderGraph* graph = Insight::Graphics::RenderGraph::New();


		m_imguiRenderer = ImGuiRenderer::New();
	}

	GraphicsModule::~GraphicsModule()
	{
		GPUDevice::Instance()->WaitForGPU();

		if (ImGuiRenderer::IsInitialised())
		{
			ImGuiRenderer* imguiRenderer = ImGuiRenderer::Instance();
			::Delete(m_imguiRenderer);
		}

		::Delete(Insight::Graphics::RenderGraph::Instance());
		GPUDevice::Instance()->Dispose();
		::Delete(GPUDevice::Instance());
	}

	u32 imageIndex = 0;
	void GraphicsModule::Update(const float& deltaTime)
	{
		IS_PROFILE_FUNCTION();
		{
			IS_PROFILE_SCOPE("RenderGraph: Create");

			using namespace Insight;
			Graphics::RenderGraph::Instance()->Reset();

			Graphics::ImageAttachmentInfo mainPassOutput = { };
			mainPassOutput.Width = Window::GetWidth();
			mainPassOutput.Height = Window::GetHeight();
			mainPassOutput.Name = "mainPass-Color";
			mainPassOutput.Format = PixelFormat::R8G8B8A8_UNorm;

			Graphics::ImageAttachmentInfo depthOutput = {};
			depthOutput.Width = Window::GetWidth();
			depthOutput.Height = Window::GetHeight();
			depthOutput.Name = "depth";
			depthOutput.Format = PixelFormat::D24_UNorm_S8_UInt;

			auto& shadowPass = Graphics::RenderGraph::Instance()->AddPass("shadowPass", Graphics::RenderGraphQueueFlagsBits::RENDER_GRAPH_QUEUE_GRAPHICS_BIT);
			shadowPass.SetDepthStencilOutput("shaderDepthStencil", depthOutput);
			shadowPass.SetClearDepthStencil(glm::vec2(1, 0));

			// Add all my passes at runtime.
			auto& mainPass = Graphics::RenderGraph::Instance()->AddPass("MainPass", Graphics::RenderGraphQueueFlagsBits::RENDER_GRAPH_QUEUE_GRAPHICS_BIT);
			mainPass.AddColorOutput("colour", mainPassOutput);
			mainPass.AddColorOutput("normal", mainPassOutput);
			mainPass.AddColorOutput("position", mainPassOutput);
			mainPass.SetDepthStencilOutput("shaderDepthStencil", depthOutput);

			mainPass.SetClearDepthStencil(glm::vec2(1.0f, 0.0f));
			mainPass.SetClearColour(glm::vec4(0, 1, 0, 1));
			mainPass.SetRenderFunc([](Graphics::GPUCommandBuffer* cmdBuffer, Graphics::FrameBufferResources& buffers, Graphics::GPUDescriptorBuilder* builder, Graphics::RenderPass& pass)
			{
				IS_PROFILE_SCOPE("MainPassRenderFunc");
				struct UBO
				{
					glm::mat4 Proj;
					glm::mat4 View;
					glm::mat4 Model;
					glm::vec4 LightPos;
				};

				// bind material
				// bind buffers
				// Draw mesh, 

				UBO ubo = {
					m_mainCamera->GetProjMatrix(),
					m_mainCamera->GetViewMatrix(),
					glm::mat4(1.0f),
					glm::vec4(5.0f, 5.0f, 5.0f, 1.0f)
				};
				ubo.Proj[1][1] *= -1;
				Graphics::GPUBuffer* uboBuffer = buffers.at(Graphics::GPUBufferFlags::UNIFORM)->Upload(&ubo, sizeof(ubo));

				glm::mat4 modelMatrix(1.0f);
				Graphics::GPUBuffer* modelBuffer = buffers.at(Graphics::GPUBufferFlags::UNIFORM)->Upload(&modelMatrix, sizeof(glm::mat4));

				int textureDiffuse = 1;
				Graphics::GPUBuffer* textureDiffuseBuffer = buffers.at(Graphics::GPUBufferFlags::UNIFORM)->Upload(&textureDiffuse, sizeof(int));

				Graphics::GPUImage* diffuseTexture = nullptr;
				Utils::Hasher diffuseTextureHasher;
				diffuseTextureHasher.Hash("./data/models/nano/body_dif.png");
				if (Graphics::GPUImageCache::Instance()->GetItem(diffuseTextureHasher.GetHash(), diffuseTexture))
				{
					diffuseTexture->Init(Graphics::GPUImageDesc::Texture(1, SampleLevel::None, PixelFormat::R8G8B8A8_UNorm, "./data/models/nano/body_dif.png"));
				}

				Graphics::GPUImageView* diffuseTextureView = Graphics::GPUImageView::New();
				pass.AddLifeTimeObject(diffuseTextureView);
				diffuseTextureView->Init(diffuseTexture);

				Graphics::GPUShader* defaultShader = nullptr;
				if (Graphics::GPUShaderCache::Instance()->GetItem(0, defaultShader))
				{
					IS_PROFILE_SCOPE("Default shader create");
					defaultShader->SetStage(ShaderStage::Vertex, "./data/shaders/vulkan/default.vert", Graphics::ShaderStageInput::FilePath);
					defaultShader->SetStage(ShaderStage::Fragment, "./data/shaders/vulkan/default.frag", Graphics::ShaderStageInput::FilePath);
					defaultShader->Compile();
				}

				Graphics::GPUPipeline* defaultPipeline = Graphics::GPUPipeline::New();
				{
					IS_PROFILE_SCOPE("Default pipeline create and bind");
					pass.AddLifeTimeObject(defaultPipeline);
					defaultPipeline->SetShader(defaultShader);
					defaultPipeline->Init(pass.GetGraphPass(), Graphics::GPUPipelineDesc(PrimitiveTopologyType::Triangle_List, PolygonMode::Fill, CullMode::Back, FrontFace::Counter_Clockwise));
					cmdBuffer->BindPipeline(PipelineBindPoint::Graphics, defaultPipeline);
				}

				Graphics::GPUDescriptorSet* vertexSet = Graphics::GPUDescriptorSet::New();
				Graphics::GPUDescriptorSet* fragSet = Graphics::GPUDescriptorSet::New();
				{
					IS_PROFILE_SCOPE("Build descriptor set");
					builder->BindBuffer(0, uboBuffer, DescriptorType::Unifom_Buffer, ShaderStage::Vertex)
						->BindBuffer(1, modelBuffer, DescriptorType::Unifom_Buffer, ShaderStage::Vertex)->Build(vertexSet);

					builder->BindImage(0, diffuseTexture, DescriptorType::Combined_Image_Sampler, ShaderStage::Fragment)
						->BindBuffer(1, textureDiffuseBuffer, DescriptorType::Unifom_Buffer, ShaderStage::Fragment)->Build(fragSet);
				}
				Graphics::GPUDescriptorSet* sets[] = { vertexSet, fragSet };
				cmdBuffer->BindDescriptorSets(PipelineBindPoint::Graphics, defaultPipeline, 0, ARRAY_COUNT(sets), sets, 0, nullptr);

				{
					IS_PROFILE_SCOPE("Upload mesh vertices");
					for (auto& mesh : Scene::ActiveScene()->GetAllComponents<MeshComponent>())
					{
						if (mesh.GetMesh() != nullptr)
						{
							for (u32 subMeshIndex = 0; subMeshIndex < mesh.GetMesh()->GetMeshSubCount(); ++subMeshIndex)
							{
								SubMesh& subMesh = mesh.GetMesh()->GetSubMesh(subMeshIndex);

								u32 offsets[] = { 0 };
								Graphics::GPUBuffer* verticesBuffer[] = { subMesh.GetGPUVerticesBuffer() };
								cmdBuffer->BindVertexBuffers(0, 1, verticesBuffer, offsets);
								cmdBuffer->BindIndexBuffer(subMesh.GetGPUIndexBuffer(), 0, Graphics::GPUCommandBufferIndexType::UINT32);
								cmdBuffer->DrawIndexed(subMesh.GetIndicesCount(), 1, 0, 0, 0);
								//			auto* subMesh = mesh->GetMesh()->GetSubMesh(subMeshIndex);
								//			auto vertices = subMesh->GetVertices();
								//			Graphics::GPUBuffer* meshBuffer[] = { buffers.at(Graphics::GPUBufferFlags::VERTEX)->Upload(vertices.data(), sizeof(Vertex) * vertices.size()) };
								//			auto indices = subMesh->GetIndices();
								//			Graphics::GPUBuffer* indexBuffer = buffers.at(Graphics::GPUBufferFlags::INDEX)->Upload(indices.data(), sizeof(u32) * indices.size());

								//			u32 offsets[] = { 0 };
								//			cmdBuffer->BindVertexBuffers(0, 1, meshBuffer, offsets);
								//			cmdBuffer->BindIndexBuffer(indexBuffer, 0, Graphics::GPUCommandBufferIndexType::UINT32);
							}
						}
					}
				}

				::Delete(vertexSet);
				::Delete(fragSet);
			});
		}

		Insight::Graphics::RenderGraph::Instance()->SetbackBufferSource("colour");
		++imageIndex;

		m_imguiRenderer->EndFrame();
		m_imguiRenderer->Render();

		Insight::Graphics::RenderGraph::Instance()->Build();
		Insight::Graphics::RenderGraph::Instance()->LogToConsole();
		Insight::Graphics::RenderGraph::Instance()->Execute();
	}

	void GraphicsModule::WaitForIdle()
	{
		GPUDevice::Instance()->WaitForGPU();
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
}