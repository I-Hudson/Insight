#include "ispch.h"
#ifdef IS_OPENGL
#include "Insight/Renderer/OpenGL/OpenGL.h"
#include "Insight/Renderer/OpenGL/OpenGLMaterial.h"
#include "Insight/Renderer/OpenGL/OpenGLShader.h"
#include "Insight/Renderer/OpenGL/OpenGLRenderer.h"
#include "Insight/Module/WindowModule.h"

#include "Insight/Module/WindowModule.h"
#include "Insight/Event/EventManager.h"
#include "Insight/Component/MeshComponent.h"
#include "Insight/Camera.h"

#include "Insight/Config/Config.h"



namespace Insight
{
	namespace Render
	{
		OpenGLRenderer::OpenGLRenderer(RendererStartUpData& startupData)
			: m_windowModule(startupData.WindowModule)
		{
			ShaderData data 
			{ 
				{ "opengl_shaders/vertex.vert", "opengl_shaders/frag.frag" },
				glm::vec2(m_windowModule->GetWindow()->GetWidth(), m_windowModule->GetWindow()->GetHeight())
			};
			m_shader = NEW_ON_HEAP(OpenGLShader, data);
		}

		OpenGLRenderer::~OpenGLRenderer()
		{
			DELETE_ON_HEAP(m_shader);
		}

		void OpenGLRenderer::Clear()
		{
			glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}

		void OpenGLRenderer::Render(Camera* mainCamera, std::vector<MeshComponent*> meshes)
		{
		}

		void OpenGLRenderer::Present()
		{
			glfwSwapBuffers(m_windowModule->GetWindow()->m_window);
		}

		void OpenGLRenderer::RecreateFramebuffers(const Event& event)
		{
		}
	}
}
#endif