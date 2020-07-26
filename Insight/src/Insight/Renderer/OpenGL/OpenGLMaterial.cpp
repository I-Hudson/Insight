#include "ispch.h"
#ifdef IS_OPENGL
#include "Insight/Renderer/OpenGL/OpenGL.h"
#include "Insight/Renderer/OpenGL/OpenGLRenderer.h"
#include "Insight/Renderer/OpenGL/OpenGLMaterial.h"

#include "Insight/Time/Time.h"

namespace Insight
{
	namespace Render
	{
		OpenGLRenderer* OpenGLMaterial::s_Renderer;

		OpenGLMaterial::OpenGLMaterial()
		{
		}

		OpenGLMaterial::~OpenGLMaterial()
		{
		}

		void OpenGLMaterial::SetShader(Shader* shader)
		{
			
		}

		Shader* OpenGLMaterial::GetShader()
		{
			return m_shader;
		}

		void OpenGLMaterial::SetUniforms()
		{

		}

		MVPUniformBuffer m_mvp;
		static int temp = 0;
		void OpenGLMaterial::UpdateMVPUniform(const glm::mat4& proj, const glm::mat4& view, const glm::mat4& model)
		{
			
		}

		void OpenGLMaterial::UpdateUniform(const std::string& key, void* uniformData, size_t size, int binding)
		{

		}

		void OpenGLMaterial::UpdateSampler2D(const std::string& key, void* imageView, void* sampler, int binding)
		{

		}

		void OpenGLMaterial::Bind()
		{
		}

		void OpenGLMaterial::DestroyUniformBuffers()
		{

		}
	}
}
#endif