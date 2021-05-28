#include "ispch.h"
#include "Engine/Module/GraphicsModule.h"
#include "Engine/Core/Application.h"
#include "Engine/Module/WindowModule.h"
#include "Engine/Instrumentor/Instrumentor.h"
#include "Engine/Component/MeshComponent.h"
#include "Engine/Component/TransformComponent.h"
#include "Engine/Component/CameraComponent.h"
#include "Engine/Component/DirectionalLightComponent.h"
#include "Engine/Graphics/ImGuiRenderer.h"
#include "Engine/Config/Config.h"
#include "Engine/Scene/Scene.h"

#include "Engine/GraphicsAPI/Vulkan/GPUDeviceVulkan.h"

#include "Engine/Time/Stopwatch.h"
#include "Engine/Core/Log.h"

#include "Engine/Graphics/RenderList.h"
#include "Engine/Module/GraphicsModule.h"
#include "Engine/Graphics/Debug/Gizmos.h"
#include "Engine/Graphics/Model/Model.h"
#include "Engine/Graphics/Shaders/GPUShader.h"
#include "Engine/Graphics/RenderGraph/RenderGraph.h"
#include "Engine/Graphics/Image/GPUImage.h"
#include "Engine/Graphics/GPUCommandBuffer.h"
#include "Engine/Graphics/GPUDescriptorSet.h"
#include "Engine/Graphics/GPUBuffer.h"
#include "Engine/Graphics/GPUDynamicBuffer.h"
#include "Engine/Graphics/GPUSync.h"


#include "stb_image.h"
#include "glm/gtc/matrix_transform.hpp"
#include "imgui.h"

// TODO: Remove this.
#include "Engine/GraphicsAPI/Vulkan/VulkanUtils.h"
//

namespace Insight::Module
{
	//glm::mat4 DirectionToViewMatrix(const glm::vec3& vec, const glm::vec3& position = glm::vec3(0))
	//{
	//	glm::vec3 const f(vec);
	//	glm::vec3 const s(glm::normalize(glm::cross(glm::vec3(0, 1, 0), f)));
	//	glm::vec3 const u(cross(f, s));

	//	glm::mat4 Result(1);
	//	Result[0][0] = s.x;
	//	Result[1][0] = s.y;
	//	Result[2][0] = s.z;
	//	Result[0][1] = u.x;
	//	Result[1][1] = u.y;
	//	Result[2][1] = u.z;
	//	Result[0][2] = -f.x;
	//	Result[1][2] = -f.y;
	//	Result[2][2] = -f.z;
	//	Result[3][0] = -glm::dot(s, position);
	//	Result[3][1] = -glm::dot(u, position);
	//	Result[3][2] = glm::dot(f, position);
	//	return Result;
	//}

	GraphicsModule::GraphicsModule()
	{ }

	GraphicsModule::~GraphicsModule()
	{
		Graphics::GPUDevice::Instance()->WaitForGPU();

		if (ImGuiRenderer::IsInitialised())
		{
			ImGuiRenderer* imguiRenderer = ImGuiRenderer::Instance();
			::Delete(m_imguiRenderer);
		}

		::Delete(Insight::Graphics::RenderGraph::Instance());
		Graphics::GPUDevice::Instance()->Dispose();
		::Delete(Graphics::GPUDevice::Instance());
	}

	void GraphicsModule::OnCreate()
	{
		PixelFormatExtensions::Init();
		Graphics::GPUDevice* gpuDevice = Graphics::GPUDevice::New();
		gpuDevice->Init();

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

		m_state = ModuleState::Running;
	}

	u32 imageIndex = 0;
	bool shadowMapFilter = false;

