#include "Renderpass.h"
#include "Runtime/Engine.h"

#include "Graphics/RenderContext.h"

#include "Graphics/RenderGraph/RenderGraph.h"
#include "Graphics/RenderGraph/RenderGraphBuilder.h"
#include "Graphics/RenderGraphV2/RenderGraphV2.h"
#include "Graphics/Frustum.h"
#include "Graphics/Window.h"
#include "Graphics/GFXHelper.h"

#include "Core/Profiler.h"
#include "Core/Logger.h"
#include "Core/EnginePaths.h"

#include "World/WorldSystem.h"
#include "ECS/Components/TransformComponent.h"

#include "Animation/AnimationSystem.h"

#include "Maths/Utils.h"

#include "Asset/Assets/Model.h"
#include "Resource/Mesh.h"

namespace Insight
{
//#define SHADOW_PASS_SCENE_OBEJCTS

	const float ShadowZNear = 0.1f;
	const float ShadowZFar = 512.0f;
	const float ShadowFOV = 65.0f;
	const u32 Shadow_Depth_Tex_Size = 1024 * 4;

	constexpr bool Reverse_Z_For_Depth = false;
	constexpr bool Depth_Prepass = false;

	const float Main_Camera_Near_Plane = 0.1f;
	const float Main_Camera_Far_Plane = 1024.0f;

	float cascade_split_lambda = 0.85f;

	Maths::Vector3 dir_light_direction = Maths::Vector3(0.5f, -0.7f, 0.5f);

	int pendingRenderResolution[2] = { 0, 0 };

	Graphics::Frustum MainCameraFrustum = {};
	bool UseCustomFrustum = false;

	std::vector<std::pair<Ref<Runtime::ModelAsset>, bool>> modelsToAddToScene;

	static bool enableFSR = false;
	static float fsrSharpness = 1.0f;

	static int MeshLod = 0;
	static bool RenderMaterialBatching = false;

	RenderFrame renderFrame;

	enum class DefaultModels
	{
		None,
		Backpack,
		SponzaMain,
		SponzaMain_Curtains,
		SponzaFull,
	};
	constexpr DefaultModels DefaultModelToLoad = DefaultModels::None;

	namespace Graphics
	{

		struct GlobalResources
		{
			RHI_BufferView Buffer_Frame_View = { };
			RHI_BufferView Buffer_Directional_Light_View = { };
		};
		GlobalResources g_global_resources = {};

		BufferFrame::BufferFrame()
		{
			SetGPUSkinningEnabled(Runtime::AnimationSystem::Instance().IsGPUSkinningEnabled());
		}

		float aspect = 0.0f;
		void Renderpass::Create()
		{
			IS_PROFILE_FUNCTION();

			if constexpr (DefaultModelToLoad == DefaultModels::Backpack)
			{
				Ref<Runtime::ModelAsset> model_backpack = Runtime::AssetRegistry::Instance().LoadAsset(EnginePaths::GetResourcePath() + "/models/Survival_BackPack_2/obj/backpack.obj").As<Runtime::ModelAsset>();
				modelsToAddToScene.push_back({ model_backpack, false });
			}
			else if constexpr (DefaultModelToLoad == DefaultModels::SponzaMain)
			{
				Ref<Runtime::ModelAsset> model_sponza = Runtime::AssetRegistry::Instance().LoadAsset(EnginePaths::GetResourcePath() + "/models/Sponza/Sponza/NewSponza_Main_glTF_002.gltf").As<Runtime::ModelAsset>();
				modelsToAddToScene.push_back({ model_sponza, false });
			}
			else if constexpr (DefaultModelToLoad == DefaultModels::SponzaMain_Curtains)
			{
				Ref<Runtime::ModelAsset> model_sponza = Runtime::AssetRegistry::Instance().LoadAsset(EnginePaths::GetResourcePath() + "/models/Sponza/Sponza/NewSponza_Main_glTF_002.gltf").As<Runtime::ModelAsset>();
				Ref<Runtime::ModelAsset> model_sponza_curtains = Runtime::AssetRegistry::Instance().LoadAsset(EnginePaths::GetResourcePath() + "/models/Sponza/Curtains/NewSponza_Curtains_glTF.gltf").As<Runtime::ModelAsset>();
				modelsToAddToScene.push_back({ model_sponza, false });
				modelsToAddToScene.push_back({ model_sponza_curtains, false });
			}
			else if constexpr (DefaultModelToLoad == DefaultModels::SponzaFull)
			{
				Ref<Runtime::ModelAsset> model_sponza = Runtime::AssetRegistry::Instance().LoadAsset(EnginePaths::GetResourcePath() + "/models/Sponza/Sponza/NewSponza_Main_glTF_002.gltf").As<Runtime::ModelAsset>();
				Ref<Runtime::ModelAsset> model_sponza_curtains = Runtime::AssetRegistry::Instance().LoadAsset(EnginePaths::GetResourcePath() + "/models/Sponza/Curtains/NewSponza_Curtains_glTF.gltf").As<Runtime::ModelAsset>();
				Ref<Runtime::ModelAsset> model_sponza_ivy = Runtime::AssetRegistry::Instance().LoadAsset(EnginePaths::GetResourcePath() + "/models/Sponza/Ivy/NewSponza_IvyGrowth_glTF.gltf").As<Runtime::ModelAsset>();
				Ref<Runtime::ModelAsset> model_sponza_trees = Runtime::AssetRegistry::Instance().LoadAsset(EnginePaths::GetResourcePath() + "/models/Sponza/Trees/NewSponza_CypressTree_glTF.gltf").As<Runtime::ModelAsset>();
				
				modelsToAddToScene.push_back({ model_sponza, false });
				modelsToAddToScene.push_back({ model_sponza_curtains, false });
				modelsToAddToScene.push_back({ model_sponza_ivy, false });
				modelsToAddToScene.push_back({ model_sponza_trees, false });
			}

			m_buffer_frame = {};
			aspect = (float)Window::Instance().GetWidth() / (float)Window::Instance().GetHeight();
			m_buffer_frame.Projection = Maths::Matrix4::CreatePerspective(Maths::DegreesToRadians(90.0f), aspect, 0.1f, 1024.0f);
			m_buffer_frame.View = Maths::Matrix4::Identity;
			MainCameraFrustum = Graphics::Frustum(m_buffer_frame.View, m_buffer_frame.Projection, Main_Camera_Far_Plane);

			RenderContext* render_context = &RenderContext::Instance();
			RHI_SamplerManager& sampler_manager = render_context->GetSamplerManager();

			RHI_SamplerCreateInfo sampler_create_info = { };
			sampler_create_info.MagFilter = Filter::Linear;
			sampler_create_info.MinFilter = Filter::Linear;
			sampler_create_info.MipmapMode = SamplerMipmapMode::Nearest;
			sampler_create_info.AddressMode = SamplerAddressMode::ClampToBoarder;
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
			sampler_create_info.AddressMode = SamplerAddressMode::ClampToBoarder;
			m_buffer_samplers.ClampToBoarder_Sampler = sampler_manager.GetOrCreateSampler(sampler_create_info);
			sampler_create_info.AddressMode = SamplerAddressMode::MirroredRepeat;
			m_buffer_samplers.MirroredRepeat_Sampler = sampler_manager.GetOrCreateSampler(sampler_create_info);

			const u32 width = Window::Instance().GetWidth();
			const u32 height = Window::Instance().GetHeight();
			RenderGraph::Instance().SetRenderResolution(Maths::Vector2(width, height));
			RenderGraph::Instance().SetOutputResolution(Maths::Vector2(width, height));

			pendingRenderResolution[0] = RenderGraph::Instance().GetRenderResolution().x;
			pendingRenderResolution[1] = RenderGraph::Instance().GetRenderResolution().y;

			m_imgui_pass.Create();

			CreateAllCommonShaders();

			Graphics::RHI_FSR::Instance().Init();
		}

