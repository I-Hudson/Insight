#include "Renderpass.h"
#include "Runtime/Engine.h"

#include "Graphics/RenderContext.h"
#include "Graphics/GraphicsManager.h"
#include "Graphics/RenderTarget.h"
#include "Graphics/RenderGraph/RenderGraph.h"
#include "Graphics/Frustum.h"
#include "Graphics/Window.h"

#include "Core/Profiler.h"

#include "Input/InputManager.h"

#include "Scene/SceneManager.h"
#include "ECS/Components/TransformComponent.h"
#include "ECS/Components/MeshComponent.h"

#include "Resource/Model.h"
#include "Resource/Mesh.h"
#include "Resource/Texture2D.h"
#include "Resource/Material.h"

#include <glm/gtx/matrix_interpolation.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Insight
{
	const float ShadowZNear = 0.1f;
	const float ShadowZFar = 512.0f;
	const float ShadowFOV = 65.0f;
	const u32 Shadow_Depth_Tex_Size = 1024 * 4;


	constexpr bool Reverse_Z_For_Depth = false;
	constexpr bool Depth_Prepass = false;

	const float Main_Camera_Near_Plane = 0.1f;
	const float Main_Camera_Far_Plane = 1024.0f;

	float cascade_split_lambda = 0.85f;

	glm::vec3 dir_light_direction = glm::vec3(0.5f, -0.7f, 0.5f);

	std::vector<Ptr<ECS::Entity>> entities_to_render;

#define ECS_RENDER

	namespace Graphics
	{
		float aspect = 0.0f;
		void Renderpass::Create()
		{
			//Runtime::Model* model_backpack = static_cast<Runtime::Model*>(Runtime::ResourceManager::Instance().Load("./Resources/models/Survival_BackPack_2/backpack.obj", Runtime::Model::GetStaticResourceTypeId()));
			//Runtime::Model* model_sponza_obj = static_cast<Runtime::Model*>(Runtime::ResourceManager::Instance().Load("./Resources/models/sponza_old/sponza.obj", Runtime::Model::GetStaticResourceTypeId()));
			Runtime::Model* model_sponza = static_cast<Runtime::Model*>(Runtime::ResourceManager::Instance().Load("./Resources/models/Main.1_Sponza/NewSponza_Main_glTF_002.gltf", Runtime::Model::GetStaticResourceTypeId()));
			Runtime::Model* model_sponza_curtains = static_cast<Runtime::Model*>(Runtime::ResourceManager::Instance().Load("./Resources/models/PKG_A_Curtains/NewSponza_Curtains_glTF.gltf", Runtime::Model::GetStaticResourceTypeId()));
			//Runtime::Model* model_vulklan_scene = static_cast<Runtime::Model*>(Runtime::ResourceManager::Instance().Load("./Resources/models/vulkanscene_shadow_20.gltf", Runtime::Model::GetStaticResourceTypeId()));
			//Runtime::Model* model = static_cast<Runtime::Model*>(Runtime::ResourceManager::Instance().Load("./Resources/models/Survival_BackPack_2/backpack.obj", Runtime::Model::GetStaticResourceTypeId()));

			//while (model_sponza->GetResourceState() != Runtime::EResoruceStates::Loaded
			//	&& model_sponza_curtains->GetResourceState() != Runtime::EResoruceStates::Loaded)
			{
			}
			Runtime::ResourceManager::Instance().Print();
			model_sponza->CreateEntityHierarchy();
			model_sponza_curtains->CreateEntityHierarchy();
			//model_backpack->CreateEntityHierarchy();
			//model_vulklan_scene->CreateEntityHierarchy();
			//model->CreateEntityHierarchy();
			//model->CreateEntityHierarchy();

			m_buffer_frame = {};
			aspect = (float)Window::Instance().GetWidth() / (float)Window::Instance().GetHeight();
			m_buffer_frame.Projection = glm::perspective(glm::radians(90.0f), aspect, 0.1f, 1024.0f);
			m_buffer_frame.View = glm::mat4(1.0f);

			RenderContext* render_context = GraphicsManager::Instance().GetRenderContext();
			RHI_SamplerManager& sampler_manager = render_context->GetSamplerManager();

			RHI_SamplerCreateInfo sampler_create_info = { };
			sampler_create_info.MagFilter = Filter::Linear;
			sampler_create_info.MinFilter = Filter::Linear;
			sampler_create_info.MipmapMode = SamplerMipmapMode::Nearest;
			sampler_create_info.AddressMode = SamplerAddressMode::ClampToEdge;
			sampler_create_info.CompareEnabled = true;
			if (Reverse_Z_For_Depth)
			{
				sampler_create_info.CompareOp = CompareOp::Greater;
			}
			else
			{
				sampler_create_info.CompareOp = CompareOp::Less;
			}
			m_buffer_samplers.Shadow_Sampler = sampler_manager.GetOrCreateSampler(sampler_create_info);
			sampler_create_info.AddressMode = SamplerAddressMode::Repeat;
			m_buffer_samplers.Repeat_Sampler = sampler_manager.GetOrCreateSampler(sampler_create_info);
			sampler_create_info.AddressMode = SamplerAddressMode::ClampToEdge;
			m_buffer_samplers.Clamp_Sampler = sampler_manager.GetOrCreateSampler(sampler_create_info);
			sampler_create_info.AddressMode = SamplerAddressMode::MirroredRepeat;
			m_buffer_samplers.MirroredRepeat_Sampler = sampler_manager.GetOrCreateSampler(sampler_create_info);

			m_imgui_pass.Create();
		}

		bool useShadowCamera = false;

		void Renderpass::Render()
		{
			IS_PROFILE_FUNCTION();

			IMGUI_VALID(ImGui::Checkbox("Move shadow camera", &useShadowCamera));

			if (Input::InputManager::IsKeyPressed(IS_KEY_ENTER))
			{
				Runtime::ResourceManager::Instance().Unload(Runtime::ResourceManager::Instance().Load("./Resources/models/sponza_old/sponza.obj", Runtime::Model::GetStaticResourceTypeId()));
				auto entities = App::SceneManager::Instance().GetActiveScene().Lock()->GetAllEntitiesWithComponentByName(ECS::MeshComponent::Type_Name);
				for (auto entity : entities)
				{
					static_cast<ECS::MeshComponent*>(entity->GetComponentByName(ECS::MeshComponent::Type_Name))->SetMesh(nullptr);
				}
			}

			WPtr<App::Scene> w_scene = App::SceneManager::Instance().GetActiveScene();
			if (RPtr<App::Scene> scene = w_scene.Lock())
			{
				entities_to_render = scene->GetAllEntitiesWithComponentByName(ECS::MeshComponent::Type_Name);
			}

			RenderGraph::Instance().SetRenderResolution({ Window::Instance().GetWidth(), Window::Instance().GetHeight() });
			RenderGraph::Instance().SetOutputResolution({ Window::Instance().GetWidth(), Window::Instance().GetHeight() });

			UpdateCamera(m_buffer_frame);
			m_buffer_frame.Render_Resolution = RenderGraph::Instance().GetRenderResolution();
			m_buffer_frame.Ouput_Resolution = RenderGraph::Instance().GetOutputResolution();

			ShadowPass();
			//ShadowCullingPass();
			if (Depth_Prepass)
			{
				DepthPrepass();
			}
			Sample();
			Composite();
			Swapchain();
			ImGuiPass();
		}

		void Renderpass::Destroy()
		{
			GraphicsManager::Instance().GetRenderContext()->GpuWaitForIdle();
			m_imgui_pass.Release();
		}

		glm::vec2 swapchainColour = { 0,0 };
		glm::vec2 swapchainColour2 = { 0,0 };

		glm::mat4 MakeInfReversedZProjRH(float fovY_radians, float aspectWbyH, float zNear)
		{
			float f = 1.0f / tan(fovY_radians / 2.0f);
			return glm::mat4(
				f / aspectWbyH, 0.0f, 0.0f, 0.0f,
				0.0f, f, 0.0f, 0.0f,
				0.0f, 0.0f, 0.0f, -1.0f,
				0.0f, 0.0f, zNear, 0.0f);
		}

		BufferLight GetReverseZDepthCamera(BufferLight camera, bool inverse)
		{
			BufferLight shadowCamera = camera;

			/*float h = 1.0 / glm::tan(glm::radians(45.0f) * 0.5f);
			float aspect = (float)m_depthTarget->GetDesc().Width / (float)m_depthTarget->GetDesc().Height;
			float w = h / aspect;
			float a = -ShadowZNear / (ShadowZFar - ShadowZNear);
			float b = (ShadowZNear * ShadowZFar) / (ShadowZFar - ShadowZNear);

			shadowCamera.Projection = glm::mat4(
				w, 0.0f, 0.0f, 0.0f,
				0.0f, -h, 0.0f, 0.0f,
				0.0f, 0.0f, a, 1.0f,
				0.0f, 0.0f, b, 0.0f);*/

			glm::mat4 reverseZProj = glm::mat4(1);
			reverseZProj[2][2] = -1;
			reverseZProj[2][3] = 1;
			//shadowCamera.Projection = MakeInfReversedZProjRH(glm::radians(ShadowFOV), 1.0f, ShadowZNear);
			//shadowCamera.ProjView = shadowCamera.Projection * (inverse ? glm::inverse(shadowCamera.View) : shadowCamera.View);

			return shadowCamera;
		}

		void ImGuiMat4(glm::mat4& matrix)
		{
			float x_row[4] = { matrix[0].x, matrix[0].y, matrix[0].z, matrix[0].w };
			float y_row[4] = { matrix[1].x, matrix[1].y, matrix[1].z, matrix[1].w };
			float z_row[4] = { matrix[2].x, matrix[2].y, matrix[2].z, matrix[2].w };
			float w_row[4] = { matrix[3].x, matrix[3].y, matrix[3].z, matrix[3].w };
			IMGUI_VALID(ImGui::DragFloat4("Camera X Row", x_row));
			IMGUI_VALID(ImGui::DragFloat4("Camera Y Row", y_row));
			IMGUI_VALID(ImGui::DragFloat4("Camera Z Row", z_row));
			IMGUI_VALID(ImGui::DragFloat4("Camera W Row", w_row));
			matrix[0] = glm::vec4(x_row[0], x_row[1], x_row[2], x_row[3]);
			matrix[1] = glm::vec4(y_row[0], y_row[1], y_row[2], y_row[3]);
			matrix[2] = glm::vec4(z_row[0], z_row[1], z_row[2], z_row[3]);
			matrix[3] = glm::vec4(w_row[0], w_row[1], w_row[2], w_row[3]);
		}

		void Renderpass::ShadowPass()
		{
			struct PassData
			{
				BufferLight Cascade_Cameras;
				RGTextureHandle Depth_Tex;
				std::vector<Ptr<ECS::Entity>> Entities;
			};
			PassData data;
			data.Entities = entities_to_render;
			data.Cascade_Cameras = BufferLight::GetCascades(m_buffer_frame, 4, cascade_split_lambda);

			IMGUI_VALID(ImGui::Begin("Directional Light Direction"));
			float dir[3] = { dir_light_direction.x, dir_light_direction.y, dir_light_direction.z };
			IMGUI_VALID(if (ImGui::DragFloat3("Direction", dir, 0.001f, -1.0f, 1.0f))
			{
				dir_light_direction = glm::vec3(dir[0], dir[1], dir[2]);
			});

			static float depth_constant_factor = 4.0f;
			static float depth_slope_factor = 1.5f;
			IMGUI_VALID(ImGui::DragFloat("Dpeth bias constant factor", &depth_constant_factor, 0.01f));
			IMGUI_VALID(ImGui::DragFloat("Dpeth bias slope factor", &depth_slope_factor, 0.01f));
			IMGUI_VALID(ImGui::DragFloat("Cascade Split Lambda", &cascade_split_lambda, 0.001f, 0.0f, 1.0f));

			IMGUI_VALID(ImGui::End());

			/// Look into "panking" for dir light https:///www.gamedev.net/forums/topic/639036-shadow-mapping-and-high-up-objects/
			RenderGraph::Instance().AddPass<PassData>(L"Cascade shadow pass",
				[](PassData& data, RenderGraphBuilder& builder)
				{
					IS_PROFILE_SCOPE("Cascade shadow pass setup");

					RHI_TextureCreateInfo tex_create_info = RHI_TextureCreateInfo::Tex2DArray(Shadow_Depth_Tex_Size, Shadow_Depth_Tex_Size,
						PixelFormat::D32_Float, ImageUsageFlagsBits::DepthStencilAttachment | ImageUsageFlagsBits::Sampled, 4);

					RGTextureHandle depth_tex = builder.CreateTexture(L"Cascade_Shadow_Tex", tex_create_info);
					builder.WriteDepthStencil(depth_tex);
					data.Depth_Tex = depth_tex;

					builder.SetViewport(Shadow_Depth_Tex_Size, Shadow_Depth_Tex_Size);
					builder.SetScissor(Shadow_Depth_Tex_Size, Shadow_Depth_Tex_Size);

					ShaderDesc shader_description = { };
					shader_description.VertexFilePath = L"./Resources/Shaders/hlsl/Cascade_Shadow.hlsl";
					builder.SetShader(shader_description);

					PipelineStateObject pso = { };
					pso.Name = L"Cascade_Shadow_PSO";
					pso.ShaderDescription = shader_description;
					//pso.CullMode = CullMode::Front;
					pso.FrontFace = FrontFace::CounterClockwise;
					pso.DepthClampEnabled = false;
					pso.DepthBaisEnabled = true;
					if (Reverse_Z_For_Depth)
					{
						pso.DepthSteniclClearValue = glm::vec2(0.0f, 0.0f);
						pso.DepthCompareOp = CompareOp::GreaterOrEqual;
					}
					else
					{
						pso.DepthCompareOp = CompareOp::LessOrEqual;
					}
					pso.Dynamic_States = { DynamicState::Viewport, DynamicState::Scissor, DynamicState::DepthBias };
					builder.SetPipeline(pso);
				},
				[](PassData& data, RenderGraph& render_graph, RHI_CommandList* cmdList)
				{
					IS_PROFILE_SCOPE("Cascade shadow pass execute");

					PipelineStateObject pso = render_graph.GetPipelineStateObject(L"Cascade shadow pass");
					cmdList->BindPipeline(pso, nullptr);

					if (Reverse_Z_For_Depth)
					{
						cmdList->SetDepthBias(-depth_constant_factor, 0.0f, -depth_slope_factor);
					}
					else
					{
						cmdList->SetDepthBias(depth_constant_factor, 0.0f, depth_slope_factor);
					}

					cmdList->SetUniform(1, 0, data.Cascade_Cameras);

					RHI_Texture* depth_tex = render_graph.GetRHITexture(data.Depth_Tex);
					for (u32 i = 0; i < depth_tex->GetInfo().Layer_Count; ++i)
					{
						RenderpassDescription renderpass_description = render_graph.GetRenderpassDescription(L"Cascade shadow pass");
						renderpass_description.DepthStencilAttachment.Layer_Array_Index = static_cast<u32>(i);
						cmdList->BeginRenderpass(renderpass_description);

						cmdList->SetPushConstant(0, sizeof(int), &i);

						for (const Ptr<ECS::Entity> e : data.Entities)
						{
							ECS::TransformComponent* transform_component = static_cast<ECS::TransformComponent*>(e->GetComponentByName(ECS::TransformComponent::Type_Name));
							glm::mat4 transform = transform_component->GetTransform();

							BufferPerObject object = {};
							object.Transform = transform;
							object.Previous_Transform = transform;
							cmdList->SetUniform(1, 1, object);

							ECS::MeshComponent* mesh_component = static_cast<ECS::MeshComponent*>(e->GetComponentByName(ECS::MeshComponent::Type_Name));
							if (!mesh_component
								|| !mesh_component->GetMesh())
							{
								continue;
							}
							mesh_component->GetMesh()->Draw(cmdList);
						}
						cmdList->EndRenderpass();
					}
				}, std::move(data));
		}

		void Renderpass::ShadowCullingPass()
		{
			struct TData
			{ };
			RenderGraph::Instance().AddPass<TData>(L"Shadow_Culling", [](TData& data, RenderGraphBuilder& builder)
				{

				},
				[](TData& data, RenderGraph& rg, RHI_CommandList* cmd_list)
				{

				});
		}

		void Renderpass::DepthPrepass()
		{
			IS_PROFILE_FUNCTION();

			struct PassData
			{
				BufferFrame Buffer_Frame = { };
			};
			PassData pass_data = {};
			pass_data.Buffer_Frame = m_buffer_frame;

			RenderGraph::Instance().AddPass<PassData>(L"Depth_Prepass", 
				[](PassData& data, RenderGraphBuilder& builder)
				{
					IS_PROFILE_SCOPE("Depth_Prepass pass setup");

					RHI_TextureCreateInfo textureCreateInfo = RHI_TextureCreateInfo::Tex2D(
						  builder.GetRenderResolution().x
						, builder.GetRenderResolution().y
						, PixelFormat::D32_Float
						, ImageUsageFlagsBits::DepthStencilAttachment | ImageUsageFlagsBits::Sampled);
					RGTextureHandle depthStencil = builder.CreateTexture(L"Depth_Prepass_DepthStencil", textureCreateInfo);
					builder.WriteDepthStencil(depthStencil);

					ShaderDesc shaderDesc;
					{
						IS_PROFILE_SCOPE("Depth_Prepass_GetShader");
						shaderDesc.VertexFilePath = L"Resources/Shaders/hlsl/Depth_Prepass.hlsl";
					}
					builder.SetShader(shaderDesc);

					PipelineStateObject depth_Prepass_pso = { };
					{
						IS_PROFILE_SCOPE("Depth_Prepass_SetPipelineStateObject");
						depth_Prepass_pso.Name = L"Depth_Prepass_PSO";
						depth_Prepass_pso.CullMode = CullMode::Back;
						depth_Prepass_pso.FrontFace = FrontFace::CounterClockwise;
						depth_Prepass_pso.ShaderDescription = shaderDesc;

						depth_Prepass_pso.DepthTest = true;
						depth_Prepass_pso.DepthWrite = true;
						depth_Prepass_pso.DepthCompareOp = CompareOp::LessOrEqual;
					}
					builder.SetPipeline(depth_Prepass_pso);

					builder.SetViewport(Window::Instance().GetWidth(), Window::Instance().GetHeight());
					builder.SetScissor(Window::Instance().GetWidth(), Window::Instance().GetHeight());
				},
				[](PassData& data, RenderGraph& render_graph, RHI_CommandList* cmdList)
				{
					IS_PROFILE_SCOPE("Depth_Prepass pass execute");

					PipelineStateObject pso = render_graph.GetPipelineStateObject(L"Depth_Prepass");
					cmdList->BindPipeline(pso, nullptr);
					cmdList->BeginRenderpass(render_graph.GetRenderpassDescription(L"Depth_Prepass"));

					{
						IS_PROFILE_SCOPE("Depth_Prepass-SetUniform");
						cmdList->SetUniform(0, 0, &data.Buffer_Frame, sizeof(data.Buffer_Frame));
					}

					Frustum camera_frustum(data.Buffer_Frame.View, data.Buffer_Frame.Projection, 1000.0f);

					for (const Ptr<ECS::Entity> e : entities_to_render)
					{
						ECS::MeshComponent* mesh_component = static_cast<ECS::MeshComponent*>(e->GetComponentByName(ECS::MeshComponent::Type_Name));
						if (!mesh_component
							|| !mesh_component->GetMesh())
						{
							continue;
						}

						ECS::TransformComponent* transform_component = static_cast<ECS::TransformComponent*>(e->GetComponentByName(ECS::TransformComponent::Type_Name));
						glm::mat4 transform = transform_component->GetTransform();

						BufferPerObject object = {};
						object.Transform = transform;
						object.Previous_Transform = transform;
						cmdList->SetUniform(1, 1, object);

						mesh_component->GetMesh()->Draw(cmdList);
					}

					cmdList->EndRenderpass();
				}, std::move(pass_data));
		}

		void Renderpass::Sample()
		{
			IS_PROFILE_FUNCTION();

			struct TestPassData
			{
				BufferFrame Buffer_Frame = { };
				BufferSamplers Buffer_Samplers = { };
				std::vector<Ptr<ECS::Entity>> Entities;
				int Mesh_Lod = 0;
			};
			TestPassData Pass_Data = {};
			Pass_Data.Buffer_Frame = m_buffer_frame;
			Pass_Data.Buffer_Samplers = m_buffer_samplers;
			Pass_Data.Entities = entities_to_render;

			BufferLight shader_cameras = BufferLight::GetCascades(m_buffer_frame, 4, cascade_split_lambda);

			static int camera_index = 0;
			static int mesh_lod_index = 0;
			const char* cameras[] = { "Default", "Shadow0", "Shadow1", "Shadow2", "Shadow3" };

			std::array<std::string, Runtime::Mesh::s_LOD_Count> mesh_lods;
			std::array<const char*, Runtime::Mesh::s_LOD_Count> mesh_lods_const_char;
			for (size_t i = 0; i < Runtime::Mesh::s_LOD_Count; ++i)
			{
				mesh_lods[i] = "LOD_" + std::to_string(i);
				mesh_lods_const_char[i] = mesh_lods[i].c_str();
			}

			ImGui::Begin("GBuffer");
			ImGui::ListBox("Availible cameras", &camera_index, cameras, ARRAYSIZE(cameras));
			ImGui::ListBox("Mesh LODs", &mesh_lod_index, mesh_lods_const_char.data(), mesh_lods_const_char.size());
			ImGui::End();

			if (camera_index > 0)
			{
				Pass_Data.Buffer_Frame.Proj_View = shader_cameras.ProjView[camera_index - 1];
			}
			Pass_Data.Mesh_Lod = mesh_lod_index;

			RenderGraph::Instance().AddPass<TestPassData>(L"GBuffer", 
				[](TestPassData& data, RenderGraphBuilder& builder)
				{
					IS_PROFILE_SCOPE("GBuffer pass setup");

					RHI_TextureCreateInfo textureCreateInfo = RHI_TextureCreateInfo::Tex2D(
						  builder.GetRenderResolution().x
						, builder.GetRenderResolution().y
						, PixelFormat::R8G8B8A8_UNorm
						, ImageUsageFlagsBits::ColourAttachment | ImageUsageFlagsBits::Sampled);
					RGTextureHandle colourRT = builder.CreateTexture(L"ColourRT", textureCreateInfo);
					builder.WriteTexture(colourRT);

					textureCreateInfo = RHI_TextureCreateInfo::Tex2D(
						  builder.GetRenderResolution().x
						, builder.GetRenderResolution().y
						, PixelFormat::R16G16B16A16_Float
						, ImageUsageFlagsBits::ColourAttachment | ImageUsageFlagsBits::Sampled);
					RGTextureHandle normal_rt = builder.CreateTexture(L"NormalRT", textureCreateInfo);
					builder.WriteTexture(normal_rt);

					textureCreateInfo = RHI_TextureCreateInfo::Tex2D(
						  builder.GetRenderResolution().x
						, builder.GetRenderResolution().y
						, PixelFormat::R16G16_Float
						, ImageUsageFlagsBits::ColourAttachment | ImageUsageFlagsBits::Sampled);
					RGTextureHandle velocity_rt = builder.CreateTexture(L"VelocityRT", textureCreateInfo);
					builder.WriteTexture(velocity_rt);

					textureCreateInfo.Format = PixelFormat::D32_Float;
					textureCreateInfo.ImageUsage = ImageUsageFlagsBits::DepthStencilAttachment | ImageUsageFlagsBits::Sampled;
					if (Depth_Prepass)
					{
						RGTextureHandle depth_prepass = builder.GetTexture(L"Depth_Prepass_DepthStencil");
						builder.WriteDepthStencil(depth_prepass);
					}
					else
					{
						RGTextureHandle depthStencil = builder.CreateTexture(L"GBuffer_DepthStencil", textureCreateInfo);
						builder.WriteDepthStencil(depthStencil);
					}
					builder.ReadTexture(builder.GetTexture(L"Cascade_Shadow_Tex"));

					ShaderDesc shaderDesc;
					{
						IS_PROFILE_SCOPE("GBuffer-GetShader");
						shaderDesc.VertexFilePath = L"Resources/Shaders/hlsl/GBuffer.hlsl";
						shaderDesc.PixelFilePath = L"Resources/Shaders/hlsl/GBuffer.hlsl";
					}
					builder.SetShader(shaderDesc);

					PipelineStateObject gbufferPso = { };
					{
						IS_PROFILE_SCOPE("GBuffer-SetPipelineStateObject");
						gbufferPso.Name = L"GBuffer_PSO";
						gbufferPso.CullMode = CullMode::Back;
						gbufferPso.FrontFace = FrontFace::Clockwise;
						gbufferPso.ShaderDescription = shaderDesc;
						gbufferPso.DepthCompareOp = CompareOp::LessOrEqual;

						if (Depth_Prepass)
						{
							gbufferPso.DepthWrite = false;
						}
					}
					builder.SetPipeline(gbufferPso);

					if (Depth_Prepass)
					{
						RenderpassDescription renderpass_description = { };
						renderpass_description.DepthStencilAttachment = AttachmentDescription::Load(PixelFormat::D32_Float, Graphics::ImageLayout::DepthAttachmentStencilReadOnly);
						renderpass_description.DepthStencilAttachment.InitalLayout = ImageLayout::DepthAttachmentStencilReadOnly;
						builder.SetRenderpass(renderpass_description);
					}

					builder.SetViewport(Window::Instance().GetWidth(), Window::Instance().GetHeight());
					builder.SetScissor(Window::Instance().GetWidth(), Window::Instance().GetHeight());
				},
				[this](TestPassData& data, RenderGraph& render_graph, RHI_CommandList* cmdList)
				{
					IS_PROFILE_SCOPE("GBuffer pass execute");

					PipelineStateObject pso = render_graph.GetPipelineStateObject(L"GBuffer");
					cmdList->BindPipeline(pso, nullptr);
					cmdList->BeginRenderpass(render_graph.GetRenderpassDescription(L"GBuffer"));

					{
						IS_PROFILE_SCOPE("Set Buffer Frame Uniform");
						BindCommonResources(cmdList, data.Buffer_Frame, data.Buffer_Samplers);
					}

					Frustum camera_frustum(data.Buffer_Frame.View, data.Buffer_Frame.Projection, 1000.0f);

					for (const Ptr<ECS::Entity> e : data.Entities)
					{
						ECS::MeshComponent* mesh_component = static_cast<ECS::MeshComponent*>(e->GetComponentByName(ECS::MeshComponent::Type_Name));
						if (!mesh_component
							|| !mesh_component->GetMesh())
						{
							continue;
						}

						Runtime::Material* material = mesh_component->GetMaterial();
						if (!material)
						{
							//continue;
						}

						ECS::TransformComponent* transform_component = static_cast<ECS::TransformComponent*>(e->GetComponentByName(ECS::TransformComponent::Type_Name));
						glm::mat4 transform = transform_component->GetTransform();

						BufferPerObject object = {};
						object.Transform = transform;
						object.Previous_Transform = transform;

						// Theses sets and bindings shouldn't chagne.
						const Runtime::ResourceReferenceLink* diffuse_link = mesh_component->GetMesh()->GetReferenceLink(0);
						if (diffuse_link)
						{
							Runtime::Texture2D* diffuse_texture = static_cast<Runtime::Texture2D*>(diffuse_link->GetLinkResource());
							cmdList->SetTexture(1, 2, diffuse_texture->GetRHITexture());
							object.Textures_Set |= 1 << 0;
						}
						//cmdList->SetTexture(1, 3, material->GetTexture(Runtime::TextureTypes::Normal)->GetRHITexture());
						//cmdList->SetTexture(1, 4, material->GetTexture(Runtime::TextureTypes::Specular)->GetRHITexture());

						cmdList->SetUniform(1, 1, object);

						mesh_component->GetMesh()->Draw(cmdList, data.Mesh_Lod);
					}

					cmdList->EndRenderpass();
				}, std::move(Pass_Data));
		}

		void Renderpass::Composite()
		{
			struct PassData
			{
				BufferFrame Buffer_Frame;
				BufferSamplers Buffer_Samplers;
				BufferLight Cascade_Cameras;
			};
			PassData pass_data = {};
			pass_data.Buffer_Frame = m_buffer_frame;
			pass_data.Buffer_Samplers = m_buffer_samplers;
			pass_data.Cascade_Cameras = BufferLight::GetCascades(m_buffer_frame, 4, cascade_split_lambda);

			static int output_texture;
			static int cascade_override;
			const char* items[] = { "Colour", "World Normal", "World Position", "Shadow", "Colour + Shadow", "View Position", "Cascade splits", "Shadow NDC Z" };
			const char* cascade_override_items[] = { "0", "1", "2", "3" };

			IMGUI_VALID(ImGui::Begin("Composite pass"));
			IMGUI_VALID(ImGui::ListBox("Display shadow", &output_texture, items, ARRAY_COUNT(items)));
			IMGUI_VALID(ImGui::ListBox("Cascde Index shadow", &cascade_override, cascade_override_items, ARRAY_COUNT(cascade_override_items)));
			IMGUI_VALID(ImGui::End());

			ImGui::Begin("Directional light colour");
			float light_colour[3] =
			{
				m_directional_light.Light_Colour.x,
				m_directional_light.Light_Colour.y,
				m_directional_light.Light_Colour.z
			};
			if (ImGui::ColorPicker3("Light Colour", light_colour))
			{
				m_directional_light.Light_Colour.x = light_colour[0];
				m_directional_light.Light_Colour.y = light_colour[1];
				m_directional_light.Light_Colour.z = light_colour[2];
			}
			ImGui::End();

			pass_data.Cascade_Cameras.Light_Colour = m_directional_light.Light_Colour;

			RenderGraph::Instance().AddPass<PassData>(L"Composite_Pass",
				[](PassData& data, RenderGraphBuilder& builder)
				{
					builder.ReadTexture(builder.GetTexture(L"ColourRT"));
					builder.ReadTexture(builder.GetTexture(L"NormalRT"));
					builder.ReadTexture(builder.GetTexture(L"NormalRT"));
					if (Depth_Prepass)
					{
						builder.ReadTexture(builder.GetTexture(L"Depth_Prepass_DepthStencil"));
					}
					else
					{
						builder.ReadTexture(builder.GetTexture(L"GBuffer_DepthStencil"));
					}
					builder.ReadTexture(builder.GetTexture(L"Cascade_Shadow_Tex"));

					RHI_TextureCreateInfo create_info = RHI_TextureCreateInfo::Tex2D(
						  builder.GetRenderResolution().x
						, builder.GetRenderResolution().y
						, PixelFormat::R32G32B32A32_Float
						, ImageUsageFlagsBits::ColourAttachment | ImageUsageFlagsBits::Sampled);
					RGTextureHandle composite_handle = builder.CreateTexture(L"Composite_Tex", create_info);
					builder.WriteTexture(composite_handle);

					ShaderDesc shader_description = { };
					shader_description.VertexFilePath = L"./Resources/Shaders/hlsl/Composite.hlsl";
					shader_description.PixelFilePath = L"./Resources/Shaders/hlsl/Composite.hlsl";
					builder.SetShader(shader_description);

					PipelineStateObject pso = { };
					pso.Name = L"Composite_PSO";
					pso.ShaderDescription = shader_description;
					pso.DepthTest = false;
					pso.PolygonMode = PolygonMode::Fill;
					pso.CullMode = CullMode::Front;
					pso.FrontFace = FrontFace::CounterClockwise;
					builder.SetPipeline(pso);

					builder.SetViewport(Window::Instance().GetWidth(), Window::Instance().GetHeight());
					builder.SetScissor(Window::Instance().GetWidth(), Window::Instance().GetHeight());
				},
				[this](PassData& data, RenderGraph& render_graph, RHI_CommandList* cmd_list)
				{
					PipelineStateObject pso = render_graph.GetPipelineStateObject(L"Composite_Pass");
					cmd_list->BindPipeline(pso, nullptr);
					cmd_list->BeginRenderpass(render_graph.GetRenderpassDescription(L"Composite_Pass"));


					BindCommonResources(cmd_list, data.Buffer_Frame, data.Buffer_Samplers);
					cmd_list->SetUniform(1, 0, data.Cascade_Cameras);

					const u8 texture_offset = 5;
					cmd_list->SetTexture(0, texture_offset, render_graph.GetRHITexture(render_graph.GetTexture(L"ColourRT")));
					cmd_list->SetTexture(0, texture_offset + 1, render_graph.GetRHITexture(render_graph.GetTexture(L"NormalRT")));
					if (Depth_Prepass)
					{
						cmd_list->SetTexture(0, texture_offset + 2, render_graph.GetRHITexture(render_graph.GetTexture(L"Depth_Prepass_DepthStencil")));
					}
					else
					{
						cmd_list->SetTexture(0, texture_offset + 2, render_graph.GetRHITexture(render_graph.GetTexture(L"GBuffer_DepthStencil")));
					}
					cmd_list->SetTexture(0, texture_offset + 3, render_graph.GetRHITexture(render_graph.GetTexture(L"Cascade_Shadow_Tex")));

					cmd_list->SetPushConstant(0, sizeof(int), &output_texture);
					cmd_list->SetPushConstant(sizeof(int), sizeof(int), &cascade_override);

					cmd_list->Draw(3, 1, 0, 0);
					cmd_list->EndRenderpass();
				}, std::move(pass_data));
		}

		void Renderpass::Swapchain()
		{
			struct TestPassData
			{
				RGTextureHandle RenderTarget;
			};

			RenderGraph::Instance().AddPass<TestPassData>(L"SwapchainPass", [](TestPassData& data, RenderGraphBuilder& builder)
				{
					IS_PROFILE_SCOPE("Swapchain pass setup");

					RGTextureHandle rt = builder.GetTexture(L"Composite_Tex");
					builder.ReadTexture(rt);
					data.RenderTarget = rt;

					builder.WriteTexture(-1);

					ShaderDesc shaderDesc;
					shaderDesc.VertexFilePath = L"Resources/Shaders/hlsl/Swapchain.hlsl";
					shaderDesc.PixelFilePath = L"Resources/Shaders/hlsl/Swapchain.hlsl";
					builder.SetShader(shaderDesc);

					PipelineStateObject swapchainPso = { };
					swapchainPso.Name = L"Swapchain_PSO";
					swapchainPso.CullMode = CullMode::None;
					swapchainPso.ShaderDescription = shaderDesc;
					builder.SetPipeline(swapchainPso);

					builder.SetViewport(Window::Instance().GetWidth(), Window::Instance().GetHeight());
					builder.SetScissor(Window::Instance().GetWidth(), Window::Instance().GetHeight());

					builder.SetAsRenderToSwapchain();
				},
				[](TestPassData& data, RenderGraph& renderGraph, RHI_CommandList* cmdList)
				{
					IS_PROFILE_SCOPE("Swapchain pass execute");

					PipelineStateObject pso = renderGraph.GetPipelineStateObject(L"SwapchainPass");
					cmdList->BindPipeline(pso, nullptr);
					cmdList->BeginRenderpass(renderGraph.GetRenderpassDescription(L"SwapchainPass"));

					cmdList->SetTexture(0, 0, renderGraph.GetRHITexture(data.RenderTarget));
					cmdList->Draw(3, 1, 0, 0);

					cmdList->EndRenderpass();
				});
		}

		void Renderpass::ImGuiPass()
		{
			m_imgui_pass.Render();
		}


		void Renderpass::BindCommonResources(RHI_CommandList* cmd_list, BufferFrame& buffer_frame, BufferSamplers& buffer_samplers)
		{
			cmd_list->SetUniform(0, 0, buffer_frame);

			cmd_list->SetSampler(0, 1, buffer_samplers.Shadow_Sampler);
			cmd_list->SetSampler(0, 2, buffer_samplers.Repeat_Sampler);
			cmd_list->SetSampler(0, 3, buffer_samplers.Clamp_Sampler);
			cmd_list->SetSampler(0, 5, buffer_samplers.MirroredRepeat_Sampler);
		}

		float previousTime = 0;

		void Renderpass::UpdateCamera(BufferFrame& buffer_frame)
		{
			const float deltaTime = App::Engine::s_FrameTimer.GetElapsedTimeMillFloat();
			bool negative_viewport = false;

			glm::mat4 viewMatrix = buffer_frame.View;

			/// Get the camera's forward, right, up, and location vectors
			glm::vec4 vForward = viewMatrix[2];
			glm::vec4 vRight = viewMatrix[0];
			glm::vec4 vUp = viewMatrix[1];
			glm::vec4 vTranslation = viewMatrix[3];

			float frameSpeed = Input::InputManager::IsKeyPressed(IS_KEY_LEFT_SHIFT) ? deltaTime * 200 : deltaTime * 25;
			///Input::IsKeyDown(KEY_LEFT_SHIFT) ? a_deltaTime * m_cameraSpeed * 2 : a_deltaTime * m_cameraSpeed;

			/// Translate camera
			if (Input::InputManager::IsKeyPressed(IS_KEY_W))
			{
				vTranslation += vForward * frameSpeed;
			}
			if (Input::InputManager::IsKeyPressed(GLFW_KEY_S))
			{
				vTranslation -= vForward * frameSpeed;
			}
			if (Input::InputManager::IsKeyPressed(GLFW_KEY_D))
			{
				vTranslation += vRight * frameSpeed;
			}
			if (Input::InputManager::IsKeyPressed(GLFW_KEY_A))
			{
				vTranslation -= vRight * frameSpeed;
			}
			if (Input::InputManager::IsKeyPressed(GLFW_KEY_Q))
			{
				vTranslation += vUp * frameSpeed;
			}
			if (Input::InputManager::IsKeyPressed(GLFW_KEY_E))
			{
				vTranslation -= vUp * frameSpeed;
			}

			/// check for camera rotation
			static bool sbMouseButtonDown = false;
			bool mouseDown = Input::InputManager::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT);
			if (mouseDown)
			{
				viewMatrix[3] = vTranslation;

				static float siPrevMouseX = 0;
				static float siPrevMouseY = 0;

				if (sbMouseButtonDown == false)
				{
					sbMouseButtonDown = true;
					Input::InputManager::GetMousePosition(siPrevMouseX, siPrevMouseY);
				}

				float mouseX = 0, mouseY = 0;
				Input::InputManager::GetMousePosition(mouseX, mouseY);

				float iDeltaX = mouseX - siPrevMouseX;
				float iDeltaY = mouseY - siPrevMouseY;

				siPrevMouseX = mouseX;
				siPrevMouseY = mouseY;

				glm::mat4 mMat;

				/// pitch
				if (iDeltaY != 0)
				{
					float i_delta_y = static_cast<float>(iDeltaY);
					if (negative_viewport)
					{
						i_delta_y = -i_delta_y;
					}
					mMat = glm::axisAngleMatrix(vRight.xyz(), i_delta_y / 150.0f);
					vRight = mMat * vRight;
					vUp = mMat * vUp;
					vForward = mMat * vForward;
				}

				/// yaw
				if (iDeltaX != 0)
				{
					float i_delta_x = static_cast<float>(iDeltaX);
					mMat = glm::axisAngleMatrix(glm::vec3(0, 1, 0), i_delta_x / 150.0f);
					vRight = mMat * vRight;
					vUp = mMat * vUp;
					vForward = mMat * vForward;
				}

				viewMatrix[0] = vRight;
				viewMatrix[1] = vUp;
				viewMatrix[2] = vForward;

			}
			else
			{
				sbMouseButtonDown = false;
			}

			buffer_frame.View = viewMatrix;
			buffer_frame.View_Inverted = glm::inverse(viewMatrix);

			aspect = (float)Window::Instance().GetWidth() / (float)Window::Instance().GetHeight();
			aspect = std::max(0.1f, aspect);
			buffer_frame.Projection = glm::perspective(glm::radians(90.0f), aspect, Main_Camera_Near_Plane, Main_Camera_Far_Plane);

			// Setup the inverted projection view matrix.
			buffer_frame.Proj_View = buffer_frame.Projection * glm::inverse(buffer_frame.View);
			buffer_frame.Projection_View_Inverted = glm::inverse(buffer_frame.Proj_View);

			if (GraphicsManager::IsVulkan())
			{
				// Then invert the projection if vulkan.
				buffer_frame.Projection[1][1] *= -1;
				buffer_frame.Proj_View = buffer_frame.Projection * glm::inverse(buffer_frame.View);
			}
		}

		BufferLight BufferLight::GetCascades(const BufferFrame& buffer_frame, u32 cascade_count, float split_lambda)
		{
			std::vector<float> cascadeSplits;
			cascadeSplits.resize(cascade_count);
			BufferLight outCascades;

			const float nearClip = Main_Camera_Near_Plane;
			const float farClip = Main_Camera_Far_Plane;
			const float clipRange = farClip - nearClip;

			const float minZ = nearClip;
			const float maxZ = nearClip + clipRange;

			const float range = maxZ - minZ;
			const float ratio = maxZ / minZ;

			const float cascadeSplitLambda = split_lambda;

			/// Calculate split depths based on view camera frustum
			/// Based on method presented in https:///developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
			for (int i = 0; i < cascade_count; i++)
			{
				float p = (static_cast<float>(i) + 1.0f) / static_cast<float>(cascade_count);
				float log = minZ * std::pow(ratio, p);
				float uniform = minZ + range * p;
				float d = cascadeSplitLambda * (log - uniform) + uniform;
				cascadeSplits[i] = (d - nearClip) / clipRange;
			}

			/// Calculate orthographic projection matrix for each cascade
			float lastSplitDist = 0.0;
			for (u32 i = 0; i < cascade_count; i++)
			{
				float splitDist = cascadeSplits[i];

				glm::vec3 frustumCorners[8] =
				{
					glm::vec3(-1.0f,  1.0f, -1.0f),
					glm::vec3(1.0f,  1.0f, -1.0f),
					glm::vec3(1.0f, -1.0f, -1.0f),
					glm::vec3(-1.0f, -1.0f, -1.0f),
					glm::vec3(-1.0f,  1.0f,  1.0f),
					glm::vec3(1.0f,  1.0f,  1.0f),
					glm::vec3(1.0f, -1.0f,  1.0f),
					glm::vec3(-1.0f, -1.0f,  1.0f),
				};

				/// Project frustum corners into world space
				glm::mat4 invCam = glm::inverse(buffer_frame.Proj_View);
				for (u32 i = 0; i < 8; ++i)
				{
					glm::vec4 invCorner = invCam * glm::vec4(frustumCorners[i], 1.0f);
					frustumCorners[i] = invCorner / invCorner.w;
				}

				for (u32 i = 0; i < 4; ++i)
				{
					glm::vec3 dist = frustumCorners[i + 4] - frustumCorners[i];
					frustumCorners[i + 4] = frustumCorners[i] + (dist * splitDist);
					frustumCorners[i] = frustumCorners[i] + (dist * lastSplitDist);
				}

				/// Get frustum center
				glm::vec3 frustumCenter = glm::vec3(0.0f);
				for (u32 i = 0; i < 8; ++i)
				{
					frustumCenter += frustumCorners[i];
				}
				frustumCenter /= 8.0f;

				float radius = 0.0f;
				for (u32 i = 0; i < 8; ++i)
				{
					float distance = glm::length(frustumCorners[i] - frustumCenter);
					radius = glm::max(radius, distance);
				}
				radius = std::ceil(radius * 16.0f) / 16.0f;

				glm::vec3 maxExtents = glm::vec3(radius);
				glm::vec3 minExtents = -maxExtents;

				glm::mat4 reverse_z = glm::mat4(1.0f);
				reverse_z[2][2] = -1;
				reverse_z[2][3] = 1;

				/// Construct our matrixs required for the light.
				glm::vec3 lightDirection = dir_light_direction;
				glm::vec3 lightPosition = frustumCenter - glm::normalize(lightDirection) * -minExtents.z;
				glm::mat4 lightViewMatrix = glm::lookAt(lightPosition, frustumCenter, glm::vec3(0.0f, 1.0f, 0.0f));
				glm::mat4 lightOrthoMatrix = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f, maxExtents.z - minExtents.z);

				if (Reverse_Z_For_Depth)
				{
					glm::mat4 proj = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, maxExtents.z - minExtents.z, 0.0f);
					lightOrthoMatrix = reverse_z * lightOrthoMatrix;
					lightOrthoMatrix = proj;
				}

				{
					if (GraphicsManager::IsVulkan())
					{
						/// Invert the projection if vulkan. This is because vulkan's coord space is from top left, not bottom left.
						lightOrthoMatrix[1][1] *= -1;
					}

					/// Store split distance and matrix in cascade
					outCascades.SplitDepth[i] = (Main_Camera_Near_Plane + splitDist * clipRange) * -1.0f;
					outCascades.ProjView[i] = lightOrthoMatrix * lightViewMatrix;
					outCascades.Light_Direction = glm::vec4(glm::normalize(frustumCenter - lightPosition), 0.0);
				}
				lastSplitDist = cascadeSplits[i];
			}
			return outCascades;
		}
	}
}