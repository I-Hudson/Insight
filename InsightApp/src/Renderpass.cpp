#include "Renderpass.h"
#include "App/Engine.h"

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

#include <glm/gtx/matrix_interpolation.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Insight
{
	const float ShadowZNear = 0.1f;
	const float ShadowZFar = 2048.0f;
	const float ShadowFOV = 65.0f;
	const u32 Shadow_Depth_Tex_Size = 1024 * 4;

	const float Main_Camera_Near_Plane = 0.1f;
	const float Main_Camera_Far_Plane = 2048.0f;

	bool Render_Size_Related_To_Window_Size = true;
	int Render_Width = 640;
	int Render_Height = 720;

	float cascade_split_lambda = 0.55f;

	glm::vec3 dir_light_direction = glm::vec3(0.5f, -0.7f, 0.5f);

#define ECS_RENDER

	namespace Graphics
	{
		float aspect = 0.0f;
		void Renderpass::Create()
		{
			//m_testMesh.LoadFromFile("./Resources/models/vulkanscene_shadow_100.gltf");
			//m_testMesh.LoadFromFile("./Resources/models/vulkanscene_shadow_20.gltf");
			//m_testMesh.LoadFromFile("./Resources/models/plane.gltf");
			//m_testMesh.LoadFromFile("./Resources/models/sponza_old/sponza.obj");
			m_testMesh.LoadFromFile("./Resources/models/sponza/NewSponza_Main_Blender_glTF.gltf");
			//m_testMesh.LoadFromFile("./Resources/models/survival_guitar_backpack_fbx/source/Survival_BackPack_2.fbx");

			RPtr<App::Scene> active_scene = App::SceneManager::Instance().GetActiveScene().Lock();
			ECS::Entity* Test_mesh_entity = active_scene->GetECSWorld()->AddEntity("Test Mesh Entity");
			ECS::MeshComponent* mesh_component = static_cast<ECS::MeshComponent*>(Test_mesh_entity->AddComponentByName(ECS::MeshComponent::Type_Name));
			mesh_component->SetMesh(&m_testMesh);

			if (m_camera.View == glm::mat4(0.0f))
			{
				aspect = (float)Window::Instance().GetWidth() / (float)Window::Instance().GetHeight();
				m_camera.Projection = glm::perspective(glm::radians(90.0f), aspect, 0.1f, 1024.0f);
				m_camera.View = glm::mat4(1.0f);
			}
			m_imgui_pass.Create();
		}

		bool useShadowCamera = false;

		void Renderpass::Render()
		{
			IS_PROFILE_FUNCTION();

			IMGUI_VALID(ImGui::Checkbox("Move shadow camera", &useShadowCamera));

			if (Render_Size_Related_To_Window_Size)
			{
				Render_Width = Window::Instance().GetWidth();
				Render_Height = Window::Instance().GetHeight();
			}

			UpdateCamera(m_camera);
			ShadowPass();
			Sample(m_camera);
			Composite();
			Swapchain();
			ImGuiPass();
		}

		void Renderpass::Destroy()
		{
			GraphicsManager::Instance().GetRenderContext()->GpuWaitForIdle();
			m_imgui_pass.Release();
			m_testMesh.Destroy();
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

		UBO_ShadowCamera GetReverseZDepthCamera(UBO_ShadowCamera camera, bool inverse)
		{
			UBO_ShadowCamera shadowCamera = camera;

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
			shadowCamera.Projection = MakeInfReversedZProjRH(glm::radians(ShadowFOV), 1.0f, ShadowZNear);
			shadowCamera.ProjView = shadowCamera.Projection * (inverse ? glm::inverse(shadowCamera.View) : shadowCamera.View);

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
				Mesh& Mesh;
				std::vector<UBO_ShadowCamera> Cameras;
				RGTextureHandle Depth_Tex;
			};
			PassData data = { m_testMesh };
			data.Cameras = UBO_ShadowCamera::GetCascades(m_camera, 4, cascade_split_lambda);

			IMGUI_VALID(ImGui::Begin("Directional Light Direction"));
			float dir[3] = { dir_light_direction.x, dir_light_direction.y, dir_light_direction.z };
			IMGUI_VALID(if (ImGui::DragFloat3("Direction", dir, 0.001f, -1.0f, 1.0f))
			{
				dir_light_direction = glm::vec3(dir[0], dir[1], dir[2]);
			});

			ImGuiMat4(m_camera.View);
			IMGUI_VALID(ImGui::Begin("Shadow pass"));
			IMGUI_VALID(ImGui::Text("Cameras"));
			for (UBO_ShadowCamera& c : data.Cameras)
			{
				IMGUI_VALID(ImGui::Text("Projection View"));
				ImGuiMat4(c.ProjView);
				IMGUI_VALID(ImGui::Text("Projection"));
				ImGuiMat4(c.Projection);
				IMGUI_VALID(ImGui::Text("View"));
				ImGuiMat4(c.View);
				IMGUI_VALID(ImGui::Separator());
				IMGUI_VALID(ImGui::Separator());
			}
			static float depth_constant_factor = 4.0f;
			static float depth_slope_factor = 1.5f;
			IMGUI_VALID(ImGui::DragFloat("Dpeth bias constant factor", &depth_constant_factor, 0.01f));
			IMGUI_VALID(ImGui::DragFloat("Dpeth bias slope factor", &depth_slope_factor, 0.01f));
			IMGUI_VALID(ImGui::DragFloat("Cascade Split Lambda", &cascade_split_lambda, 0.001f, 0.0f, 1.0f));

			IMGUI_VALID(ImGui::End());

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
					///pso.CullMode = CullMode::Front;
					pso.FrontFace = FrontFace::CounterClockwise;
					pso.DepthClampEnabled = false;
					pso.DepthBaisEnabled = true;
					pso.Dynamic_States = { DynamicState::Viewport, DynamicState::Scissor, DynamicState::DepthBias };
					builder.SetPipeline(pso);
				},
				[](PassData& data, RenderGraph& render_graph, RHI_CommandList* cmdList)
				{
					IS_PROFILE_SCOPE("Cascade shadow pass execute");

					PipelineStateObject pso = render_graph.GetPipelineStateObject(L"Cascade shadow pass");
					cmdList->BindPipeline(pso, nullptr);

					cmdList->SetDepthBias(depth_constant_factor, 0.0f, depth_slope_factor);
						
					WPtr<App::Scene> w_scene = App::SceneManager::Instance().GetActiveScene();
					std::vector<Ptr<ECS::Entity>> entities;
					if (RPtr<App::Scene> scene = w_scene.Lock())
					{
						entities = scene->GetAllEntitiesWithComponentByName(ECS::MeshComponent::Type_Name);
					}

					RHI_Texture* depth_tex = render_graph.GetRHITexture(data.Depth_Tex);
					for (u32 i = 0; i < depth_tex->GetInfo().Layer_Count; ++i)
					{
						RenderpassDescription renderpass_description = render_graph.GetRenderpassDescription(L"Cascade shadow pass");
						renderpass_description.DepthStencilAttachment.Layer_Array_Index = static_cast<u32>(i);
						cmdList->BeginRenderpass(renderpass_description);

						cmdList->SetUniform(0, 0, &data.Cameras.at(i), sizeof(data.Cameras.at(i)));

						Frustum camera_frustum(data.Cameras.at(i).View, data.Cameras.at(i).Projection,	1000.0f);
#ifdef ECS_RENDER
						for (const Ptr<ECS::Entity> e : entities)
						{
							static_cast<ECS::MeshComponent*>(e->GetComponentByName(ECS::MeshComponent::Type_Name))->GetMesh()->Draw(cmdList);
						}
#else
						for (Submesh* sub_mesh : data.Mesh.GetSubMeshes())
						{
							BoundingBox bounding_box = sub_mesh->GetBoundingBox();
							///if (camera_frustum.IsVisible(bounding_box.GetCenter(), bounding_box.GetExtents()))
							{
								sub_mesh->Draw(cmdList);
							}
						}
#endif

						cmdList->EndRenderpass();
					}
				}, std::move(data));
		}

		void Renderpass::Sample(UBO_Camera& camera)
		{
			IS_PROFILE_FUNCTION();

#ifdef RENDER_GRAPH_ENABLED
			struct TestPassData
			{
				Mesh& TestMesh;
			};
			TestPassData passData =
			{
				m_testMesh
			};

			static int camera_index = 0;
			static const char* camera_names[] = { "Default", "Shadow 0", "Shadow 1", "Shadow 2", "Shadow 3" };
			IMGUI_VALID(ImGui::Begin("Camera index"));
			IMGUI_VALID(ImGui::ListBox("Cameras", &camera_index, camera_names, ARRAY_COUNT(camera_names)));
			IMGUI_VALID(ImGui::End());

			std::vector<UBO_ShadowCamera> shader_cameras = UBO_ShadowCamera::GetCascades(m_camera, 4, cascade_split_lambda);

			RenderGraph::Instance().AddPass<TestPassData>(L"GBuffer", [](TestPassData& data, RenderGraphBuilder& builder)
				{
					IS_PROFILE_SCOPE("GBuffer pass setup");

					RHI_TextureCreateInfo textureCreateInfo = RHI_TextureCreateInfo::Tex2D(
						  Render_Width
						, Render_Height
						, PixelFormat::R8G8B8A8_UNorm
						, ImageUsageFlagsBits::ColourAttachment | ImageUsageFlagsBits::Sampled);
					RGTextureHandle colourRT = builder.CreateTexture(L"ColourRT", textureCreateInfo);
					builder.WriteTexture(colourRT);

					textureCreateInfo = RHI_TextureCreateInfo::Tex2D(
						  Render_Width
						, Render_Height
						, PixelFormat::R16G16B16A16_Float
						, ImageUsageFlagsBits::ColourAttachment | ImageUsageFlagsBits::Sampled);
					RGTextureHandle normal_rt = builder.CreateTexture(L"NormalRT", textureCreateInfo);
					builder.WriteTexture(normal_rt);

					textureCreateInfo = RHI_TextureCreateInfo::Tex2D(
						  Render_Width
						, Render_Height
						, PixelFormat::R16G16_Float
						, ImageUsageFlagsBits::ColourAttachment | ImageUsageFlagsBits::Sampled);
					RGTextureHandle velocity_rt = builder.CreateTexture(L"VelocityRT", textureCreateInfo);
					builder.WriteTexture(velocity_rt);

					textureCreateInfo.Format = PixelFormat::D32_Float;
					textureCreateInfo.ImageUsage = ImageUsageFlagsBits::DepthStencilAttachment | ImageUsageFlagsBits::Sampled;
					RGTextureHandle depthStencil = builder.CreateTexture(L"GBuffer_DepthStencil", textureCreateInfo);
					builder.WriteDepthStencil(depthStencil);

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
					}
					builder.SetPipeline(gbufferPso);

					builder.SetViewport(Window::Instance().GetWidth(), Window::Instance().GetHeight());
					builder.SetScissor(Window::Instance().GetWidth(), Window::Instance().GetHeight());
				},
				[&camera, shader_cameras](TestPassData& data, RenderGraph& render_graph, RHI_CommandList* cmdList)
				{
					IS_PROFILE_SCOPE("GBuffer pass execute");

					PipelineStateObject pso = render_graph.GetPipelineStateObject(L"GBuffer");
					cmdList->BindPipeline(pso, nullptr);
					cmdList->BeginRenderpass(render_graph.GetRenderpassDescription(L"GBuffer"));

					UBO_Camera camera_to_bind;
					if (camera_index == 0)
					{
						camera_to_bind = camera;
					}
					else if (camera_index == 1)
					{
						const u32 i = 0;
						camera_to_bind = { shader_cameras.at(i).ProjView, shader_cameras.at(i).Projection, shader_cameras.at(i).View };
					}
					else if (camera_index == 2)
					{
						const u32 i = 1;
						camera_to_bind = { shader_cameras.at(i).ProjView, shader_cameras.at(i).Projection, shader_cameras.at(i).View };
					}
					else if (camera_index == 3)
					{
						const u32 i = 2;
						camera_to_bind = { shader_cameras.at(i).ProjView, shader_cameras.at(i).Projection, shader_cameras.at(i).View };
					}
					else if (camera_index == 4)
					{
						const u32 i = 3;
						camera_to_bind = { shader_cameras.at(i).ProjView, shader_cameras.at(i).Projection, shader_cameras.at(i).View };
					}
					{
						IS_PROFILE_SCOPE("GBuffer-SetUniform");
						cmdList->SetUniform(0, 0, &camera_to_bind, sizeof(camera_to_bind));
					}

					WPtr<App::Scene> w_scene = App::SceneManager::Instance().GetActiveScene();
					std::vector<Ptr<ECS::Entity>> entities;
					if (RPtr<App::Scene> scene = w_scene.Lock())
					{
						entities = scene->GetAllEntitiesWithComponentByName(ECS::MeshComponent::Type_Name);
					}

					Frustum camera_frustum(camera.View, camera.Projection, 1000.0f);
					
#ifdef ECS_RENDER
					for (const Ptr<ECS::Entity> e : entities)
					{
						static_cast<ECS::MeshComponent*>(e->GetComponentByName(ECS::MeshComponent::Type_Name))->GetMesh()->Draw(cmdList);
					}
#else
					for (Submesh* sub_mesh : data.TestMesh.GetSubMeshes())
					{
						BoundingBox bounding_box = sub_mesh->GetBoundingBox();
						///if (camera_frustum.IsVisible(bounding_box.GetCenter(), bounding_box.GetExtents()))
						{
							sub_mesh->Draw(cmdList);
						}
					}
#endif

					cmdList->EndRenderpass();
				}, std::move(passData));
