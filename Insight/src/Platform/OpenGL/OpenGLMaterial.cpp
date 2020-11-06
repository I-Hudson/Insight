#include "ispch.h"

#include "Platform/OpenGL/OpenGL.h"
#include "Platform/OpenGL/OpenGLRenderer.h"
#include "Platform/OpenGL/OpenGLMaterial.h"

#include "Insight/Time/Time.h"

namespace Platform
{
	OpenGLRenderer* OpenGLMaterial::s_Renderer;

	OpenGLMaterial::OpenGLMaterial()
	{
	}

	OpenGLMaterial::~OpenGLMaterial()
	{
	}

	void OpenGLMaterial::SetShader(Insight::Render::Shader* shader)
	{

	}

	Insight::Render::Shader* OpenGLMaterial::GetShader()
	{
		return m_shader;
	}

	void OpenGLMaterial::SetUniforms()
	{

	}

	MVPUniformBufferOpenGL m_mvp;
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

	MaterialRenderData OpenGLMaterial::IncrementUsageCount(const MeshComponent* meshComponent)
	{
		return MaterialRenderData();
	}

	void OpenGLMaterial::DecrementUsageCount(const MeshComponent* meshComponent)
	{
	}

	void OpenGLMaterial::Bind()
	{
	}

	void OpenGLMaterial::DestroyUniformBuffers()
	{

	}
}