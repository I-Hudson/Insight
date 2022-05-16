#include "Graphics/Renderpass.h"
#include "Graphics/RenderContext.h"
#include "Graphics/Window.h"
#include "Graphics/GraphicsManager.h"

#include "optick.h"
#include <glm/gtx/matrix_interpolation.hpp>
#include <glm/ext/matrix_clip_space.hpp>

namespace Insight
{
	namespace Graphics
	{
		void Renderpass::Create()
		{
			m_testMesh.LoadFromFile("./Resources/models/sponza_old/sponza.obj");

			if (m_camera.View == glm::mat4(0.0f))
			{
				float aspect = (float)Window::Instance().GetWidth() / (float)Window::Instance().GetHeight();
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
		}

		void Renderpass::Render()
		{
			OPTICK_EVENT();
			UpdateCamera();
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

			m_testMesh.Destroy();
		}

		glm::vec2 swapchainColour = { 0,0 };
		glm::vec2 swapchainColour2 = { 0,0 };

		void Renderpass::Sample()
		{
			OPTICK_EVENT();

			RHI_Shader* shader = nullptr;
			{
				//ZoneScopedN("GetShader");
				ShaderDesc shaderDesc;
				shaderDesc.VertexFilePath = L"Resources/Shaders/hlsl/Swapchain.hlsl";
				shaderDesc.PixelFilePath = L"Resources/Shaders/hlsl/Swapchain.hlsl";
				shader = Renderer::GetShader(shaderDesc);
			}

			PipelineStateObject pso{};
			{
				//ZoneScopedN("SetPipelineStateObject");
				pso.Name = L"Swapchain_PSO";
				pso.Shader = shader;
				pso.CullMode = CullMode::None;
				pso.RenderTargets.clear();
				pso.Swapchain = true;
				Renderer::SetPipelineStateObject(pso);
			}

			Renderer::SetViewport(Window::Instance().GetWidth(), Window::Instance().GetHeight());
			Renderer::SetScissor(Window::Instance().GetWidth(), Window::Instance().GetHeight());

			Renderer::BindVertexBuffer(m_vertexBuffer);
			Renderer::BindIndexBuffer(m_indexBuffer);

			IMGUI_VALID(ImGui::DragFloat2("Swapchain colour", &swapchainColour.x, 0.01f, 0.0f, 1.0f));
			IMGUI_VALID(ImGui::DragFloat2("Swapchain colour2", &swapchainColour2.x, 0.01f, 0.0f, 1.0f));
			{
				//ZoneScopedN("SetUniform");
				Renderer::SetUniform(0, 0, &swapchainColour, sizeof(swapchainColour));
				Renderer::SetUniform(0, 1, &swapchainColour2, sizeof(swapchainColour2));
				Renderer::SetUniform(0, 2, &m_camera, sizeof(m_camera));
			}

			m_testMesh.Draw();

			//Renderer::Draw(3, 1, 0, 0);
			//Renderer::DrawIndexed(3, 1, 0, 0, 0);
		}

		float previousTime = 0;

		void Renderpass::UpdateCamera()
		{
			float deltaTime = glfwGetTime() - previousTime;
			previousTime = glfwGetTime();

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
			m_camera.ProjView = m_camera.Projection * glm::inverse(m_camera.View);
		}
	}
}