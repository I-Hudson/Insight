#include "ispch.h"
#ifdef IS_OPENGL
#include "Platform/OpenGL/OpenGL.h"
#include "Platform/OpenGL/OpenGLMaterial.h"
#include "Platform/OpenGL/OpenGLShader.h"
#include "Platform/OpenGL/OpenGLRenderer.h"
#include "Insight/Module/WindowModule.h"

#include "Insight/Module/ModuleManager.h"
#include "Insight/Module/WindowModule.h"
#include "Insight/Event/EventManager.h"
#include "Insight/Component/MeshComponent.h"
#include "Insight/Camera.h"

#include "Insight/Config/Config.h"



namespace Platform
{
	OpenGLRenderer::OpenGLRenderer()
	{
		m_windowModule = Insight::Module::ModuleManager::GetInstance()->GetModule<Insight::Module::WindowModule>();

		std::vector<std::string> shaderPaths = { "opengl_shaders/vertex.vert", "opengl_shaders/frag.frag" };
		glm::ivec2 extent = glm::ivec2(m_windowModule->GetWindow()->GetWidth(), m_windowModule->GetWindow()->GetHeight());
		m_shader = NEW_ON_HEAP(OpenGLShader, shaderPaths, extent);
	}

	OpenGLRenderer::~OpenGLRenderer()
	{
		DELETE_ON_HEAP(m_shader);
	}

	void OpenGLRenderer::Clear()
	{
		glClearColor(0.45f, 0.0f, 0.75f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRenderer::Render(CameraComponent* mainCamera, std::vector<MeshComponent*> meshes)
	{
	}

	void OpenGLRenderer::Present()
	{
		glfwSwapBuffers(m_windowModule->GetWindow()->m_window);
	}

	void OpenGLRenderer::RecreateFramebuffers(const Insight::Event& event)
	{
	}
}
#endif