	void GraphicsModule::Update(const float& deltaTime)
	{
		if (Application::GetState() == ApplicationState::Init)
		{
			if (ImGuiRenderer::Instance()->IsInit())
			{
				ImGui::Begin("Graphics Module-Loading");
				ImGui::Text("Application is loading.");
				ImGui::End();
			}

			// The engine is still loading. We don't want to do any heavy rendering.
			Graphics::RenderGraph::Instance()->Reset();
			Graphics::GPUDevice::Instance()->BeginFrame();
			InitLoading();
			m_imguiRenderer->EndFrame();
			m_imguiRenderer->Render();
			Insight::Graphics::RenderGraph::Instance()->Build();
			//Insight::Graphics::RenderGraph::Instance()->LogToConsole();
			Insight::Graphics::RenderGraph::Instance()->Execute();
			Graphics::GPUDevice::Instance()->EndFrame();

			return;
		}

		IS_PROFILE_FUNCTION();
		{
			IS_PROFILE_SCOPE("RenderGraph: Create");

			Graphics::RenderList* renderList = Graphics::RenderList::GetFromPool();
			Scene::ActiveScene()->OnDraw(renderList);

			using namespace Insight;
			Graphics::RenderGraph::Instance()->Reset();
			Graphics::GPUDevice::Instance()->BeginFrame();

			ShadowMap();
			Deffered();
#ifdef IS_EDITOR
			//Insight::Graphics::Debug::Gizmos::Instance()->DrawGizmos(*m_mainCamera);
#endif

			// If we are in editor then set a blank image as the output.
#ifdef IS_EDITOR
			Editor();
			Insight::Graphics::RenderGraph::Instance()->SetbackBufferSource("editorPass_Output");			
#else
			// We are in a game build .exe. the backbuffer source should a an image which has had processing on it.
			Insight::Graphics::RenderGraph::Instance()->SetbackBufferSource("color");
#endif
			++imageIndex;
		}

		m_imguiRenderer->EndFrame();
		m_imguiRenderer->Render();

		Insight::Graphics::RenderGraph::Instance()->Build();
		//Insight::Graphics::RenderGraph::Instance()->LogToConsole();
		Insight::Graphics::RenderGraph::Instance()->Execute();

		Graphics::GPUDevice::Instance()->EndFrame();
	}

	void GraphicsModule::WaitForIdle()
	{
		Graphics::GPUDevice::Instance()->WaitForGPU();
	}

	GraphicsRendererAPI GraphicsModule::GetAPI()
	{
		return (GraphicsRendererAPI)CONFIG_VAL(GraphicsConfig.GraphicsAPI);
	}

	bool GraphicsModule::IsD311()
	{
		return false;
	}

	bool GraphicsModule::IsD312()
	{
		return false;
	}

	bool GraphicsModule::IsVulkan()
	{
		return GetAPI() == GraphicsRendererAPI::Vulkan;
	}

	bool GraphicsModule::IsOpenGL()
	{
		return false;
	}

	void GraphicsModule::InitLoading()
	{
		Graphics::ImageAttachmentInfo passOutput = { };
		passOutput.Width = Window::GetWidth();
		passOutput.Height = Window::GetHeight();
		passOutput.Name = "module-init-loading";
		passOutput.Format = PixelFormat::R8G8B8A8_UNorm;

		// Add all my passes at runtime.
		auto& pass = Graphics::RenderGraph::Instance()->AddPass("GModuleLoadingPass", Graphics::RenderGraphQueueFlagsBits::RENDER_GRAPH_QUEUE_GRAPHICS_BIT);
		pass.AddColorOutput("GModule_output", passOutput);
		pass.SetClearColour(glm::vec4(0, 0, 0.75f, 1));

		Graphics::RenderGraph::Instance()->SetbackBufferSource("GModule_output");
	}