#endif ///RENDER_GRAPH_ENABLED
		}

		void Renderpass::Composite()
		{
			struct PassData
			{ };

			static int output_texture;
			static int cascade_override;
			const char* items[] = { "Colour", "World Normal", "World Position", "Shadow", "Colour + Shadow", "View Position", "Cascade splits", "Shadow NDC Z"};
			const char* cascade_override_items[] = { "0", "1", "2", "3" };

			IMGUI_VALID(ImGui::Begin("Composite pass"));
			IMGUI_VALID(ImGui::ListBox("Display shadow", &output_texture, items, ARRAY_COUNT(items)));
			IMGUI_VALID(ImGui::ListBox("Cascde Index shadow", &cascade_override, cascade_override_items, ARRAY_COUNT(cascade_override_items)));
			IMGUI_VALID(ImGui::End());

			std::vector<UBO_ShadowCamera> shader_cameras = UBO_ShadowCamera::GetCascades(m_camera, 4, cascade_split_lambda);

			RenderGraph::Instance().AddPass<PassData>(L"Composite_Pass", 
				[](PassData& data, RenderGraphBuilder& builder)
				{		
					builder.ReadTexture(builder.GetTexture(L"ColourRT"));
					builder.ReadTexture(builder.GetTexture(L"NormalRT"));
					builder.ReadTexture(builder.GetTexture(L"NormalRT"));
					builder.ReadTexture(builder.GetTexture(L"GBuffer_DepthStencil"));
					builder.ReadTexture(builder.GetTexture(L"Cascade_Shadow_Tex"));

					RHI_TextureCreateInfo create_info = RHI_TextureCreateInfo::Tex2D(
						Render_Width
						, Render_Height
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
				[this, shader_cameras](PassData& data, RenderGraph& render_graph, RHI_CommandList* cmd_list)
				{
					PipelineStateObject pso = render_graph.GetPipelineStateObject(L"Composite_Pass");
					cmd_list->BindPipeline(pso, nullptr);
					cmd_list->BeginRenderpass(render_graph.GetRenderpassDescription(L"Composite_Pass"));

					UBO_Camera camera = m_camera;
					cmd_list->SetUniform(0, 0, camera);
					cmd_list->SetUniform(0, 1, shader_cameras.data(), static_cast<u32>(shader_cameras.size() * sizeof(UBO_ShadowCamera)));

					cmd_list->SetTexture(1, 0, render_graph.GetRHITexture(render_graph.GetTexture(L"ColourRT")));
					cmd_list->SetTexture(1, 1, render_graph.GetRHITexture(render_graph.GetTexture(L"NormalRT")));
					cmd_list->SetTexture(1, 2, render_graph.GetRHITexture(render_graph.GetTexture(L"GBuffer_DepthStencil")));

					RHI_SamplerCreateInfo sampler_create_info = { };
					sampler_create_info.MagFilter = Filter::Linear;
					sampler_create_info.MinFilter = Filter::Linear;
					sampler_create_info.MipmapMode = SamplerMipmapMode::Nearest;
					sampler_create_info.AddressMode = SamplerAddressMode::ClampToEdge;
					sampler_create_info.CompareEnabled = true;
					sampler_create_info.CompareOp = CompareOp::Less;
					RHI_Sampler* shadow_sampler = GraphicsManager::Instance().GetRenderContext()->GetSamplerManager().GetOrCreateSampler(sampler_create_info);

					cmd_list->SetTexture(1, 3, render_graph.GetRHITexture(render_graph.GetTexture(L"Cascade_Shadow_Tex")));
					cmd_list->SetSampler(1, 4, shadow_sampler);

					cmd_list->SetPushConstant(0, sizeof(int), &output_texture);
					cmd_list->SetPushConstant(sizeof(int), sizeof(int), &cascade_override);

					cmd_list->Draw(3, 1, 0, 0);
					cmd_list->EndRenderpass();
				});
		}

		void Renderpass::Swapchain()
		{
#ifdef RENDER_GRAPH_ENABLED
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
#endif ///RENDER_GRAPH_ENABLED
		}

		void Renderpass::ImGuiPass()
		{
#ifdef RENDER_GRAPH_ENABLED
			m_imgui_pass.Render();
#endif ///RENDER_GRAPH_ENABLED
		}

		float previousTime = 0;

		void Renderpass::UpdateCamera(UBO_Camera& camera)
		{
			const float deltaTime = App::Engine::s_FrameTimer.GetElapsedTimeMillFloat();
			bool negative_viewport = false;

			glm::mat4 viewMatrix = camera.View;

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

			camera.View = viewMatrix;
			camera.View_Inverted = glm::inverse(viewMatrix);

			aspect = (float)Window::Instance().GetWidth() / (float)Window::Instance().GetHeight();
			aspect = std::max(0.1f, aspect);
			camera.Projection = glm::perspective(glm::radians(90.0f), aspect, Main_Camera_Near_Plane, Main_Camera_Far_Plane);

			// Setup the inverted projection view matrix.
			camera.ProjView = camera.Projection * glm::inverse(camera.View);
			camera.Projection_View_Inverted = glm::inverse(camera.ProjView);

			if (GraphicsManager::IsVulkan())
			{
				// Then invert the projection if vulkan.
				camera.Projection[1][1] *= -1;
				camera.ProjView = camera.Projection * glm::inverse(camera.View);
			}
		}

		std::vector<UBO_ShadowCamera> UBO_ShadowCamera::GetCascades(const UBO_Camera& camera, int cascadeCount, float split_lambda)
		{
			std::vector<float> cascadeSplits;
			cascadeSplits.resize(cascadeCount);
			std::vector<UBO_ShadowCamera> outCascades;
			outCascades.resize(cascadeCount);

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
			for (int i = 0; i < cascadeCount; i++)
			{
				float p = (static_cast<float>(i) + 1.0f) / static_cast<float>(cascadeCount);
				float log = minZ * std::pow(ratio, p);
				float uniform = minZ + range * p;
				float d = cascadeSplitLambda * (log - uniform) + uniform;
				cascadeSplits[i] = (d - nearClip) / clipRange;
			}

			/// Calculate orthographic projection matrix for each cascade
			float lastSplitDist = 0.0;
			for (u32 i = 0; i < cascadeCount; i++)
			{
				float splitDist = cascadeSplits[i];

				glm::vec3 frustumCorners[8] = 
				{
					glm::vec3(-1.0f,  1.0f, -1.0f),
					glm::vec3( 1.0f,  1.0f, -1.0f),
					glm::vec3( 1.0f, -1.0f, -1.0f),
					glm::vec3(-1.0f, -1.0f, -1.0f),
					glm::vec3(-1.0f,  1.0f,  1.0f),
					glm::vec3( 1.0f,  1.0f,  1.0f),
					glm::vec3( 1.0f, -1.0f,  1.0f),
					glm::vec3(-1.0f, -1.0f,  1.0f),
				};

				/// Project frustum corners into world space
				glm::mat4 invCam = glm::inverse(camera.ProjView);
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

				/// Construct our matrixs required for the light.
				glm::vec3 lightDirection = dir_light_direction;
				glm::vec3 lightPosition = frustumCenter - glm::normalize(lightDirection) * -minExtents.z;
				glm::mat4 lightViewMatrix = glm::lookAt(lightPosition, frustumCenter, glm::vec3(0.0f, 1.0f, 0.0f));
				glm::mat4 lightOrthoMatrix = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f, maxExtents.z - minExtents.z);

				{
					if (GraphicsManager::IsVulkan())
					{
						/// Invert the projection if vulkan. This is because vulkan's coord space is from top left, not bottom left.
						lightOrthoMatrix[1][1] *= -1;
					}

					/// Store split distance and matrix in cascade
					outCascades[i].SplitDepth = (Main_Camera_Near_Plane + splitDist * clipRange) * -1.0f;
					outCascades[i].ProjView = lightOrthoMatrix * lightViewMatrix;
					outCascades[i].Projection = lightOrthoMatrix;
					outCascades[i].View = lightViewMatrix;
					outCascades[i].Light_Direction = glm::vec4(glm::normalize(frustumCenter - lightPosition), 0.0);
				}
				lastSplitDist = cascadeSplits[i];
			}
			return outCascades;
		}
	}
}