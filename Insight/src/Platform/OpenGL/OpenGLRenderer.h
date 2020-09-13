#pragma once
#ifdef IS_OPENGL
#include "Insight/Core.h"
#include "Insight/Renderer/Renderer.h"
#include "Insight/Module/WindowModule.h"
#include "Insight/Event/Event.h"

#include "Insight/Renderer/ShaderModule.h"
#include "Insight/Renderer/Material.h"
#include "Insight/Assimp/Mesh.h"

class CameraComponent;

namespace Platform
{
	class OpenGLMaterial;
	class OpenGLShader;

	class IS_API OpenGLRenderer : public Insight::Renderer
	{
	public:
		OpenGLRenderer();
		virtual ~OpenGLRenderer() override;

		virtual void Clear() override;
		virtual void Render(CameraComponent* mainCamera, std::vector<MeshComponent*> meshes) override;
		virtual void Present() override;

	private:
		void RecreateFramebuffers(const Insight::Event& event);

	private:

		Mesh* m_testMesh;
		OpenGLShader* m_shader;

		bool m_enableValidationLayers = true;
		std::vector<const char*> m_validationLayers;

		std::vector<const char*> m_deviceExtensions;

		Insight::Module::WindowModule* m_windowModule;

		friend OpenGLMaterial;
	};
}
#endif