		void Renderpass::Render(bool render)
		{
			IS_PROFILE_FUNCTION();

			ImGui::Begin("Renderpass options:");
			ImGui::SliderInt("Mesh Lods", &MeshLod, 0, Runtime::Mesh::s_MAX_LOD_COUNT - 1);
			ImGui::Checkbox("Use Material Batching", &RenderMaterialBatching);
			ImGui::End();

			{
				IS_PROFILE_SCOPE("Add models to scene");
				for (size_t i = 0; i < modelsToAddToScene.size(); ++i)
				{
					if (modelsToAddToScene.at(i).first
						&& !modelsToAddToScene.at(i).second
						&& modelsToAddToScene.at(i).first->GetAssetState() == Runtime::AssetState::Loaded)
					{
						modelsToAddToScene.at(i).second = true;
						for (size_t modelCreateIdx = 0; modelCreateIdx < 1; ++modelCreateIdx)
						{
							ECS::Entity* entity = modelsToAddToScene.at(i).first->CreateEntityHierarchy();

							Maths::Matrix4 transform = Maths::Matrix4::Identity;
							//transform[3] = Maths::Vector4(10.0f, 0.0f, 25.0f, 1.0f);
							entity->GetComponent<ECS::TransformComponent>()->SetTransform(transform);
						}

						IS_LOG_CORE_INFO("Model '{}' added to scene.", modelsToAddToScene.at(i).first->GetFileName());
					}
				}
			}

			{
				IS_PROFILE_SCOPE("Render Resolution");
				ImGui::Begin("Render");
				ImGui::DragInt2("Render Resolution", pendingRenderResolution);
				if (ImGui::Button("Apply Render Resolution"))
				{
					RenderGraph::Instance().SetRenderResolution(Maths::Vector2(pendingRenderResolution[0], pendingRenderResolution[1]));
				}
				ImGui::End();
			}

			{
				IS_PROFILE_SCOPE("Create render frame");
				renderFrame = App::Engine::Instance().GetSystemRegistry().GetSystem<Runtime::GraphicsSystem>()->GetRenderFrame();
			}

			{
				IS_PROFILE_SCOPE("BufferFrame cameras");
				m_buffer_frame.Proj_View = renderFrame.MainCamera.Camera.GetProjectionViewMatrix();
				m_buffer_frame.Projection = renderFrame.MainCamera.Camera.GetProjectionMatrix();
				m_buffer_frame.View = renderFrame.MainCamera.Transform;
			}

			{
				IS_PROFILE_SCOPE("FSR2");
				if (enableFSR)
				{
					Maths::Vector2 const renderResolution = RenderGraph::Instance().GetRenderResolution();
					RHI_FSR::Instance().GenerateJitterSample(&m_taaJitterX, &m_taaJitterY);
					m_taaJitterX = (m_taaJitterX / static_cast<float>(renderResolution.x));
					m_taaJitterY = (m_taaJitterY / static_cast<float>(renderResolution.y));

					Maths::Matrix4 translation = Maths::Matrix4::Identity.Translated(Maths::Vector4(m_taaJitterX, m_taaJitterY, 0.0f, 0.0f));
					Maths::Matrix4 trans = Maths::Matrix4::Identity.Translated(Maths::Vector3(m_taaJitterX, m_taaJitterY, 0.0f));
					m_buffer_frame.Projection = m_buffer_frame.Projection * translation;

					if (RenderContext::Instance().GetGraphicsAPI() == GraphicsAPI::Vulkan)
					{
						Maths::Matrix4 proj = m_buffer_frame.Projection;
						proj[1][1] *= -1;
						m_buffer_frame.Proj_View = proj * m_buffer_frame.View.Inversed();
					}
					else
					{
						m_buffer_frame.Proj_View = m_buffer_frame.Projection * m_buffer_frame.View.Inversed();
					}
				}
			}

			{
				IS_PROFILE_SCOPE("BufferFrame resolutions");
				m_buffer_frame.View_Inverted = renderFrame.MainCamera.Camera.GetInvertedViewMatrix();
				m_buffer_frame.Projection_View_Inverted = renderFrame.MainCamera.Camera.GetInvertedProjectionViewMatrix();

				m_buffer_frame.Render_Resolution[0] = RenderGraph::Instance().GetRenderResolution().x;
				m_buffer_frame.Render_Resolution[1] = RenderGraph::Instance().GetRenderResolution().y;


				m_buffer_frame.Ouput_Resolution[0] = RenderGraph::Instance().GetOutputResolution().x;
				m_buffer_frame.Ouput_Resolution[1] = RenderGraph::Instance().GetOutputResolution().y;
			}

			{
				IS_PROFILE_SCOPE("GetCascades");
				BufferLight::GetCascades(m_directional_light, m_buffer_frame, 4, 0.95f);
			}

			RenderGraph::Instance().AddPreRender([this](RenderGraph& render_graph, RHI_CommandList* cmd_list)
				{
					g_global_resources.Buffer_Frame_View = cmd_list->UploadUniform(m_buffer_frame);
					g_global_resources.Buffer_Directional_Light_View = cmd_list->UploadUniform(m_directional_light);
				});
			RenderGraph::Instance().AddPostRender([this](RenderGraph& render_graph, RHI_CommandList* cmd_list)
				{
					GFXHelper::Reset();
				});

			if (render)
			{
				ShadowPass();
				//ShadowCullingPass();
				if (Depth_Prepass)
				{
					//DepthPrepass();
				}
				GBuffer();
				TransparentGBuffer();
				Composite();
				FSR2();
			}
			Swapchain(true);
			// Post processing. Happens after the main scene has finished rendering and the image has been supplied to the swapchain.
			GFXHelper();
			ImGuiPass();
		}

		void Renderpass::Destroy()
		{
			RenderContext::Instance().GpuWaitForIdle();
			m_imgui_pass.Release();
			Graphics::RHI_FSR::Instance().Destroy();

			Runtime::WorldSystem::Instance().RemoveWorld(Runtime::WorldSystem::Instance().FindWorldByName("EditorWorld"));
		}

		void Renderpass::FrameSetup()
		{
			IS_PROFILE_FUNCTION();

			ImGui::Begin("Renderpass options:");
			ImGui::SliderInt("Mesh Lods", &MeshLod, 0, Runtime::Mesh::s_MAX_LOD_COUNT - 1);
			ImGui::Checkbox("Use Material Batching", &RenderMaterialBatching);
			ImGui::End();

			{
				IS_PROFILE_SCOPE("Add models to scene");
				for (size_t i = 0; i < modelsToAddToScene.size(); ++i)
				{
					if (modelsToAddToScene.at(i).first
						&& !modelsToAddToScene.at(i).second
						&& modelsToAddToScene.at(i).first->GetAssetState() == Runtime::AssetState::Loaded)
					{
						modelsToAddToScene.at(i).second = true;
						for (size_t modelCreateIdx = 0; modelCreateIdx < 1; ++modelCreateIdx)
						{
							ECS::Entity* entity = modelsToAddToScene.at(i).first->CreateEntityHierarchy();

							Maths::Matrix4 transform = Maths::Matrix4::Identity;
							//transform[3] = Maths::Vector4(10.0f, 0.0f, 25.0f, 1.0f);
							entity->GetComponent<ECS::TransformComponent>()->SetTransform(transform);
						}

						IS_LOG_CORE_INFO("Model '{}' added to scene.", modelsToAddToScene.at(i).first->GetFileName());
					}
				}
			}

			{
				IS_PROFILE_SCOPE("Render Resolution");
				ImGui::Begin("Render");
				ImGui::DragInt2("Render Resolution", pendingRenderResolution);
				if (ImGui::Button("Apply Render Resolution"))
				{
					RenderGraph::Instance().SetRenderResolution(Maths::Vector2(pendingRenderResolution[0], pendingRenderResolution[1]));
				}
				ImGui::End();
			}

			renderFrame = App::Engine::Instance().GetSystemRegistry().GetSystem<Runtime::GraphicsSystem>()->GetRenderFrame();

			m_buffer_frame.Proj_View = renderFrame.MainCamera.Camera.GetProjectionViewMatrix();
			m_buffer_frame.Projection = renderFrame.MainCamera.Camera.GetProjectionMatrix();
			m_buffer_frame.View = renderFrame.MainCamera.Camera.GetViewMatrix();

			if (enableFSR)
			{
				Maths::Vector2 const renderResolution = RenderGraph::Instance().GetRenderResolution();
				RHI_FSR::Instance().GenerateJitterSample(&m_taaJitterX, &m_taaJitterY);
				m_taaJitterX = (m_taaJitterX / static_cast<float>(renderResolution.x));
				m_taaJitterY = (m_taaJitterY / static_cast<float>(renderResolution.y));

				Maths::Matrix4 translation = Maths::Matrix4::Identity.Translated(Maths::Vector4(m_taaJitterX, m_taaJitterY, 0.0f, 0.0f));
				Maths::Matrix4 trans = Maths::Matrix4::Identity.Translated(Maths::Vector3(m_taaJitterX, m_taaJitterY, 0.0f));
				m_buffer_frame.Projection = m_buffer_frame.Projection * translation;

				if (RenderContext::Instance().GetGraphicsAPI() == GraphicsAPI::Vulkan)
				{
					Maths::Matrix4 proj = m_buffer_frame.Projection;
					proj[1][1] *= -1;
					m_buffer_frame.Proj_View = proj * m_buffer_frame.View.Inversed();
				}
				else
				{
					m_buffer_frame.Proj_View = m_buffer_frame.Projection * m_buffer_frame.View.Inversed();
				}
			}

			m_buffer_frame.View_Inverted = renderFrame.MainCamera.Camera.GetInvertedViewMatrix();
			m_buffer_frame.Projection_View_Inverted = renderFrame.MainCamera.Camera.GetInvertedProjectionViewMatrix();

			m_buffer_frame.Render_Resolution[0] = RenderGraph::Instance().GetRenderResolution().x;
			m_buffer_frame.Render_Resolution[1] = RenderGraph::Instance().GetRenderResolution().y;

			m_buffer_frame.Ouput_Resolution[0] = RenderGraph::Instance().GetOutputResolution().x;
			m_buffer_frame.Ouput_Resolution[1] = RenderGraph::Instance().GetOutputResolution().y;

			BufferLight::GetCascades(m_directional_light, m_buffer_frame, 4, 0.95f);

			RenderGraph::Instance().AddPreRender([this](RenderGraph& render_graph, RHI_CommandList* cmd_list)
				{
					g_global_resources.Buffer_Frame_View = cmd_list->UploadUniform(m_buffer_frame);
					g_global_resources.Buffer_Directional_Light_View = cmd_list->UploadUniform(m_directional_light);
				});
			RenderGraph::Instance().AddPostRender([this](RenderGraph& render_graph, RHI_CommandList* cmd_list)
				{
					GFXHelper::Reset();
				});
		}

