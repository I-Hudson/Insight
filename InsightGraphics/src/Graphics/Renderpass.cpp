#include "Graphics/Renderpass.h"
#include "Graphics/RenderContext.h"
#include "Graphics/Window.h"
#include "Graphics/GraphicsManager.h"

#include "Graphics/RenderTarget.h"

#include "Core/Profiler.h"

#include "Graphics/RenderGraph/RenderGraph.h"

#ifdef IS_VULKAN_ENABLED
#include "Graphics/RHI/Vulkan/RHI_CommandList_Vulkan.h"
#endif
#ifdef IS_DX12_ENABLED
#include "Graphics/RHI/DX12/RHI_CommandList_DX12.h"
#endif
#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_dx12.h>

#include <glm/gtx/matrix_interpolation.hpp>
#include <glm/ext/matrix_clip_space.hpp>

namespace Insight
{
	const float ShadowZNear = 1.0f;
	const float ShadowZFar = 512.0f * 4;
	const float ShadowFOV = 45.0f;

	namespace Graphics
	{
		float aspect = 0.0f;
		void Renderpass::Create()
		{
			m_testMesh.LoadFromFile("./Resources/models/sponza_old/sponza.obj");

			if (m_camera.View == glm::mat4(0.0f))
			{
				aspect = (float)Window::Instance().GetWidth() / (float)Window::Instance().GetHeight();
				m_camera.Projection = glm::perspective(glm::radians(90.0f), aspect, 0.1f, 5000.0f);
				m_camera.View = glm::mat4(1.0f);

				m_shadowCamera = { m_camera };
				m_shadowCamera.Projection = glm::perspective(glm::radians(ShadowFOV), aspect, ShadowZNear, ShadowZFar);
			}

#ifndef RENDER_GRAPH_ENABLED
			if (!m_vertexBuffer)
			{
				//ZoneScopedN("CreateVertexBuffer");

				Vertex vertices[3] =
				{
					Vertex( glm::vec4(-1.0f, 0.0f, 0.0f, 0.0f),	glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f) ),
					Vertex( glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),	glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f) ),
					Vertex( glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),	glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f) ),
				};
				m_vertexBuffer = Renderer::CreateVertexBuffer(sizeof(vertices), sizeof(Vertex));
				m_vertexBuffer->Upload(vertices, sizeof(vertices));
				m_vertexBuffer->SetName(L"TriangleMesh_Vertex_Buffer");
			}

			if (!m_indexBuffer)
			{
				int indices[3] = { 0, 1, 2, };
				m_indexBuffer = Renderer::CreateIndexBuffer(sizeof(int) * ARRAYSIZE(indices));
				m_indexBuffer->Upload(indices, sizeof(indices));
				m_indexBuffer->SetName(L"TriangleMesh_Index_Buffer");
			}

			if (!m_testTexture)
			{
				m_testTexture = Renderer::CreateTexture();
				m_testTexture->LoadFromFile("./Resources/testTexture.png");
				m_testTexture->SetName(L"TestTexture");
			}

			if (!m_shadowTarget)
			{
				m_shadowTarget = Renderer::CreateRenderTarget();
				int const oneK = 1024;
				int const textureSize = oneK * 8;
				Graphics::RenderTargetDesc desc = Graphics::RenderTargetDesc(textureSize, textureSize, PixelFormat::D32_Float, { 1, 0, 0, 1 });
				//Graphics::RenderTargetDesc desc = Graphics::RenderTargetDesc(Window::Instance().GetWidth(), Window::Instance().GetHeight(), PixelFormat::D32_Float, { 1, 0, 0, 1 });
				m_shadowTarget->Create("ShadowPassDepth", desc);
				m_shadowTarget->GetTexture()->SetName(L"ShadowPassDepth");
			}

			if (!m_colourTarget)
			{
				m_colourTarget = Renderer::CreateRenderTarget();
				Graphics::RenderTargetDesc desc = Graphics::RenderTargetDesc(Window::Instance().GetWidth(), Window::Instance().GetHeight(), PixelFormat::B8G8R8A8_UNorm, { 1, 0, 0, 1 });
				m_colourTarget->Create("StandardColour", desc);
				m_colourTarget->GetTexture()->SetName(L"GBufferColour");
			}

			if (!m_depthTarget)
			{
				m_depthTarget = Renderer::CreateRenderTarget();
				Graphics::RenderTargetDesc desc = Graphics::RenderTargetDesc(Window::Instance().GetWidth(), Window::Instance().GetHeight(), PixelFormat::D32_Float, { 1, 0, 0, 1 });
				m_depthTarget->Create("StandardDepth", desc);
				m_depthTarget->GetTexture()->SetName(L"GBufferDepth");
			}

			if (!m_compositeTarget)
			{
				m_compositeTarget = Renderer::CreateRenderTarget();
				Graphics::RenderTargetDesc desc = Graphics::RenderTargetDesc(Window::Instance().GetWidth(), Window::Instance().GetHeight(), PixelFormat::B8G8R8A8_UNorm, { 1, 0, 0, 1 });
				m_compositeTarget->Create("CompositeTarget", desc);
				m_compositeTarget->GetTexture()->SetName(L"CompositeTarget");
			}
