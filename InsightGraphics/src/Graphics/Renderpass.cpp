#include "Graphics/Renderpass.h"
#include "Graphics/RenderContext.h"
#include "Graphics/Window.h"
#include "Graphics/GraphicsManager.h"

#include "Graphics/RenderTarget.h"

#include "Core/Profiler.h"

#include <glm/gtx/matrix_interpolation.hpp>
#include <glm/ext/matrix_clip_space.hpp>

namespace Insight
{
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
			}

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
				int const textureSize = oneK * 4;
				Graphics::RenderTargetDesc desc = Graphics::RenderTargetDesc(textureSize, textureSize, PixelFormat::D16_UNorm, { 1, 0, 0, 1 });
				m_shadowTarget->Create("ShadowPassDepth", desc);
				m_shadowTarget->GetTexture()->SetName(L"ShadowPassDepth");
			}

			if (!m_colourTarget)
			{
				m_colourTarget = Renderer::CreateRenderTarget();
				Graphics::RenderTargetDesc desc = Graphics::RenderTargetDesc(Window::Instance().GetWidth(), Window::Instance().GetHeight(), PixelFormat::B8G8R8A8_UNorm, { 1, 0, 0, 1 });
				m_colourTarget->Create("StandardColour", desc);
				m_shadowTarget->GetTexture()->SetName(L"StandardColour");
			}

			if (!m_depthTarget)
			{
				m_depthTarget = Renderer::CreateRenderTarget();
				Graphics::RenderTargetDesc desc = Graphics::RenderTargetDesc(Window::Instance().GetWidth(), Window::Instance().GetHeight(), PixelFormat::D32_Float, { 1, 0, 0, 1 });
				m_depthTarget->Create("StandardDepth", desc);
				m_shadowTarget->GetTexture()->SetName(L"StandardDepth");
			}
		}

		void Renderpass::Render()
		{
			IS_PROFILE_FUNCTION();
			UpdateCamera();
			ShadowPass();
			Sample();
		}

		void Renderpass::Destroy()
		{
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

			if (m_depthTarget)
			{
				Renderer::FreeRenderTarget(m_depthTarget);
				m_depthTarget = nullptr;
			}

			m_testMesh.Destroy();
		}

		glm::vec2 swapchainColour = { 0,0 };
		glm::vec2 swapchainColour2 = { 0,0 };

		void Renderpass::ShadowPass()
		{
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
				shaderPassPso.DepthStencil = m_shadowTarget;
				shaderPassPso.DepthSteniclClearValue = glm::vec2(1.0f, 0.0f);
				Renderer::SetPipelineStateObject(shaderPassPso);
			}

			int const shadowTargetWidth = m_shadowTarget->GetDesc().Width;
			int const shadowTargetHeight = m_shadowTarget->GetDesc().Height;

			Renderer::SetViewport(shadowTargetWidth, shadowTargetHeight);
			Renderer::SetScissor(shadowTargetWidth, shadowTargetHeight);

			//UBO_Camera shadowCamera = m_camera;

			//float const zNear = 0.1f;
			//float const zFar = 5000.0f;

			//float h = 1.0 / glm::tan(glm::radians(90.0f) * 0.5f);
			//float aspect = (float)Window::Instance().GetWidth() / (float)Window::Instance().GetHeight();
			//float w = h / aspect;
			//float a = -zNear / (zFar - zNear);
			//float b = (zNear * zFar) / (zFar - zNear);

			//shadowCamera.Projection = glm::mat4(
			//	w, 0.0f, 0.0f, 0.0f,
			//	0.0f, -h, 0.0f, 0.0f,
			//	0.0f, 0.0f, a, 0.0f,
			//	0.0f, 0.0f, b, 0.0f);
			//shadowCamera.Projection = glm::perspective(glm::radians(90.0f), aspect, zFar, zNear);
			//shadowCamera.ProjView = shadowCamera.Projection * glm::inverse(shadowCamera.View);

			//Renderer::SetUniform(0, 0, &shadowCamera, sizeof(shadowCamera));
			Renderer::SetUniform(0, 0, &m_camera, sizeof(m_camera));

			Renderer::BindVertexBuffer(m_vertexBuffer);
			Renderer::BindIndexBuffer(m_indexBuffer);

			m_testMesh.Draw();
		}

		void Renderpass::Sample()
		{
			IS_PROFILE_FUNCTION();

			RHI_Shader* gbufferShader = nullptr;
			{
				IS_PROFILE_SCOPE("GBuffer-GetShader");
				ShaderDesc shaderDesc;
				shaderDesc.VertexFilePath = L"Resources/Shaders/hlsl/GBuffer.hlsl";
				shaderDesc.PixelFilePath = L"Resources/Shaders/hlsl/GBuffer.hlsl";
				gbufferShader = Renderer::GetShader(shaderDesc);
			}
			PipelineStateObject gbufferPso{};
			{
				IS_PROFILE_SCOPE("GBuffer-SetPipelineStateObject");
				gbufferPso.Name = L"GBuffer_PSO";
				gbufferPso.Shader = gbufferShader;
				gbufferPso.CullMode = CullMode::Front;
				gbufferPso.Swapchain = false;
				gbufferPso.RenderTargets = { m_colourTarget };
				gbufferPso.DepthStencil = m_depthTarget;
				Renderer::SetPipelineStateObject(gbufferPso);
			}

			Renderer::SetViewport(Window::Instance().GetWidth(), Window::Instance().GetHeight());
			Renderer::SetScissor(Window::Instance().GetWidth(), Window::Instance().GetHeight());

			Renderer::BindVertexBuffer(m_vertexBuffer);
			Renderer::BindIndexBuffer(m_indexBuffer);

			IMGUI_VALID(ImGui::DragFloat2("Swapchain colour", &swapchainColour.x, 0.01f, 0.0f, 1.0f));
			IMGUI_VALID(ImGui::DragFloat2("Swapchain colour2", &swapchainColour2.x, 0.01f, 0.0f, 1.0f));
			{
				IS_PROFILE_SCOPE("GBuffer-SetUniform");
				Renderer::SetUniform(0, 0, &m_camera, sizeof(m_camera));
				Renderer::SetTexture(0, 1, m_depthTarget->GetTexture());
			}

			m_testMesh.Draw();

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
		}

		float previousTime = 0;

		void Renderpass::UpdateCamera()
		{
			float deltaTime = (float)glfwGetTime() - previousTime;
			previousTime = (float)glfwGetTime();

			glm::mat4 viewMatrix = m_camera.View;

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

				m_camera.View = viewMatrix;
			}
			else
			{
				sbMouseButtonDown = false;
			}

			aspect = (float)Window::Instance().GetWidth() / (float)Window::Instance().GetHeight();
			m_camera.Projection = glm::perspective(glm::radians(90.0f), aspect, 0.1f, 5000.0f);
			m_camera.ProjView = m_camera.Projection * glm::inverse(m_camera.View);
		}
	}
}