	void GraphicsModule::ShadowMap()
	{
		using namespace Insight;

		Graphics::ImageAttachmentInfo shadowPassCascadeMap = { };
		shadowPassCascadeMap.Width = 2048 * 2;
		shadowPassCascadeMap.Height = 2048 * 2;
		shadowPassCascadeMap.Name = "shadowPass-CascadeMap";
		shadowPassCascadeMap.Format = PixelFormat::D32_Float;
		//shadowPassCascadeMap.ViewInfo.ImageViewTytpe = Graphics::GPUImageViewType::Type_2D_Array;
		shadowPassCascadeMap.SamplerDesc.AddressModeU = SamplerAddressMode::Clamp_To_Edge;
		shadowPassCascadeMap.SamplerDesc.AddressModeV = SamplerAddressMode::Clamp_To_Edge;
		shadowPassCascadeMap.SamplerDesc.AddressModeW = SamplerAddressMode::Clamp_To_Edge;
		shadowPassCascadeMap.SamplerDesc.MaxLoad = 1.0f;
		shadowPassCascadeMap.AutoSizeToWindow = false;

		auto& shadowPass = Graphics::RenderGraph::Instance()->AddPass("shadowPass", Graphics::RenderGraphQueueFlagsBits::RENDER_GRAPH_QUEUE_GRAPHICS_BIT);
		shadowPass.SetDepthStencilOutput("shaderPass_cacadeMap", shadowPassCascadeMap);
		shadowPass.SetClearDepthStencil(glm::vec2(1.0f, 0.0f));
		shadowPass.SetClearColour(glm::vec4(0, 0, 0, 0));
		shadowPass.SetWindowRect(Maths::Rect(0, 0, shadowPassCascadeMap.Width, shadowPassCascadeMap.Height));
		shadowPass.AddSubpassDependencies(Graphics::SubpassDependency::ShadowPass());
		shadowPass.SetRenderFunc([](Graphics::GPUCommandBuffer* cmdBuffer, Graphics::FrameBufferResources& buffers, Graphics::GPUDescriptorBuilder* builder, Graphics::RenderPass& pass)
		{
			IS_PROFILE_SCOPE("Shadow Map");
			cmdBuffer->SetDepthBias(1.25f, 0.0f, 1.75f);

			DirectionalLightComponent& lightCom = Scene::ActiveScene()->GetAllComponents<DirectionalLightComponent>().at(0);
			DirectionalLightComponentData& data = lightCom.GetComponentData<DirectionalLightComponentData>();
			struct UBO
			{
				glm::mat4 DepthMVP;
			};
			UBO depthMVP;
			//depthMVP.DepthMVP = glm::perspective(glm::radians(data.FOV), 1.0f, data.NearPlane, data.FarPlane) * DirectionToViewMatrix(data.Direction, lightCom.GetEntity().GetComponent<TransformComponent>().GetPostion());
			Graphics::GPUBuffer* uboBuffer = buffers.at(Graphics::GPUBufferFlags::UNIFORM)->Upload(&depthMVP, sizeof(UBO));

			Graphics::GPUShader* defaultShader = nullptr;
			Utils::Hasher hasher;
			hasher.Hash("./data/shaders/vulkan/shadow/shaderPass.vert");
			if (Graphics::GPUShaderCache::Instance()->GetItem(hasher.GetHash(), defaultShader))
			{
				IS_PROFILE_SCOPE("Default shader create");
				defaultShader->SetStage(ShaderStage::Vertex, "./data/shaders/vulkan/shadow/shaderPass.vert", Graphics::ShaderStageInput::FilePath);
				defaultShader->Compile();
			}

			Graphics::GPUPipeline* defaultPipeline = nullptr;
			{
				IS_PROFILE_SCOPE("Default pipeline create and bind");
				hasher.Clear();
				hasher.Hash(defaultShader);
				Graphics::GPUPipelineDesc pipelineDesc = Graphics::GPUPipelineDesc(PrimitiveTopologyType::Triangle_List, PolygonMode::Fill, CullMode::Front, FrontFace::Counter_Clockwise);
				hasher.Hash(pipelineDesc.Hash());
				if (Graphics::GPUPipelineCache::Instance()->GetItem(hasher.GetHash(), defaultPipeline))
				{
					defaultPipeline->SetShader(defaultShader);
					pipelineDesc.DepthBaisEnabled = true;
					defaultPipeline->Init(pass.GetGraphPass(), pipelineDesc);
				}
				cmdBuffer->BindPipeline(PipelineBindPoint::Graphics, defaultPipeline);
			}

			Graphics::GPUDescriptorSet* vertexSet = Graphics::GPUDescriptorSet::New();
			{
				IS_PROFILE_SCOPE("Draw mesh vertices");
				for (auto& mesh : Scene::ActiveScene()->GetAllComponents<MeshComponent>())
				{
					if (mesh.GetMesh() != nullptr)
					{
						glm::mat4 modelMatrix = mesh.GetEntity().GetComponent<TransformComponent>().GetTransform();
						Graphics::GPUBuffer* modelBuffer = buffers.at(Graphics::GPUBufferFlags::UNIFORM)->Upload(&modelMatrix, sizeof(glm::mat4));

						{
							IS_PROFILE_SCOPE("Build per mesh descriptor set");
							builder->BindBuffer(0, uboBuffer, DescriptorType::Unifom_Buffer, ShaderStage::Vertex)
								->BindBuffer(1, modelBuffer, DescriptorType::Unifom_Buffer, ShaderStage::Vertex)->Build(vertexSet);
						}
						Graphics::GPUDescriptorSet* sets[] = { vertexSet };
						cmdBuffer->BindDescriptorSets(PipelineBindPoint::Graphics, defaultPipeline, 0, ARRAY_COUNT(sets), sets, 0, nullptr);

						for (u32 subMeshIndex = 0; subMeshIndex < mesh.GetMesh()->GetSubMeshCount(); ++subMeshIndex)
						{
							Graphics::SubMesh& subMesh = const_cast<Graphics::SubMesh&>(mesh.GetMesh()->GetSubMesh(subMeshIndex));

							u32 offsets[] = { 0 };
							Graphics::GPUBuffer* verticesBuffer[] = { subMesh.GetGPUVertexBuffer() };
							cmdBuffer->BindVertexBuffers(0, 1, verticesBuffer, offsets);
							cmdBuffer->BindIndexBuffer(subMesh.GetGPUIndexBuffer(), 0, Graphics::GPUCommandBufferIndexType::UINT32);
							cmdBuffer->DrawIndexed(subMesh.GetIndexCount(), 1, 0, 0, 0);
						}
					}
				}
			}
			::Delete(vertexSet);
		});
	}
	