		void Renderpass::RenderMainPasses(bool render)
		{
			if (render)
			{
				ShadowPass();
				//ShadowCullingPass();
				if (Depth_Prepass)
				{
					//DepthPrepass();
				}
				GBuffer();
				TransparentGBuffer();
				Composite();
				FSR2();
			}
		}

		void Renderpass::RenderSwapchain(bool renderResultImage)
		{
			Swapchain(renderResultImage);
		}

		void Renderpass::RenderPostprocessing()
		{
			// Post processing. Happens after the main scene has finished rendering and the image has been supplied to the swapchain.
			GFXHelper();
			ImGuiPass();
		}

		Maths::Vector2 swapchainColour = Maths::Vector2(0,0);
		Maths::Vector2 swapchainColour2 = Maths::Vector2(0,0);

		Maths::Matrix4 MakeInfReversedZProjRH(float fovY_radians, float aspectWbyH, float zNear)
		{
			float f = 1.0f / tan(fovY_radians / 2.0f);
			return Maths::Matrix4(
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

			shadowCamera.Projection = Maths::Matrix4(
				w, 0.0f, 0.0f, 0.0f,
				0.0f, -h, 0.0f, 0.0f,
				0.0f, 0.0f, a, 1.0f,
				0.0f, 0.0f, b, 0.0f);*/

			Maths::Matrix4 reverseZProj = Maths::Matrix4::Identity;
			reverseZProj[2][2] = -1;
			reverseZProj[2][3] = 1;
			//shadowCamera.Projection = MakeInfReversedZProjRH(glm::radians(ShadowFOV), 1.0f, ShadowZNear);
			//shadowCamera.ProjView = shadowCamera.Projection * (inverse ? glm::inverse(shadowCamera.View) : shadowCamera.View);

			return shadowCamera;
		}

		void ImGuiMat4(Maths::Matrix4& matrix)
		{
			float x_row[4] = { matrix[0].x, matrix[0].y, matrix[0].z, matrix[0].w };
			float y_row[4] = { matrix[1].x, matrix[1].y, matrix[1].z, matrix[1].w };
			float z_row[4] = { matrix[2].x, matrix[2].y, matrix[2].z, matrix[2].w };
			float w_row[4] = { matrix[3].x, matrix[3].y, matrix[3].z, matrix[3].w };
			ImGui::DragFloat4("Camera X Row", x_row);
			ImGui::DragFloat4("Camera Y Row", y_row);
			ImGui::DragFloat4("Camera Z Row", z_row);
			ImGui::DragFloat4("Camera W Row", w_row);
			matrix[0] = Maths::Vector4(x_row[0], x_row[1], x_row[2], x_row[3]);
			matrix[1] = Maths::Vector4(y_row[0], y_row[1], y_row[2], y_row[3]);
			matrix[2] = Maths::Vector4(z_row[0], z_row[1], z_row[2], z_row[3]);
			matrix[3] = Maths::Vector4(w_row[0], w_row[1], w_row[2], w_row[3]);
		}

		void Renderpass::ShadowPass()
		{
			IS_PROFILE_FUNCTION();

			struct PassData
			{
				RGTextureHandle Depth_Tex;
				RenderFrame RenderFrame;
			};
			PassData data;
			{
				IS_PROFILE_SCOPE("pass data setup");
				data.Depth_Tex = -1;
				data.RenderFrame = renderFrame;
			}

			static float depth_constant_factor = 4.0f;
			static float depth_slope_factor = 1.5f;
			{
				IS_PROFILE_SCOPE("imgui drawing");
				ImGui::Begin("Directional Light Direction");
				float dir[3] = { dir_light_direction.x, dir_light_direction.y, dir_light_direction.z };
				if (ImGui::DragFloat3("Direction", dir, 0.001f, -1.0f, 1.0f))
				{
					dir_light_direction = Maths::Vector3(dir[0], dir[1], dir[2]);
				}

				ImGui::DragFloat("Dpeth bias constant factor", &depth_constant_factor, 0.01f);
				ImGui::DragFloat("Dpeth bias slope factor", &depth_slope_factor, 0.01f);
				ImGui::DragFloat("Cascade Split Lambda", &cascade_split_lambda, 0.001f, 0.0f, 1.0f);

				ImGui::End();
			}

			/// Look into "panking" for dir light https:///www.gamedev.net/forums/topic/639036-shadow-mapping-and-high-up-objects/
			RenderGraph::Instance().AddPass<PassData>("Cascade shadow pass",
				[](PassData& data, RenderGraphBuilder& builder)
				{
					IS_PROFILE_SCOPE("Cascade shadow pass setup");

					RHI_TextureInfo tex_create_info = RHI_TextureInfo::Tex2DArray(Shadow_Depth_Tex_Size, Shadow_Depth_Tex_Size,
						PixelFormat::D16_UNorm, ImageUsageFlagsBits::DepthStencilAttachment | ImageUsageFlagsBits::Sampled, 4);

					RGTextureHandle depth_tex = builder.CreateTexture("Cascade_Shadow_Tex", tex_create_info);
					builder.WriteDepthStencil(depth_tex);
					data.Depth_Tex = depth_tex;

					ShaderDesc shader_description("CascadeShaderMap", {}, ShaderStageFlagBits::ShaderStage_Vertex);
					shader_description.InputLayout = ShaderDesc::GetDefaultShaderInputLayout();
					builder.SetShader(shader_description);

					PipelineStateObject pso = { };
					pso.Name = "Cascade_Shadow_PSO";
					pso.ShaderDescription = shader_description;
					pso.CullMode = CullMode::Front;
					pso.FrontFace = FrontFace::CounterClockwise;
					pso.DepthClampEnabled = false;
					pso.DepthBaisEnabled = true;
					pso.DepthConstantBaisValue = RenderContext::Instance().IsRenderOptionsEnabled(RenderOptions::ReverseZ) ? -depth_constant_factor : depth_constant_factor;
					pso.DepthSlopeBaisValue = RenderContext::Instance().IsRenderOptionsEnabled(RenderOptions::ReverseZ) ? -depth_slope_factor : depth_slope_factor;
					if (Reverse_Z_For_Depth)
					{
						pso.DepthCompareOp = CompareOp::GreaterOrEqual;
					}
					else
					{
						pso.DepthCompareOp = CompareOp::LessOrEqual;
					}
					pso.Dynamic_States = { DynamicState::Viewport, DynamicState::Scissor };
					builder.SetPipeline(pso);

					builder.SetViewport(Shadow_Depth_Tex_Size, Shadow_Depth_Tex_Size);
					builder.SetScissor(Shadow_Depth_Tex_Size, Shadow_Depth_Tex_Size);
				},
				[this](PassData& data, RenderGraph& render_graph, RHI_CommandList* cmdList)
				{
					IS_PROFILE_SCOPE("Cascade shadow pass execute");

					PipelineStateObject pso = render_graph.GetPipelineStateObject("Cascade shadow pass");
					cmdList->BindPipeline(pso, nullptr);

					cmdList->SetUniform(1, 0, g_global_resources.Buffer_Directional_Light_View);

					RHI_Texture* depth_tex = render_graph.GetRHITexture(data.Depth_Tex);
					for (u32 i = 0; i < depth_tex->GetInfo().Layer_Count; ++i)
					{
						IS_PROFILE_SCOPE("Slice");
						Graphics::Frustum camera_frustum(m_directional_light.View[i], m_directional_light.Projection[i], ShadowZFar);

						RenderpassDescription renderpass_description = render_graph.GetRenderpassDescription("Cascade shadow pass");
						renderpass_description.DepthStencilAttachment.Layer_Array_Index = static_cast<u32>(i);
						cmdList->BeginRenderpass(renderpass_description);

						const float CasacdeMinRaius[s_Cascade_Count] = { 0.0f, 2.5f, 5.0f, 8.5f };
						for (RenderWorld const& world : data.RenderFrame.RenderWorlds)
						{
							for (const u64 meshIndex : world.OpaqueMeshIndexs)
							{
								const RenderMesh& mesh = world.Meshes.at(meshIndex);
								if (mesh.BoudingBox.GetRadius() < CasacdeMinRaius[i])
								{
									continue;
								}

								struct alignas(16) Object
								{
									Maths::Matrix4 Transform;
									int CascadeIndex;
								};
								Object object =
								{
									mesh.Transform,
									static_cast<int>(i)
								};
								cmdList->SetUniform(2, 1, object);

								const Runtime::MeshLOD& renderMeshLod = mesh.GetLOD(MeshLod);
#ifdef VERTEX_SPLIT_STREAMS
								const Graphics::RHI_BufferView vertexBuffers[] =
								{
									renderMeshLod.VertexBuffers.PositionView,
									renderMeshLod.VertexBuffers.BoneIdsView,
									renderMeshLod.VertexBuffers.BoneWeightsView,
								};
								cmdList->SetVertexBuffer(vertexBuffers, ARRAY_COUNT(vertexBuffers));
#else
								cmdList->SetVertexBuffer(renderMeshLod.VertexBufferView);
#endif
								cmdList->SetIndexBuffer(renderMeshLod.IndexBufferView, Graphics::IndexType::Uint32);
								cmdList->DrawIndexed(renderMeshLod.Index_count, 1, renderMeshLod.First_index, renderMeshLod.Vertex_offset, 0);
								++RenderStats::Instance().MeshCount;
							}
						}
						cmdList->EndRenderpass();
					}
				}, std::move(data));
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

			RenderGraph::Instance().AddPass<PassData>("Depth_Prepass", 
				[](PassData& data, RenderGraphBuilder& builder)
				{
					IS_PROFILE_SCOPE("Depth_Prepass pass setup");

					RHI_TextureInfo textureCreateInfo = RHI_TextureInfo::Tex2D(
						  builder.GetRenderResolution().x
						, builder.GetRenderResolution().y
						, PixelFormat::D32_Float
						, ImageUsageFlagsBits::DepthStencilAttachment | ImageUsageFlagsBits::Sampled);
					RGTextureHandle depthStencil = builder.CreateTexture("Depth_Prepass_DepthStencil", textureCreateInfo);
					builder.WriteDepthStencil(depthStencil);

					ShaderDesc shaderDesc("DepthPrepass", {}, ShaderStageFlagBits::ShaderStage_Vertex);
					builder.SetShader(shaderDesc);

					PipelineStateObject depth_Prepass_pso = { };
					{
						IS_PROFILE_SCOPE("Depth_Prepass_SetPipelineStateObject");
						depth_Prepass_pso.Name = "Depth_Prepass_PSO";
						depth_Prepass_pso.CullMode = CullMode::Front;
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

					PipelineStateObject pso = render_graph.GetPipelineStateObject("Depth_Prepass");
					cmdList->BindPipeline(pso, nullptr);
					cmdList->BeginRenderpass(render_graph.GetRenderpassDescription("Depth_Prepass"));

					{
						IS_PROFILE_SCOPE("Depth_Prepass-SetUniform");
						cmdList->SetUniform(0, 0, g_global_resources.Buffer_Frame_View);
					}

					Frustum camera_frustum(data.Buffer_Frame.View, data.Buffer_Frame.Projection, 1000.0f);

					/*for (const Ptr<ECS::Entity> e : opaque_entities_to_render)
					{
						ECS::MeshComponent* mesh_component = static_cast<ECS::MeshComponent*>(e->GetComponentByName(ECS::MeshComponent::Type_Name));
						if (!mesh_component
							|| !mesh_component->GetMesh())
						{
							continue;
						}

						ECS::TransformComponent* transform_component = static_cast<ECS::TransformComponent*>(e->GetComponentByName(ECS::TransformComponent::Type_Name));
						Maths::Matrix4 transform = transform_component->GetTransform();

						BufferPerObject object = {};
						object.Transform = transform;
						object.Previous_Transform = transform;
						cmdList->SetUniform(1, 1, object);

						mesh_component->GetMesh()->Draw(cmdList);
					}*/

					cmdList->EndRenderpass();
				}, std::move(pass_data));
		}

		void Renderpass::GBuffer()
		{
			IS_PROFILE_FUNCTION();

			struct TestPassData
			{
				RenderFrame RenderFrame;
				BufferFrame Buffer_Frame = { };
				BufferSamplers Buffer_Samplers = { };
			};
			TestPassData Pass_Data;
			{
				IS_PROFILE_SCOPE("RenderFrame copy");
				Pass_Data.RenderFrame = renderFrame;
			}
			Pass_Data.Buffer_Frame = m_buffer_frame;
			Pass_Data.Buffer_Samplers = m_buffer_samplers;

			RenderGraph::Instance().AddPass<TestPassData>("GBuffer", 
				[](TestPassData& data, RenderGraphBuilder& builder)
				{
					IS_PROFILE_SCOPE("GBuffer pass setup");

					RHI_TextureInfo textureCreateInfo = RHI_TextureInfo::Tex2D(
						  builder.GetRenderResolution().x
						, builder.GetRenderResolution().y
						, PixelFormat::R8G8B8A8_UNorm
						, ImageUsageFlagsBits::ColourAttachment | ImageUsageFlagsBits::Sampled);
					RGTextureHandle colourRT = builder.CreateTexture("ColourRT", textureCreateInfo);
					builder.WriteTexture(colourRT);

					textureCreateInfo = RHI_TextureInfo::Tex2D(
						  builder.GetRenderResolution().x
						, builder.GetRenderResolution().y
						, PixelFormat::R16G16B16A16_Float
						, ImageUsageFlagsBits::ColourAttachment | ImageUsageFlagsBits::Sampled);
					RGTextureHandle normal_rt = builder.CreateTexture("NormalRT", textureCreateInfo);
					builder.WriteTexture(normal_rt);

					textureCreateInfo = RHI_TextureInfo::Tex2D(
						  builder.GetRenderResolution().x
						, builder.GetRenderResolution().y
						, PixelFormat::R16G16_Float
						, ImageUsageFlagsBits::ColourAttachment | ImageUsageFlagsBits::Sampled);
					RGTextureHandle velocity_rt = builder.CreateTexture("VelocityRT", textureCreateInfo);
					builder.WriteTexture(velocity_rt);

					textureCreateInfo.Format = PixelFormat::D32_Float;
					textureCreateInfo.ImageUsage = ImageUsageFlagsBits::DepthStencilAttachment | ImageUsageFlagsBits::Sampled;
					if (Depth_Prepass)
					{
						RGTextureHandle depth_prepass = builder.GetTexture("Depth_Prepass_DepthStencil");
						builder.WriteDepthStencil(depth_prepass);
					}
					else
					{
						RGTextureHandle depthStencil = builder.CreateTexture("GBuffer_DepthStencil", textureCreateInfo);
						builder.WriteDepthStencil(depthStencil);
					}
					builder.ReadTexture(builder.GetTexture("Cascade_Shadow_Tex"));

					ShaderDesc shaderDesc("GBuffer", {}, ShaderStageFlagBits::ShaderStage_Vertex | ShaderStageFlagBits::ShaderStage_Pixel);
					shaderDesc.InputLayout = ShaderDesc::GetDefaultShaderInputLayout();
					builder.SetShader(shaderDesc);

					PipelineStateObject gbufferPso = { };
					{
						IS_PROFILE_SCOPE("GBuffer-SetPipelineStateObject");
						gbufferPso.Name = "GBuffer_PSO";
						gbufferPso.CullMode = CullMode::Front;
						gbufferPso.FrontFace = FrontFace::CounterClockwise;
						gbufferPso.ShaderDescription = shaderDesc;
						gbufferPso.DepthCompareOp = CompareOp::LessOrEqual;

						if (Reverse_Z_For_Depth)
						{
							gbufferPso.DepthCompareOp = CompareOp::GreaterOrEqual;
						}
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

					builder.SetViewport(builder.GetRenderResolution().x, builder.GetRenderResolution().y);
					builder.SetScissor(builder.GetRenderResolution().x, builder.GetRenderResolution().y);
				},
				[this](TestPassData& data, RenderGraph& render_graph, RHI_CommandList* cmdList)
				{
					IS_PROFILE_SCOPE("GBuffer pass execute");

					PipelineStateObject pso = render_graph.GetPipelineStateObject("GBuffer");
					cmdList->BindPipeline(pso, nullptr);
					cmdList->BeginRenderpass(render_graph.GetRenderpassDescription("GBuffer"));

					{
						IS_PROFILE_SCOPE("Set Buffer Frame Uniform");
						BindCommonResources(cmdList, data.Buffer_Frame, data.Buffer_Samplers);
					}

					Frustum camera_frustum;
					if (UseCustomFrustum)
					{
						Frustum camera_frustum = MainCameraFrustum;
					}
					else
					{
						camera_frustum = Frustum(data.Buffer_Frame.View, data.Buffer_Frame.Projection, Main_Camera_Far_Plane);
					}

					for (const RenderWorld& world : data.RenderFrame.RenderWorlds)
					{
						if (RenderMaterialBatching)
						{
							for (const RenderMaterailBatch materialBatch : world.MaterialBatch)
							{
								BufferPerObject object = {};
								{
									IS_PROFILE_SCOPE("Set textures");

									const RenderMaterial& renderMaterial = materialBatch.Material;
									// Theses sets and bindings shouldn't chagne.
									RHI_Texture* diffuseTexture = renderMaterial.Textures[(u64)Runtime::TextureAssetTypes::Diffuse];
									if (diffuseTexture)
									{
										cmdList->SetTexture(3, 0, diffuseTexture);
										object.Textures_Set[0] = 1;
									}
								}

								for (const u64 meshIndex : materialBatch.OpaqueMeshIndex)
								{
									IS_PROFILE_SCOPE("Draw Entity");
									const RenderMesh& mesh = world.Meshes.at(meshIndex);

									object.Transform = mesh.Transform;
									object.Previous_Transform = mesh.Transform;
									cmdList->SetUniform(2, 0, object);

									const Runtime::MeshLOD& renderMeshLod = mesh.GetLOD(MeshLod);
#ifdef VERTEX_SPLIT_STREAMS
									const Graphics::RHI_BufferView vertexBuffers[] =
									{
										renderMeshLod.VertexBuffers.PositionView,
										renderMeshLod.VertexBuffers.NormalView,
										renderMeshLod.VertexBuffers.ColourView,
										renderMeshLod.VertexBuffers.UVView,
										renderMeshLod.VertexBuffers.BoneIdsView,
										renderMeshLod.VertexBuffers.BoneWeightsView,
									};
									cmdList->SetVertexBuffer(vertexBuffers, ARRAY_COUNT(vertexBuffers));
#else
									cmdList->SetVertexBuffer(renderMeshLod.VertexBufferView);
#endif
									cmdList->SetIndexBuffer(renderMeshLod.IndexBufferView, Graphics::IndexType::Uint32);
									cmdList->DrawIndexed(renderMeshLod.Index_count, 1, renderMeshLod.First_index, renderMeshLod.Vertex_offset, 0);
									++RenderStats::Instance().MeshCount;
								}
							}
						}
						else
						{
							for (const u64 meshIndex : world.OpaqueMeshIndexs)
							{
								IS_PROFILE_SCOPE("Draw Entity");
								const RenderMesh& mesh = world.Meshes.at(meshIndex);

								BufferPerObject object = {};
								object.Transform = mesh.Transform;
								object.Previous_Transform = mesh.Transform;

								{
									IS_PROFILE_SCOPE("Set textures");

									const RenderMaterial& renderMaterial = mesh.Material;
									// Theses sets and bindings shouldn't chagne.
									RHI_Texture* diffuseTexture = renderMaterial.Textures[(u64)Runtime::TextureAssetTypes::Diffuse];
									if (diffuseTexture)
									{
										cmdList->SetTexture(3, 0, diffuseTexture);
										object.Textures_Set[0] = 1;
									}
								}

								cmdList->SetUniform(2, 0, object);

								const Runtime::MeshLOD& renderMeshLod = mesh.GetLOD(MeshLod);
#ifdef VERTEX_SPLIT_STREAMS
								const Graphics::RHI_BufferView vertexBuffers[] =
								{
									renderMeshLod.VertexBuffers.PositionView,
									renderMeshLod.VertexBuffers.NormalView,
									renderMeshLod.VertexBuffers.ColourView,
									renderMeshLod.VertexBuffers.UVView,
									renderMeshLod.VertexBuffers.BoneIdsView,
									renderMeshLod.VertexBuffers.BoneWeightsView,
								};
								cmdList->SetVertexBuffer(vertexBuffers, ARRAY_COUNT(vertexBuffers));
#else
								cmdList->SetVertexBuffer(renderMeshLod.VertexBufferView);
#endif
								cmdList->SetIndexBuffer(renderMeshLod.IndexBufferView, Graphics::IndexType::Uint32);
								cmdList->DrawIndexed(renderMeshLod.Index_count, 1, renderMeshLod.First_index, renderMeshLod.Vertex_offset, 0);
								++RenderStats::Instance().MeshCount;
							}
						}
					}
					
					cmdList->EndRenderpass();
				}, std::move(Pass_Data));
		}

		void Renderpass::TransparentGBuffer()
		{
			IS_PROFILE_FUNCTION();

			struct TestPassData
			{
				RenderFrame RenderFrame;
				BufferFrame Buffer_Frame = { };
				BufferSamplers Buffer_Samplers = { };
			};
			TestPassData Pass_Data;
			{
				IS_PROFILE_SCOPE("RenderFrame copy");
				Pass_Data.RenderFrame = renderFrame;
			}
			Pass_Data.Buffer_Frame = m_buffer_frame;
			Pass_Data.Buffer_Samplers = m_buffer_samplers;

			RenderGraph::Instance().AddPass<TestPassData>("Transparent_GBuffer",
				[](TestPassData& data, RenderGraphBuilder& builder)
				{
					IS_PROFILE_SCOPE("Transparent_GBuffer pass setup");

					RGTextureHandle colourRT = builder.GetTexture("ColourRT");
					builder.WriteTexture(colourRT);
					RGTextureHandle normal_rt = builder.GetTexture("NormalRT");
					builder.WriteTexture(normal_rt);
					RGTextureHandle velocity_rt = builder.GetTexture("VelocityRT");
					builder.WriteTexture(velocity_rt);

					RGTextureHandle depth = {};
					if (Depth_Prepass)
					{
						depth = builder.GetTexture("Depth_Prepass_DepthStencil");
						builder.WriteDepthStencil(depth);
					}
					else
					{
						depth = builder.GetTexture("GBuffer_DepthStencil");
						builder.WriteDepthStencil(depth);
					}
					builder.ReadTexture(builder.GetTexture("Cascade_Shadow_Tex"));

					RenderpassDescription renderpassDescription = {};
					renderpassDescription.AddAttachment(AttachmentDescription::Load(builder.GetRHITexture(colourRT)->GetFormat(), ImageLayout::ColourAttachment));
					renderpassDescription.AddAttachment(AttachmentDescription::Load(builder.GetRHITexture(normal_rt)->GetFormat(), ImageLayout::ColourAttachment));
					renderpassDescription.AddAttachment(AttachmentDescription::Load(builder.GetRHITexture(velocity_rt)->GetFormat(), ImageLayout::ColourAttachment));
					for (AttachmentDescription& attachment : renderpassDescription.Attachments)
					{
						attachment.InitalLayout = ImageLayout::ColourAttachment;
					}

					renderpassDescription.DepthStencilAttachment = AttachmentDescription::Load(builder.GetRHITexture(depth)->GetFormat(), ImageLayout::DepthStencilAttachment);
					renderpassDescription.DepthStencilAttachment.InitalLayout = ImageLayout::DepthStencilAttachment;
					builder.SetRenderpass(renderpassDescription);

					ShaderDesc shaderDesc("GBuffer", {}, ShaderStageFlagBits::ShaderStage_Vertex | ShaderStageFlagBits::ShaderStage_Pixel);
					shaderDesc.InputLayout = ShaderDesc::GetDefaultShaderInputLayout();
					builder.SetShader(shaderDesc);

					PipelineStateObject pso = { };
					{
						IS_PROFILE_SCOPE("SetPipelineStateObject");
						pso.ShaderDescription = shaderDesc;
						pso.Name = "Transparent_GBuffer";
						pso.CullMode = CullMode::Front;
						pso.FrontFace = FrontFace::CounterClockwise;
						pso.BlendEnable = true;
						pso.SrcColourBlendFactor = BlendFactor::SrcAlpha;
						pso.DstColourBlendFactor = BlendFactor::OneMinusSrcAlpha;
						pso.ColourBlendOp = BlendOp::Add;
						pso.SrcAplhaBlendFactor = BlendFactor::One;
						pso.DstAplhaBlendFactor = BlendFactor::One;
						pso.AplhaBlendOp = BlendOp::Add;

						if (Reverse_Z_For_Depth)
						{
							pso.DepthCompareOp = CompareOp::GreaterOrEqual;
						}
						if (Depth_Prepass)
						{
							pso.DepthWrite = false;
						}
					}
					builder.SetPipeline(pso);

					builder.SetViewport(builder.GetRenderResolution().x, builder.GetRenderResolution().y);
					builder.SetScissor(builder.GetRenderResolution().x, builder.GetRenderResolution().y);
				},
				[this](TestPassData& data, RenderGraph& render_graph, RHI_CommandList* cmdList)
				{
					IS_PROFILE_SCOPE("Transparent_GBuffer pass execute");

					PipelineStateObject pso = render_graph.GetPipelineStateObject("Transparent_GBuffer");
					cmdList->BindPipeline(pso, nullptr);
					cmdList->BeginRenderpass(render_graph.GetRenderpassDescription("Transparent_GBuffer"));

					{
						IS_PROFILE_SCOPE("Set Buffer Frame Uniform");
						BindCommonResources(cmdList, data.Buffer_Frame, data.Buffer_Samplers);
					}

					Frustum camera_frustum(data.Buffer_Frame.View, data.Buffer_Frame.Projection, Main_Camera_Far_Plane);

					for (const RenderWorld& world : data.RenderFrame.RenderWorlds)
					{
						if (RenderMaterialBatching)
						{
							for (const RenderMaterailBatch materialBatch : world.MaterialBatch)
							{
								BufferPerObject object = {};
								{
									IS_PROFILE_SCOPE("Set textures");

									const RenderMaterial& renderMaterial = materialBatch.Material;
									// Theses sets and bindings shouldn't chagne.
									RHI_Texture* diffuseTexture = renderMaterial.Textures[(u64)Runtime::TextureAssetTypes::Diffuse];
									if (diffuseTexture)
									{
										cmdList->SetTexture(3, 0, diffuseTexture);
										object.Textures_Set[0] = 1;
									}
								}

								for (const u64 meshIndex : materialBatch.TransparentMeshIndex)
								{
									IS_PROFILE_SCOPE("Draw Entity");
									const RenderMesh& mesh = world.Meshes.at(meshIndex);

									object.Transform = mesh.Transform;
									object.Previous_Transform = mesh.Transform;
									cmdList->SetUniform(2, 0, object);

									const Runtime::MeshLOD& renderMeshLod = mesh.GetLOD(MeshLod);
#ifdef VERTEX_SPLIT_STREAMS
									const Graphics::RHI_BufferView vertexBuffers[] =
									{
										renderMeshLod.VertexBuffers.PositionView,
										renderMeshLod.VertexBuffers.NormalView,
										renderMeshLod.VertexBuffers.ColourView,
										renderMeshLod.VertexBuffers.UVView,
										renderMeshLod.VertexBuffers.BoneIdsView,
										renderMeshLod.VertexBuffers.BoneWeightsView,
									};
									cmdList->SetVertexBuffer(vertexBuffers, ARRAY_COUNT(vertexBuffers));
#else
									cmdList->SetVertexBuffer(renderMeshLod.VertexBufferView);
#endif
									cmdList->SetIndexBuffer(renderMeshLod.IndexBufferView, Graphics::IndexType::Uint32);
									cmdList->DrawIndexed(renderMeshLod.Index_count, 1, renderMeshLod.First_index, renderMeshLod.Vertex_offset, 0);
									++RenderStats::Instance().MeshCount;
								}
							}
						}
						else
						{
							for (const u64 meshIndex : world.TransparentMeshIndexs)
							{
								IS_PROFILE_SCOPE("Draw Entity");
								const RenderMesh& mesh = world.Meshes.at(meshIndex);

								BufferPerObject object = {};
								object.Transform = mesh.Transform;
								object.Previous_Transform = mesh.Transform;

								const RenderMaterial& renderMaterial = mesh.Material;
								// Theses sets and bindings shouldn't chagne.
								RHI_Texture* diffuseTexture = renderMaterial.Textures[(u64)Runtime::TextureAssetTypes::Diffuse];
								if (diffuseTexture)
								{
									cmdList->SetTexture(3, 0, diffuseTexture);
									object.Textures_Set[0] = 1;
								}

								cmdList->SetUniform(2, 0, object);

								const Runtime::MeshLOD& renderMeshLod = mesh.GetLOD(MeshLod);
#ifdef VERTEX_SPLIT_STREAMS
								const Graphics::RHI_BufferView vertexBuffers[] =
								{
									renderMeshLod.VertexBuffers.PositionView,
									renderMeshLod.VertexBuffers.NormalView,
									renderMeshLod.VertexBuffers.ColourView,
									renderMeshLod.VertexBuffers.UVView,
									renderMeshLod.VertexBuffers.BoneIdsView,
									renderMeshLod.VertexBuffers.BoneWeightsView,
								};
								cmdList->SetVertexBuffer(vertexBuffers, ARRAY_COUNT(vertexBuffers));
#else
								cmdList->SetVertexBuffer(renderMeshLod.VertexBufferView);
#endif
								cmdList->SetIndexBuffer(renderMeshLod.IndexBufferView, Graphics::IndexType::Uint32);
								cmdList->DrawIndexed(renderMeshLod.Index_count, 1, renderMeshLod.First_index, renderMeshLod.Vertex_offset, 0);
								++RenderStats::Instance().MeshCount;
							}
						}
					}
					

					cmdList->EndRenderpass();
				}, std::move(Pass_Data));
		}

		void Renderpass::Composite()
		{
			IS_PROFILE_FUNCTION();
			struct PassData
			{
				BufferFrame Buffer_Frame;
				BufferSamplers Buffer_Samplers;
			};
			PassData pass_data = {};
			pass_data.Buffer_Frame = m_buffer_frame;
			pass_data.Buffer_Samplers = m_buffer_samplers;

			RenderGraph::Instance().AddPass<PassData>("Composite_Pass",
				[](PassData& data, RenderGraphBuilder& builder)
				{
					builder.ReadTexture(builder.GetTexture("ColourRT"));
					builder.ReadTexture(builder.GetTexture("NormalRT"));
					if (Depth_Prepass)
					{
						builder.ReadTexture(builder.GetTexture("Depth_Prepass_DepthStencil"));
					}
					else
					{
						builder.ReadTexture(builder.GetTexture("GBuffer_DepthStencil"));
					}
					builder.ReadTexture(builder.GetTexture("Cascade_Shadow_Tex"));

					RHI_TextureInfo create_info = RHI_TextureInfo::Tex2D(
						  builder.GetRenderResolution().x
						, builder.GetRenderResolution().y
						, PixelFormat::R8G8B8A8_UNorm
						, ImageUsageFlagsBits::ColourAttachment | ImageUsageFlagsBits::Sampled | ImageUsageFlagsBits::Storage);
					RGTextureHandle composite_handle = builder.CreateTexture("Composite_Tex", create_info);
					builder.WriteTexture(composite_handle);

					ShaderDesc shader_description("Composite", {}, ShaderStageFlagBits::ShaderStage_Vertex | ShaderStageFlagBits::ShaderStage_Pixel);
					builder.SetShader(shader_description);

					PipelineStateObject pso = { };
					pso.Name = "Composite_PSO";
					pso.ShaderDescription = shader_description;
					pso.DepthTest = false;
					pso.PolygonMode = PolygonMode::Fill;
					pso.CullMode = CullMode::Front;
					pso.FrontFace = FrontFace::CounterClockwise;
					builder.SetPipeline(pso);

					builder.SetViewport(builder.GetRenderResolution().x, builder.GetRenderResolution().y);
					builder.SetScissor(builder.GetRenderResolution().x, builder.GetRenderResolution().y);
				},
				[this](PassData& data, RenderGraph& render_graph, RHI_CommandList* cmd_list)
				{
					PipelineStateObject pso = render_graph.GetPipelineStateObject("Composite_Pass");
					cmd_list->BindPipeline(pso, nullptr);
					cmd_list->BeginRenderpass(render_graph.GetRenderpassDescription("Composite_Pass"));

					BindCommonResources(cmd_list, data.Buffer_Frame, data.Buffer_Samplers);
					cmd_list->SetUniform(1, 0, g_global_resources.Buffer_Directional_Light_View);

					const u8 texture_offset = 0;
					cmd_list->SetTexture(3, texture_offset, render_graph.GetRHITexture(render_graph.GetTexture("ColourRT")));
					cmd_list->SetTexture(3, texture_offset + 1, render_graph.GetRHITexture(render_graph.GetTexture("NormalRT")));
					if (Depth_Prepass)
					{
						cmd_list->SetTexture(1, 1, render_graph.GetRHITexture(render_graph.GetTexture("Depth_Prepass_DepthStencil")));
					}
					else
					{
						cmd_list->SetTexture(1, 1, render_graph.GetRHITexture(render_graph.GetTexture("GBuffer_DepthStencil")));
					}
					cmd_list->SetTexture(1, 0, render_graph.GetRHITexture(render_graph.GetTexture("Cascade_Shadow_Tex")));

					cmd_list->Draw(3, 1, 0, 0);
					cmd_list->EndRenderpass();
				}, std::move(pass_data));
		}

		void Renderpass::FSR2()
		{
			IS_PROFILE_FUNCTION();

			if (RenderGraph::Instance().GetRenderResolution() == RenderGraph::Instance().GetOutputResolution())
			{
				return;
			}

			bool reset = false;
			if (ImGui::Checkbox("Enable FSR", &enableFSR))
			{
				reset = true;
			}
			ImGui::DragFloat("FSR sharpness", &fsrSharpness, 0.05f, 0.0f, 1.0f);

			struct PassData
			{
				BufferFrame BufferFrame;
				float NearPlane;
				float FarPlane;
				float FOVY;
			};
			PassData passData = {};
			passData.BufferFrame = m_buffer_frame;

			passData.NearPlane = renderFrame.MainCamera.Camera.GetNearPlane();
			passData.FarPlane = renderFrame.MainCamera.Camera.GetFarPlane();
			passData.FOVY = renderFrame.MainCamera.Camera.GetFovY();

			RenderGraph::Instance().AddPass<PassData>("FSR2",
				[](PassData& data, RenderGraphBuilder& builder)
				{
					RHI_TextureInfo create_info = RHI_TextureInfo::Tex2D(
						builder.GetOutputResolution().x
						, builder.GetOutputResolution().y
						, PixelFormat::R8G8B8A8_UNorm
						, ImageUsageFlagsBits::Sampled | ImageUsageFlagsBits::Storage);
					RGTextureHandle textureHandle = builder.CreateTexture("FSR_Output", create_info);

					builder.SetViewport(RenderGraph::Instance().GetOutputResolution().x, RenderGraph::Instance().GetOutputResolution().y);
					builder.SetScissor(RenderGraph::Instance().GetOutputResolution().x, RenderGraph::Instance().GetOutputResolution().y);

					builder.SkipTextureWriteBarriers();
				},
				[this, reset](PassData& data, RenderGraph& render_graph, RHI_CommandList* cmd_list)
				{
					RHI_FSR::Instance().Dispatch(cmd_list
						, render_graph.GetRHITexture("Composite_Tex")
						, render_graph.GetRHITexture("GBuffer_DepthStencil")
						, render_graph.GetRHITexture("VelocityRT")
						, render_graph.GetRHITexture("FSR_Output")
						, data.NearPlane
						, data.FarPlane
						, data.FOVY
						, App::Engine::s_FrameTimer.GetElapsedTimeMillFloat()
						, fsrSharpness
						, reset);
				}, std::move(passData));

		}

		void Renderpass::GFXHelper()
		{
			IS_PROFILE_FUNCTION();

			struct TestPassData
			{
				BufferFrame Buffer_Frame;
				BufferSamplers Buffer_Samplers;
			};
			TestPassData passData = {};
			passData.Buffer_Frame = m_buffer_frame;
			passData.Buffer_Samplers = m_buffer_samplers;

			RenderGraph::Instance().AddPass<TestPassData>("GFXHelperPass", [this](TestPassData& data, RenderGraphBuilder& builder)
				{
					IS_PROFILE_SCOPE("GFXHelper pass setup");

					builder.SetViewport(builder.GetOutputResolution().x, builder.GetOutputResolution().y);
					builder.SetScissor(builder.GetOutputResolution().x, builder.GetOutputResolution().y);
					builder.SetAsRenderToSwapchain();

					builder.WriteTexture(-1);

					ShaderDesc shaderDesc("GFXHelper", {}, ShaderStageFlagBits::ShaderStage_Vertex | ShaderStageFlagBits::ShaderStage_Pixel);
					builder.SetShader(shaderDesc);

					PipelineStateObject pso = { };
					pso.Name = "GFXHelper_PSO";
					pso.ShaderDescription = shaderDesc;

					pso.PrimitiveTopologyType = PrimitiveTopologyType::LineList;
					pso.PolygonMode = PolygonMode::Line;
					pso.CullMode = CullMode::Front;
					pso.FrontFace = FrontFace::CounterClockwise;
					pso.Dynamic_States.push_back(DynamicState::LineWidth);

					builder.SetPipeline(pso);

					RenderpassDescription renderpassDescription = { };
					renderpassDescription.AddAttachment(AttachmentDescription::Load(PixelFormat::Unknown, Graphics::ImageLayout::PresentSrc));
					renderpassDescription.Attachments.back().InitalLayout = ImageLayout::ColourAttachment;
					builder.SetRenderpass(renderpassDescription);
				},
				[&](TestPassData& data, RenderGraph& renderGraph, RHI_CommandList* cmdList)
				{
					IS_PROFILE_SCOPE("GFXHelper pass execute");

					if (GFXHelper::m_lines.size() == 0)
					{
						return;
					}

					PipelineStateObject pso = renderGraph.GetPipelineStateObject("GFXHelperPass");
					cmdList->BindPipeline(pso, nullptr);
					cmdList->BeginRenderpass(renderGraph.GetRenderpassDescription("GFXHelperPass"));

					{
						IS_PROFILE_SCOPE("Set Buffer Frame Uniform");
						BindCommonResources(cmdList, data.Buffer_Frame, data.Buffer_Samplers);
					}

					struct Line
					{
						Line(Maths::Vector3 pos, Maths::Vector4 color)
							: Pos(Maths::Vector4(pos, 1.0f)), Color(color)
						{}
						Maths::Vector4 Pos;
						Maths::Vector4 Color;
					};

					struct GFXHelperDrawCall
					{
						int VertexCount;
						int InstanceCount;
						int FirstVertex;
						int FirstInstance;
					};

					int drawCallVertexStart = 0;
					std::vector<Line> gizmoLineData;
					std::vector<GFXHelperDrawCall> gizmoDrawCall;
					gizmoDrawCall.reserve(GFXHelper::m_lines.size());

					{
						IS_PROFILE_SCOPE("[GFXHelper pass] Directions");
						for (const GFXHelperLine& line : GFXHelper::m_lines)
						{
							std::array<Line, 2> lines
							{
								Line(line.Start, line.Colour),
								Line(line.End, line.Colour)
							};

							gizmoLineData.insert(gizmoLineData.end(), lines.begin(), lines.end());
							gizmoDrawCall.push_back({ 2, 1, drawCallVertexStart, 0 });
							drawCallVertexStart += 2;
						}
					}


					static RHI_Buffer* vBuffer = nullptr;
					if (!vBuffer)
					{
						vBuffer = Renderer::CreateVertexBuffer(sizeof(Line)* gizmoLineData.size(), sizeof(Line));
					}

					if (vBuffer->GetSize() < sizeof(Line) * gizmoLineData.size())
					{
						vBuffer->Resize(sizeof(Line)* gizmoLineData.size());
					}

					{
						IS_PROFILE_SCOPE("[Gizmos] Upload vertex buffer");

						vBuffer->Upload(gizmoLineData.data(), sizeof(Line) * gizmoLineData.size());
					}

					RHI_Buffer* vBuffers[] = { vBuffer };
					u32 offsets[] = { 0 };
					cmdList->SetVertexBuffer(vBuffer);
					cmdList->SetLineWidth(1.0f);
					{
						IS_PROFILE_SCOPE("[Gizmos] Draw");
						for (const GFXHelperDrawCall& dc : gizmoDrawCall)
						{
							cmdList->Draw(dc.VertexCount, dc.InstanceCount, dc.FirstVertex, dc.FirstInstance);
						}
					}
					cmdList->EndRenderpass();
				}, std::move(passData));
		}

		void Renderpass::Swapchain(bool renderResultImage)
		{
			IS_PROFILE_FUNCTION();

			struct TestPassData
			{
				RGTextureHandle RenderTarget;
			};

#ifdef RENDERGRAPH_V2_ENABLED
			RenderGraphV2::Instance().AddGraphicsPass("Swapchain",
				[](RenderGraphGraphicsPassV2& pass)
				{
					pass.SetAsRenderToSwapchain();

					ShaderDesc shaderDesc("Swapchain", {}, ShaderStageFlagBits::ShaderStage_Vertex | ShaderStageFlagBits::ShaderStage_Pixel);
					pass.SetShader(shaderDesc);

					PipelineStateObject swapchainPso = { };
					swapchainPso.Name = "Swapchain_PSO";
					swapchainPso.CullMode = CullMode::Front;
					swapchainPso.ShaderDescription = shaderDesc;
					pass.SetPipeline(swapchainPso);

					const Maths::Vector2 renderGraphOutputResolution = pass.RenderGraph->GetOutputResolution();
					pass.SetViewport(renderGraphOutputResolution.x, renderGraphOutputResolution.y);
					pass.SetScissor(renderGraphOutputResolution.x, renderGraphOutputResolution.y);
				},
				[&](const RenderGraphExecuteData& data)
				{
					IS_PROFILE_SCOPE("Swapchain pass execute");

					RenderGraphV2& renderGraph = data.RenderGraph;
					RHI_CommandList* cmdList = data.CmdList;

					PipelineStateObject pso = renderGraph.GetPipelineStateObject("SwapchainPass");
					cmdList->BindPipeline(pso, nullptr);
					cmdList->BeginRenderpass(renderGraph.GetRenderpassDescription("SwapchainPass"));

					const RGTextureHandle renderTarget = renderGraph.GetTexture("Composite_Texture");
					if (renderTarget != -1)
					{
						cmdList->SetTexture(0, 0, renderGraph.GetRHITexture(renderTarget));
						cmdList->SetSampler(1, 0, m_buffer_samplers.Clamp_Sampler);
					}
					else
					{
						cmdList->SetTexture(0, 0, nullptr);
						cmdList->SetSampler(1, 0, nullptr);
					}

					cmdList->Draw(3, 1, 0, 0);
					cmdList->EndRenderpass();
				},
				{ });
#endif

			RenderGraph::Instance().AddPass<TestPassData>("SwapchainPass", [](TestPassData& data, RenderGraphBuilder& builder)
				{
					IS_PROFILE_SCOPE("Swapchain pass setup");

					RGTextureHandle rt = 0;
					if (enableFSR)
					{
						rt = builder.GetTexture("FSR_Output");
						builder.SkipTextureReadBarriers();
					}
					else
					{
						rt = builder.GetTexture("Composite_Tex");
					}

					if (rt != -1)
					{
						//builder.ReadTexture(rt);
					}
					data.RenderTarget = rt;

					builder.WriteTexture(-1);

					ShaderDesc shaderDesc("Swapchain", {}, ShaderStageFlagBits::ShaderStage_Vertex | ShaderStageFlagBits::ShaderStage_Pixel);
					builder.SetShader(shaderDesc);

					PipelineStateObject swapchainPso = { };
					swapchainPso.Name = "Swapchain_PSO";
					swapchainPso.CullMode = CullMode::Front;
					swapchainPso.ShaderDescription = shaderDesc;
					builder.SetPipeline(swapchainPso);

					builder.SetViewport(builder.GetOutputResolution().x, builder.GetOutputResolution().y);
					builder.SetScissor(builder.GetOutputResolution().x, builder.GetOutputResolution().y);


					builder.SetAsRenderToSwapchain();
				},
				[this, renderResultImage](TestPassData& data, RenderGraph& renderGraph, RHI_CommandList* cmdList)
				{
					IS_PROFILE_SCOPE("Swapchain pass execute");

					PipelineStateObject pso = renderGraph.GetPipelineStateObject("SwapchainPass");
					cmdList->BindPipeline(pso, nullptr);
					cmdList->BeginRenderpass(renderGraph.GetRenderpassDescription("SwapchainPass"));

					if (data.RenderTarget != -1)
					{
						cmdList->SetTexture(0, 0, renderGraph.GetRHITexture(data.RenderTarget));
						cmdList->SetSampler(1, 0, m_buffer_samplers.Clamp_Sampler);
					}
					else
					{
						cmdList->SetTexture(0, 0, nullptr);
						cmdList->SetSampler(1, 0, nullptr);
					}

					if (renderResultImage)
					{
						cmdList->Draw(3, 1, 0, 0);
					}

					cmdList->EndRenderpass();
				});
		}

		void Renderpass::ImGuiPass()
		{
			IS_PROFILE_FUNCTION();

			m_imgui_pass.Render();
		}

		void Renderpass::CreateAllCommonShaders()
		{
			ShaderDesc shaderDesc("CascadeShaderMap", EnginePaths::GetResourcePath() + "/Shaders/hlsl/Cascade_Shadow.hlsl", ShaderStageFlagBits::ShaderStage_Vertex);
#ifdef VERTEX_SPLIT_STREAMS
			shaderDesc.InputLayout = ShaderDesc::GetShaderInputLayoutFromStreams(
				Graphics::Vertices::Stream::Position
				| Graphics::Vertices::Stream::BoneId
				| Graphics::Vertices::Stream::BoneWeight);
#else
			shaderDesc.InputLayout = ShaderDesc::GetDefaultShaderInputLayout();
#endif
			RenderContext::Instance().GetShaderManager().GetOrCreateShader(shaderDesc);

			shaderDesc = ShaderDesc("DepthPrepass", EnginePaths::GetResourcePath() + "/Shaders/hlsl/Depth_Prepass.hlsl", ShaderStageFlagBits::ShaderStage_Vertex);
#ifdef VERTEX_SPLIT_STREAMS
			shaderDesc.InputLayout = ShaderDesc::GetShaderInputLayoutFromStreams(
				Graphics::Vertices::Stream::Position
				| Graphics::Vertices::Stream::BoneId
				| Graphics::Vertices::Stream::BoneWeight);
#else
			shaderDesc.InputLayout = ShaderDesc::GetDefaultShaderInputLayout();
#endif
			//RenderContext::Instance().GetShaderManager().GetOrCreateShader(shaderDesc);

			shaderDesc = ShaderDesc("LightShadowPass", EnginePaths::GetResourcePath() + "/Shaders/hlsl/LightDepth.hlsl", ShaderStageFlagBits::ShaderStage_Vertex);
#ifdef VERTEX_SPLIT_STREAMS
			shaderDesc.InputLayout = ShaderDesc::GetShaderInputLayoutFromStreams(
				Graphics::Vertices::Stream::Position
				| Graphics::Vertices::Stream::BoneId
				| Graphics::Vertices::Stream::BoneWeight);
#else
			shaderDesc.InputLayout = ShaderDesc::GetDefaultShaderInputLayout();
#endif
			RenderContext::Instance().GetShaderManager().GetOrCreateShader(shaderDesc);

			shaderDesc = ShaderDesc("GBuffer", EnginePaths::GetResourcePath() + "/Shaders/hlsl/GBuffer.hlsl", ShaderStageFlagBits::ShaderStage_Vertex | ShaderStageFlagBits::ShaderStage_Pixel);
			shaderDesc.InputLayout = ShaderDesc::GetDefaultShaderInputLayout();
			RenderContext::Instance().GetShaderManager().GetOrCreateShader(shaderDesc);

			shaderDesc = ShaderDesc("Composite", EnginePaths::GetResourcePath() + "/Shaders/hlsl/Composite.hlsl", ShaderStageFlagBits::ShaderStage_Vertex | ShaderStageFlagBits::ShaderStage_Pixel);
			RenderContext::Instance().GetShaderManager().GetOrCreateShader(shaderDesc);

			shaderDesc = ShaderDesc("GFXHelper", EnginePaths::GetResourcePath() + "/Shaders/hlsl/GFXHelper.hlsl", ShaderStageFlagBits::ShaderStage_Vertex | ShaderStageFlagBits::ShaderStage_Pixel);
			RenderContext::Instance().GetShaderManager().GetOrCreateShader(shaderDesc);

			shaderDesc = ShaderDesc("Swapchain", EnginePaths::GetResourcePath() + "/Shaders/hlsl/Swapchain.hlsl", ShaderStageFlagBits::ShaderStage_Vertex | ShaderStageFlagBits::ShaderStage_Pixel);
			RenderContext::Instance().GetShaderManager().GetOrCreateShader(shaderDesc);

			shaderDesc = ShaderDesc("LightPass", EnginePaths::GetResourcePath() + "/Shaders/hlsl/LightPass.hlsl", ShaderStageFlagBits::ShaderStage_Vertex | ShaderStageFlagBits::ShaderStage_Pixel);
			RenderContext::Instance().GetShaderManager().GetOrCreateShader(shaderDesc);

			shaderDesc = ShaderDesc("LightPassCompute", EnginePaths::GetResourcePath() + "/Shaders/hlsl/LightPassCompute.hlsl", ShaderStageFlagBits::ShaderStage_Compute);
			RenderContext::Instance().GetShaderManager().GetOrCreateShader(shaderDesc);

			shaderDesc = ShaderDesc("ComputeSkinning", EnginePaths::GetResourcePath() + "/Shaders/hlsl/ComputeSkinning.hlsl", ShaderStageFlagBits::ShaderStage_Compute);
			RenderContext::Instance().GetShaderManager().GetOrCreateShader(shaderDesc);
		}

		void Renderpass::BindCommonResources(RHI_CommandList* cmd_list, BufferFrame& buffer_frame, BufferSamplers& buffer_samplers)
		{
			cmd_list->SetUniform(0, 0, g_global_resources.Buffer_Frame_View);

			cmd_list->SetSampler(4, 0, buffer_samplers.Shadow_Sampler);
			cmd_list->SetSampler(4, 1, buffer_samplers.Repeat_Sampler);
			cmd_list->SetSampler(4, 2, buffer_samplers.Clamp_Sampler);
			cmd_list->SetSampler(4, 3, buffer_samplers.MirroredRepeat_Sampler);
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
			const float ratio = maxZ / minZ > 0 ? minZ : 1;

			const float cascadeSplitLambda = split_lambda;

			/// Calculate split depths based on view camera frustum
			/// Based on method presented in https:///developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
			for (u32 i = 0; i < cascade_count; ++i)
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

				Maths::Vector3 frustumCorners[8] =
				{
					Maths::Vector3(-1.0f,  1.0f, -1.0f),
					Maths::Vector3(1.0f,  1.0f, -1.0f),
					Maths::Vector3(1.0f, -1.0f, -1.0f),
					Maths::Vector3(-1.0f, -1.0f, -1.0f),
					Maths::Vector3(-1.0f,  1.0f,  1.0f),
					Maths::Vector3(1.0f,  1.0f,  1.0f),
					Maths::Vector3(1.0f, -1.0f,  1.0f),
					Maths::Vector3(-1.0f, -1.0f,  1.0f),
				};

				/// Project frustum corners into world space
				Maths::Matrix4 invCam = buffer_frame.Proj_View.Inversed();
				for (u32 i = 0; i < 8; ++i)
				{
					Maths::Vector4 invCorner = invCam * Maths::Vector4(frustumCorners[i], 1.0f);
					frustumCorners[i] = Maths::Vector3((invCorner / invCorner.w));
				}

				for (u32 i = 0; i < 4; ++i)
				{
					Maths::Vector3 dist = frustumCorners[i + 4] - frustumCorners[i];
					frustumCorners[i + 4] = frustumCorners[i] + (dist * splitDist);
					frustumCorners[i] = frustumCorners[i] + (dist * lastSplitDist);
				}

				/// Get frustum center
				Maths::Vector3 frustumCenter = Maths::Vector3(0.0f);
				for (u32 i = 0; i < 8; ++i)
				{
					frustumCenter += frustumCorners[i];
				}
				frustumCenter /= 8.0f;

				float radius = 0.0f;
				for (u32 i = 0; i < 8; ++i)
				{
					float distance = (frustumCorners[i] - frustumCenter).Length();
					float glmDistance = Maths::Vector3Distance(Maths::Vector3(frustumCorners[i].x, frustumCorners[i].y, frustumCorners[i].z), Maths::Vector3(frustumCenter.x, frustumCenter.y, frustumCenter.z));
					radius = std::max(radius, distance);
				}
				radius = std::ceil(radius * 16.0f) / 16.0f;

				Maths::Vector3 maxExtents = Maths::Vector3(radius);
				Maths::Vector3 v;
				v = -v;
				Maths::Vector3 minExtents = -maxExtents;

				Maths::Matrix4 reverse_z = Maths::Matrix4::Identity;
				reverse_z[2][2] = -1;
				reverse_z[2][3] = 1;

				/// Construct our matrixs required for the light.
				Maths::Vector3 lightDirection(0.5f, -0.7f, 0.5f);
				Maths::Vector3 lightPosition = frustumCenter - lightDirection.Normalised() * -minExtents.z;
				Maths::Matrix4 lightViewMatrix = Maths::Matrix4::LookAt(lightPosition, frustumCenter, Maths::Vector3(0.0f, 1.0f, 0.0f));
				Maths::Matrix4 lightOrthoMatrix = Maths::Matrix4::CreateOrthographic(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f, maxExtents.z - minExtents.z);

				if (Reverse_Z_For_Depth)
				{
					Maths::Matrix4 proj = Maths::Matrix4::CreateOrthographic(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, maxExtents.z - minExtents.z, 0.0f);
					//glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, maxExtents.z - minExtents.z, 0.0f);
					lightOrthoMatrix = reverse_z * lightOrthoMatrix;
					lightOrthoMatrix = proj;
				}

				{
					if (RenderContext::Instance().GetGraphicsAPI() == GraphicsAPI::Vulkan)
					{
						/// Invert the projection if vulkan. This is because vulkan's coord space is from top left, not bottom left.
						lightOrthoMatrix[1][1] *= -1;
					}

					/// Store split distance and matrix in cascade
					outCascades.SplitDepth[i] = (Main_Camera_Near_Plane + splitDist * clipRange) * -1.0f;
					outCascades.ProjView[i] = lightOrthoMatrix * lightViewMatrix;
					outCascades.Projection[i] = lightOrthoMatrix;
					outCascades.View[i] = lightViewMatrix;
					outCascades.Light_Direction = Maths::Vector4((frustumCenter - lightPosition).Normalised(), 0.0);
				}
				lastSplitDist = cascadeSplits[i];
			}
			return outCascades;
		}
		
		void BufferLight::GetCascades(BufferLight& buffer_light, const BufferFrame& buffer_frame, u32 cascade_count, float split_lambda)
		{
			BufferLight light = GetCascades(buffer_frame, cascade_count, split_lambda);
			for (size_t i = 0; i < cascade_count; i++)
			{
				buffer_light.ProjView[i]   = light.ProjView[i];
				buffer_light.Projection[i] = light.Projection[i];
				buffer_light.View[i]       = light.View[i];
				buffer_light.SplitDepth[i] = light.SplitDepth[i];
			}
		}
	}
}