#endif
		}

		bool useShadowCamera = false;

		void Renderpass::Render()
		{
			IS_PROFILE_FUNCTION();

			IMGUI_VALID(ImGui::Checkbox("Move shadow camera", &useShadowCamera));

			UpdateCamera(useShadowCamera ? m_shadowCamera : m_camera);
			//ShadowPass();
			Sample(useShadowCamera ? m_shadowCamera : m_camera);
			//Composite();
			Swapchain();
			ImGuiPass();
		}

		void Renderpass::Destroy()
		{
#ifndef RENDER_GRAPH_ENABLED
			if (m_vertexBuffer)
			{
				Renderer::FreeVertexBuffer(m_vertexBuffer);
				m_vertexBuffer = nullptr;
			}

			if (m_indexBuffer)
			{
				Renderer::FreeIndexBuffer(m_indexBuffer);
				m_indexBuffer = nullptr;
			}

			if (m_testTexture)
			{
				Renderer::FreeTexture(m_testTexture);
				m_testTexture = nullptr;
			}

			if (m_shadowTarget)
			{
				Renderer::FreeRenderTarget(m_shadowTarget);
				m_shadowTarget = nullptr;
			}

			if (m_colourTarget)
			{
				Renderer::FreeRenderTarget(m_colourTarget);
				m_colourTarget = nullptr;
			}

			if (m_depthTarget)
			{
				Renderer::FreeRenderTarget(m_depthTarget);
				m_depthTarget = nullptr;
			}

			if (m_compositeTarget)
			{
				Renderer::FreeRenderTarget(m_compositeTarget);
				m_compositeTarget = nullptr;
			}
#endif 

			m_testMesh.Destroy();

#ifdef RENDER_GRAPH_ENABLED
			RenderGraph::Instance().Release();
#endif //#ifdef RENDER_GRAPH_ENABLED
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

		void Renderpass::ShadowPass()
		{
#ifndef RENDER_GRAPH_ENABLED
			RHI_Shader* shaderPassShader = nullptr;
			{
				IS_PROFILE_SCOPE("ShaderPass-GetShader");
				ShaderDesc shaderDesc;
				shaderDesc.VertexFilePath = L"Resources/Shaders/hlsl/Shadow.hlsl";
				shaderPassShader = Renderer::GetShader(shaderDesc);
			}

			PipelineStateObject shaderPassPso{};
			{
				IS_PROFILE_SCOPE("ShaderPass-SetPipelineStateObject");
				shaderPassPso.Name = L"ShaderPass_PSO";
				shaderPassPso.Shader = shaderPassShader;
				shaderPassPso.CullMode = CullMode::None;
				shaderPassPso.Swapchain = false;
				shaderPassPso.DepthStencil = m_shadowTarget->GetTexture();
				shaderPassPso.DepthCompareOp = CompareOp::GreaterOrEqual;
				shaderPassPso.DepthSteniclClearValue = glm::vec2(0.0f, 0.0f);
				Renderer::SetPipelineStateObject(shaderPassPso);
			}

			int const shadowTargetWidth = m_shadowTarget->GetDesc().Width;
			int const shadowTargetHeight = m_shadowTarget->GetDesc().Height;

			Renderer::SetViewport(shadowTargetWidth, shadowTargetHeight);
			Renderer::SetScissor(shadowTargetWidth, shadowTargetHeight);

			UBO_Camera shadowCamera = GetReverseZDepthCamera(m_shadowCamera, false);
			Renderer::SetUniform(0, 0, &shadowCamera, sizeof(shadowCamera));

			Renderer::BindVertexBuffer(m_vertexBuffer);
			Renderer::BindIndexBuffer(m_indexBuffer);

			m_testMesh.Draw();
#endif
		}

		void Renderpass::Sample(UBO_Camera& camera)
		{
			IS_PROFILE_FUNCTION();

#ifndef RENDER_GRAPH_ENABLED
			RHI_Shader* gbufferShader = nullptr;
			{
				IS_PROFILE_SCOPE("GBuffer-GetShader");
				ShaderDesc shaderDesc;
				shaderDesc.VertexFilePath = L"Resources/Shaders/hlsl/GBuffer.hlsl";
				shaderDesc.PixelFilePath = L"Resources/Shaders/hlsl/GBuffer.hlsl";
				gbufferShader = Renderer::GetShader(shaderDesc);
			}
			PipelineStateObject gbufferPso= { };
			{
				IS_PROFILE_SCOPE("GBuffer-SetPipelineStateObject");
				gbufferPso.Name = L"GBuffer_PSO";
				gbufferPso.Shader = gbufferShader;
				gbufferPso.CullMode = CullMode::Front;
				gbufferPso.Swapchain = false;
				gbufferPso.RenderTargets = { m_colourTarget->GetTexture()};
				gbufferPso.DepthStencil = m_depthTarget->GetTexture();
				Renderer::SetPipelineStateObject(gbufferPso);
			}

			Renderer::SetViewport(Window::Instance().GetWidth(), Window::Instance().GetHeight());
			Renderer::SetScissor(Window::Instance().GetWidth(), Window::Instance().GetHeight());

			Renderer::BindVertexBuffer(m_vertexBuffer);
			Renderer::BindIndexBuffer(m_indexBuffer);

			{
				IS_PROFILE_SCOPE("GBuffer-SetUniform");
				Renderer::SetUniform(0, 0, &camera, sizeof(camera));
				UBO_ShadowCamera shadowCamera = GetReverseZDepthCamera(m_shadowCamera, false);
				shadowCamera.TextureSize = { m_shadowTarget->GetDesc().Width, m_shadowTarget->GetDesc().Height };
				Renderer::SetUniform(0, 1, &shadowCamera, sizeof(shadowCamera));
				Renderer::SetTexture(0, 2, m_shadowTarget->GetTexture());
			}

			//m_testMesh.Draw();
#endif

#ifdef COMMAND_LIST_RENDER_BATCH
			//RenderPipelineData renderData;
			//renderData.PSO = gbufferPso;
			//renderData.Viewport = glm::vec2(Window::Instance().GetWidth(), Window::Instance().GetHeight());
			//renderData.Siccsior = glm::vec2(Window::Instance().GetWidth(), Window::Instance().GetHeight());
			//
			//Renderer::s_FrameCommandList.AddRenderData(renderData);
#endif

#ifdef RENDER_GRAPH_ENABLED
			struct TestPassData
			{
				Mesh& TestMesh;
			};
			TestPassData passData =
			{
				m_testMesh
			};

			RenderGraph::Instance().AddPass<TestPassData>("GBuffer", [](TestPassData& data , RenderGraphBuilder& builder)
				{
					RHI_TextureCreateInfo textureCreateInfo = { };
					textureCreateInfo.Width = Window::Instance().GetWidth();
					textureCreateInfo.Height = Window::Instance().GetHeight();
					textureCreateInfo.Depth = 1;
					textureCreateInfo.TextureType = TextureType::Tex2D;
					textureCreateInfo.ImageUsage = ImageUsageFlagsBits::ColourAttachment;
					textureCreateInfo.Format = PixelFormat::R8G8B8A8_SNorm;
					RGTextureHandle colourRT = builder.CreateTexture("ColourRT", textureCreateInfo);
					builder.WriteTexture(colourRT);

					textureCreateInfo.Format = PixelFormat::D16_UNorm;
					textureCreateInfo.ImageUsage = ImageUsageFlagsBits::DepthStencilAttachment;
					RGTextureHandle depthStencil = builder.CreateTexture("DepthStencil", textureCreateInfo);
					builder.WriteDepthStencil(depthStencil);

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
						gbufferPso.CullMode = CullMode::Front;
						gbufferPso.Swapchain = false;
					}
					builder.SetPipeline(gbufferPso);

					builder.SetViewport(Window::Instance().GetWidth(), Window::Instance().GetHeight());
					builder.SetScissor(Window::Instance().GetWidth(), Window::Instance().GetHeight());
				},
				[](TestPassData& data, RenderGraphPassBase& pass, RHI_CommandList* cmdList)
				{
					cmdList->BeginRenderpass();
					cmdList->BindPipeline(pass.m_pso, nullptr);
					cmdList->EndRenderpass();

					//data.TestMesh.Draw(cmdList);

				}, std::move(passData));
#endif //RENDER_GRAPH_ENABLED
		}

		void Renderpass::Composite()
		{
#ifndef RENDER_GRAPH_ENABLED
			RHI_Shader* compositeShader = nullptr;
			{
				IS_PROFILE_SCOPE("Composite-GetShader");
				ShaderDesc shaderDesc;
				shaderDesc.VertexFilePath = L"Resources/Shaders/hlsl/Composite.hlsl";
				shaderDesc.PixelFilePath = L"Resources/Shaders/hlsl/Composite.hlsl";
				compositeShader = Renderer::GetShader(shaderDesc);
			}

			PipelineStateObject compositePso{};
			{
				IS_PROFILE_SCOPE("Composite-SetPipelineStateObject");
				compositePso.Name = L"CompositePso_PSO";
				compositePso.Shader = compositeShader;
				compositePso.CullMode = CullMode::None;
				compositePso.RenderTargets = { m_compositeTarget->GetTexture()};
				Renderer::SetPipelineStateObject(compositePso);
			}

			{
				IS_PROFILE_SCOPE("Composite-SetUniform");
				Renderer::SetTexture(0, 0, m_colourTarget->GetTexture());
				Renderer::SetTexture(0, 1, m_shadowTarget->GetTexture());
			}

			Renderer::Draw(3, 1, 0, 0);
#endif
		}

		void Renderpass::Swapchain()
		{
#ifndef RENDER_GRAPH_ENABLED
			RHI_Shader* swapchainShader = nullptr;
			{
				IS_PROFILE_SCOPE("Swapchain-GetShader");
				ShaderDesc shaderDesc;
				shaderDesc.VertexFilePath = L"Resources/Shaders/hlsl/Swapchain.hlsl";
				shaderDesc.PixelFilePath = L"Resources/Shaders/hlsl/Swapchain.hlsl";
				swapchainShader = Renderer::GetShader(shaderDesc);
			}

			PipelineStateObject swapchainPso{};
			{
				IS_PROFILE_SCOPE("Swapchain-SetPipelineStateObject");
				swapchainPso.Name = L"Swapchain_PSO";
				swapchainPso.Shader = swapchainShader;
				swapchainPso.CullMode = CullMode::None;
				swapchainPso.Swapchain = true;
				Renderer::SetPipelineStateObject(swapchainPso);
			}

			{
				IS_PROFILE_SCOPE("Swapchain-SetUniform");
				Renderer::SetTexture(0, 0, m_colourTarget->GetTexture());
			}

			Renderer::Draw(3, 1, 0, 0);
#endif

#ifdef RENDER_GRAPH_ENABLED
			struct TestPassData
			{ };
			RenderGraph::Instance().AddPass<TestPassData>("SwapchainPass", [](TestPassData& data, RenderGraphBuilder& builder)
				{
					RGTextureHandle colourRT = builder.GetTexture("ColourRT");
					builder.ReadTexture(colourRT);
			
					ShaderDesc shaderDesc;
					shaderDesc.VertexFilePath = L"Resources/Shaders/hlsl/Swapchain.hlsl";
					shaderDesc.PixelFilePath = L"Resources/Shaders/hlsl/Swapchain.hlsl";
					builder.SetShader(shaderDesc);
			
					PipelineStateObject swapchainPso = { };
					swapchainPso.Name = L"Swapchain_PSO";
					swapchainPso.CullMode = CullMode::None;
					builder.SetPipeline(swapchainPso);
			
					builder.SetViewport(Window::Instance().GetWidth(), Window::Instance().GetHeight());
					builder.SetScissor(Window::Instance().GetWidth(), Window::Instance().GetHeight());

					builder.SetAsRenderToSwapchain();
				},
				[](TestPassData& data, RenderGraphPassBase& pass, RHI_CommandList* cmdList)
				{
					cmdList->BeginRenderpass();
					cmdList->Draw(3, 1, 0, 0);
					cmdList->EndRenderpass();
				});
#endif //RENDER_GRAPH_ENABLED
		}

		void Renderpass::ImGuiPass()
		{
#ifdef RENDER_GRAPH_ENABLED
			struct TestPassData
			{ };
			RenderGraph::Instance().AddPass<TestPassData>("ImGuiPass", [](TestPassData& data, RenderGraphBuilder& builder)
				{
					builder.SetViewport(Window::Instance().GetWidth(), Window::Instance().GetHeight());
					builder.SetScissor(Window::Instance().GetWidth(), Window::Instance().GetHeight());
					builder.SetAsRenderToSwapchain();

					RenderpassDescription renderpassDescription = { };
					renderpassDescription.AddAttachment(AttachmentDescription::DontCare(PixelFormat::Unknown, Graphics::ImageLayout::PresentSrc));
					builder.SetRenderpass(renderpassDescription);
				},
				[](TestPassData& data, RenderGraphPassBase& pass, RHI_CommandList* cmdList)
				{
					cmdList->BeginRenderpass();
#ifdef IS_VULKAN_ENABLED
					if (GraphicsManager::Instance().IsVulkan())
					{
						ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), static_cast<RHI::Vulkan::RHI_CommandList_Vulkan*>(cmdList)->GetCommandList());
					}
#endif
#ifdef IS_DX12_ENABLED
					if (GraphicsManager::Instance().IsDX12())
					{
						ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), static_cast<RHI::DX12::RHI_CommandList_DX12*>(cmdList)->GetCommandList());
					}