	void GraphicsModule::Deffered()
	{
		using namespace Insight;

		Graphics::ImageAttachmentInfo mainPassOutput = { };
		mainPassOutput.Name = "mainPass-Color";
		mainPassOutput.Format = PixelFormat::R8G8B8A8_UNorm;

		// Add all my passes at runtime.
		auto& mainPass = Graphics::RenderGraph::Instance()->AddPass("MainPass", Graphics::RenderGraphQueueFlagsBits::RENDER_GRAPH_QUEUE_GRAPHICS_BIT);
		mainPass.AddColorOutput("color", mainPassOutput);
		mainPass.AddColorOutput("normal", mainPassOutput);
		mainPass.AddColorOutput("position", mainPassOutput);
		mainPass.SetDepthStencilInput("shaderPass_cacadeMap");
		mainPass.SetDepthStencilOutput("mainPassDepthAttachment", Graphics::ImageAttachmentInfo::DepthAttachment());
		mainPass.AddSubpassDependencies(Graphics::SubpassDependency::MainDeferedPass());
		mainPass.SetClearDepthStencil(glm::vec2(1.0f, 0.0f));
		mainPass.SetClearColour(glm::vec4(0, 0, 0, 1));
		mainPass.SetRenderFunc([](Graphics::GPUCommandBuffer* cmdBuffer, Graphics::FrameBufferResources& buffers, Graphics::GPUDescriptorBuilder* builder, Graphics::RenderPass& pass)
		{
			IS_PROFILE_SCOPE("Main Pass");
			struct UBO
			{
				glm::mat4 PVMatrix;
				glm::mat4 LightSpace;
				glm::vec3 LightDir;
			};

			// bind material
			// bind buffers
			// Draw mesh, 

			DirectionalLightComponent& lightCom = Scene::ActiveScene()->GetAllComponents<DirectionalLightComponent>().at(0);
			DirectionalLightComponentData& data = lightCom.GetComponentData<DirectionalLightComponentData>();

			UBO ubo =
			{
				glm::mat4(1.0f),
				glm::mat4(1.0f),
				glm::vec3(5.0f, 5.0f, 5.0f)
			};
			//ubo.LightSpace = glm::perspective(glm::radians(data.FOV), 1.0f, data.NearPlane, data.FarPlane) * DirectionToViewMatrix(data.Direction, lightCom.GetEntity().GetComponent<TransformComponent>().GetPostion());
			ubo.LightDir = -data.Direction;
			//ubo.PVMatrix = m_mainCamera->GetProjMatrix() * glm::inverse(m_mainCamera->GetViewMatrix());
			Graphics::GPUBuffer* uboBuffer = buffers.at(Graphics::GPUBufferFlags::UNIFORM)->Upload(&ubo, sizeof(ubo));

			Graphics::GPUShader* defaultShader = nullptr;
			Utils::Hasher hasher;
			hasher.Hash("./data/shaders/vulkan/default.vert");
			hasher.Hash("./data/shaders/vulkan/default.frag");
			if (Graphics::GPUShaderCache::Instance()->GetItem(hasher.GetHash(), defaultShader))
			{
				IS_PROFILE_SCOPE("Default shader create");
				defaultShader->SetStage(ShaderStage::Vertex, "./data/shaders/vulkan/default.vert", Graphics::ShaderStageInput::FilePath);
				defaultShader->SetStage(ShaderStage::Fragment, "./data/shaders/vulkan/default.frag", Graphics::ShaderStageInput::FilePath);
				defaultShader->Compile();
			}

			hasher.Clear();
			hasher.Hash(defaultShader);
			Graphics::GPUPipelineDesc defualtPipelineDesc = Graphics::GPUPipelineDesc(PrimitiveTopologyType::Triangle_List, PolygonMode::Fill, CullMode::Back, FrontFace::Counter_Clockwise);
			hasher.Hash(defualtPipelineDesc.Hash());
			Graphics::GPUPipeline* defaultPipeline = nullptr;
			{
				IS_PROFILE_SCOPE("Default pipeline create and bind");
				if (Graphics::GPUPipelineCache::Instance()->GetItem(hasher.GetHash(), defaultPipeline))
				{
					defaultPipeline->SetShader(defaultShader);
					defaultPipeline->Init(pass.GetGraphPass(), defualtPipelineDesc);
				}
				cmdBuffer->BindPipeline(PipelineBindPoint::Graphics, defaultPipeline);
			}

			Graphics::GPUDescriptorSet* vertexSet = Graphics::GPUDescriptorSet::New();
			Graphics::GPUDescriptorSet* fragSet = Graphics::GPUDescriptorSet::New();

			{
				IS_PROFILE_SCOPE("Upload mesh vertices");
				for (auto& mesh : Scene::ActiveScene()->GetAllComponents<MeshComponent>())
				{
					if (mesh.GetMesh() != nullptr)
					{
						glm::mat4 modelMatrix = mesh.GetEntity().GetComponent<TransformComponent>().GetTransform();
						Graphics::GPUBuffer* modelBuffer = buffers.at(Graphics::GPUBufferFlags::UNIFORM)->Upload(&modelMatrix, sizeof(glm::mat4));

						Graphics::GPUImage* shadowPassTexture = (Graphics::GPUImage*)pass.GetPhysicalImage(pass.GetDepthStencilInput().GetPhysicalIndex());
						const Graphics::GPUImageView* shadowPassTextureView = pass.GetPhysicalImageView(pass.GetDepthStencilInput().GetPhysicalIndex());

						{
							IS_PROFILE_SCOPE("Build per mesh descriptor set");
							builder->BindBuffer(0, uboBuffer, DescriptorType::Unifom_Buffer, ShaderStage::Vertex)
								->BindBuffer(1, modelBuffer, DescriptorType::Unifom_Buffer, ShaderStage::Vertex)
								->BindImage(2, shadowPassTexture, DescriptorType::Combined_Image_Sampler, ShaderStage::Fragment)->Build(vertexSet);
						}
						Graphics::GPUDescriptorSet* sets[] = { vertexSet };
						cmdBuffer->BindDescriptorSets(PipelineBindPoint::Graphics, defaultPipeline, 0, ARRAY_COUNT(sets), sets, 0, nullptr);

						for (u32 subMeshIndex = 0; subMeshIndex < mesh.GetMesh()->GetSubMeshCount(); ++subMeshIndex)
						{
							int textureDiffuse = 1;
							Graphics::GPUBuffer* textureDiffuseBuffer = buffers.at(Graphics::GPUBufferFlags::UNIFORM)->Upload(&textureDiffuse, sizeof(int));

							Graphics::SubMesh& subMesh = const_cast<Graphics::SubMesh&>(mesh.GetMesh()->GetSubMesh(subMeshIndex));
							Graphics::GPUImage* diffuseTexture = nullptr;
							std::string diffuseTextureString = subMesh.GetTexture("texture_diffuse");
							if (diffuseTextureString.empty())
							{
								diffuseTextureString = "./data/embed2.jpg";
							}
							Utils::Hasher diffuseTextureHasher;
							diffuseTextureHasher.Hash(diffuseTextureString);
							if (Graphics::GPUImageCache::Instance()->GetItem(diffuseTextureHasher.GetHash(), diffuseTexture))
							{
								diffuseTexture->Init(Graphics::GPUImageDesc::Texture(1, SampleLevel::None, PixelFormat::R8G8B8A8_UNorm, diffuseTextureString));
							}

							Graphics::GPUImageView* diffuseTextureView = Graphics::GPUImageView::New();
							pass.AddLifeTimeObject(diffuseTextureView);
							diffuseTextureView->Init(diffuseTexture);

							{
								IS_PROFILE_SCOPE("Build per submesh descriptor set");
								builder->BindImage(0, diffuseTexture, DescriptorType::Combined_Image_Sampler, ShaderStage::Fragment)
									->BindBuffer(1, textureDiffuseBuffer, DescriptorType::Unifom_Buffer, ShaderStage::Fragment)->Build(fragSet);
							}
							Graphics::GPUDescriptorSet* fragSets[] = { fragSet };
							cmdBuffer->BindDescriptorSets(PipelineBindPoint::Graphics, defaultPipeline, 1, ARRAY_COUNT(fragSets), fragSets, 0, nullptr);

							u32 offsets[] = { 0 };
							Graphics::GPUBuffer* verticesBuffer[] = { subMesh.GetGPUVertexBuffer() };
							cmdBuffer->BindVertexBuffers(0, 1, verticesBuffer, offsets);
							cmdBuffer->BindIndexBuffer(subMesh.GetGPUIndexBuffer(), 0, Graphics::GPUCommandBufferIndexType::UINT32);

							cmdBuffer->DrawIndexed(subMesh.GetIndexCount(), 1, 0, 0, 0);
						}
					}
				}
			}
			::Delete(vertexSet);
			::Delete(fragSet);
		});
	}

#ifdef IS_EDITOR
	void GraphicsModule::Editor()
	{
		using namespace Insight;

		Graphics::ImageAttachmentInfo passOutput = { };
		passOutput.Width = Window::GetWidth();
		passOutput.Height = Window::GetHeight();
		passOutput.Name = "editorPass_Output-Color";
		passOutput.Format = PixelFormat::R8G8B8A8_UNorm;

		// Add all my passes at runtime.
		auto& pass = Graphics::RenderGraph::Instance()->AddPass("EditorPass", Graphics::RenderGraphQueueFlagsBits::RENDER_GRAPH_QUEUE_GRAPHICS_BIT);
		pass.AddColorOutput("editorPass_Output", passOutput);
		pass.AddColorInput("color");
		pass.AddColorInput("normal");
		pass.AddColorInput("position");
		pass.SetClearColour(glm::vec4(0.15f, 0.15f, 0.15f, 1));
	}
#endif
}