#endif
					cmdList->EndRenderpass();
				});
#endif //RENDER_GRAPH_ENABLED
		}

		float previousTime = 0;

		void Renderpass::UpdateCamera(UBO_Camera& camera)
		{
			float deltaTime = (float)glfwGetTime() - previousTime;
			previousTime = (float)glfwGetTime();

			glm::mat4 viewMatrix = camera.View;

			// Get the camera's forward, right, up, and location vectors
			glm::vec4 vForward = viewMatrix[2];
			glm::vec4 vRight = viewMatrix[0];
			glm::vec4 vUp = viewMatrix[1];
			glm::vec4 vTranslation = viewMatrix[3];

			float frameSpeed = glfwGetKey(Window::Instance().GetRawWindow(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ? deltaTime * 200 : deltaTime * 25;
			//Input::IsKeyDown(KEY_LEFT_SHIFT) ? a_deltaTime * m_cameraSpeed * 2 : a_deltaTime * m_cameraSpeed;

			// Translate camera
			if (glfwGetKey(Window::Instance().GetRawWindow(), GLFW_KEY_W))
			{
				vTranslation -= vForward * frameSpeed;
			}
			if (glfwGetKey(Window::Instance().GetRawWindow(), GLFW_KEY_S))
			{
				vTranslation += vForward * frameSpeed;
			}
			if (glfwGetKey(Window::Instance().GetRawWindow(), GLFW_KEY_D))
			{
				vTranslation += vRight * frameSpeed;
			}
			if (glfwGetKey(Window::Instance().GetRawWindow(), GLFW_KEY_A))
			{
				vTranslation -= vRight * frameSpeed;
			}
			if (glfwGetKey(Window::Instance().GetRawWindow(), GLFW_KEY_Q))
			{
				vTranslation += vUp * frameSpeed;
			}
			if (glfwGetKey(Window::Instance().GetRawWindow(), GLFW_KEY_E))
			{
				vTranslation -= vUp * frameSpeed;
			}

			// check for camera rotation
			static bool sbMouseButtonDown = false;
			bool mouseDown = glfwGetMouseButton(Window::Instance().GetRawWindow(), GLFW_MOUSE_BUTTON_RIGHT);
			if (mouseDown)
			{
				viewMatrix[3] = vTranslation;

				static double siPrevMouseX = 0;
				static double siPrevMouseY = 0;

				if (sbMouseButtonDown == false)
				{
					sbMouseButtonDown = true;
					glfwGetCursorPos(Window::Instance().GetRawWindow(), &siPrevMouseX, &siPrevMouseY);
				}

				double mouseX = 0, mouseY = 0;
				glfwGetCursorPos(Window::Instance().GetRawWindow(), &mouseX, &mouseY);

				double iDeltaX = mouseX - siPrevMouseX;
				double iDeltaY = mouseY - siPrevMouseY;

				siPrevMouseX = mouseX;
				siPrevMouseY = mouseY;

				glm::mat4 mMat;

				// pitch
				if (iDeltaY != 0)
				{
					if (GraphicsManager::IsVulkan())
					{
						mMat = glm::axisAngleMatrix(vRight.xyz(), (float)-iDeltaY / 150.0f);
					}
					else
					{
						mMat = glm::axisAngleMatrix(vRight.xyz(), (float)-iDeltaY / 150.0f);
					}
					vRight = mMat * vRight;
					vUp = mMat * vUp;
					vForward = mMat * vForward;
				}

				// yaw
				if (iDeltaX != 0)
				{
					mMat = glm::axisAngleMatrix(glm::vec3(0, 1, 0), (float)-iDeltaX / 150.0f);
					vRight = mMat * vRight;
					vUp = mMat * vUp;
					vForward = mMat * vForward;
				}

				viewMatrix[0] = vRight;
				viewMatrix[1] = vUp;
				viewMatrix[2] = vForward;

				camera.View = viewMatrix;
			}
			else
			{
				sbMouseButtonDown = false;
			}

			if (useShadowCamera)
			{
				aspect = 1.0f;
				camera.Projection = glm::perspective(glm::radians(ShadowFOV), aspect, ShadowZNear, ShadowZFar);
				camera.ProjView = camera.Projection * glm::inverse(camera.View);
			}
			else
			{
				aspect = (float)Window::Instance().GetWidth() / (float)Window::Instance().GetHeight();
				camera.Projection = glm::perspective(glm::radians(90.0f), aspect, 0.1f, 5000.0f);
				camera.ProjView = camera.Projection * glm::inverse(camera.View);
			}

			glm::vec4 point = glm::vec4(2, 0, 5, 1);

			glm::mat4 transformMatrix = glm::mat4(1);
			transformMatrix[3] = glm::vec4(10, 00, 3, 1);
			glm::mat4 invTransformMatrix = glm::inverse(transformMatrix);

			glm::vec4 finalPoint = transformMatrix * point;
			glm::vec4 invFinalPoint = invTransformMatrix * point;
		}
	}
}