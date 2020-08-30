#pragma once
#ifdef IS_OPENGL
#include "Insight/Core.h"
#include "Insight/Renderer/Shader.h"

#include "glm/glm.hpp"

namespace Platform
{
	class IS_API OpenGLShader : public Insight::Render::Shader
	{
	public:
		OpenGLShader(const std::vector<std::string>& shaders, glm::ivec2 extent);
		virtual ~OpenGLShader() override;

		virtual void Bind(void* context) override;
		virtual void Resize(int width, int height) override;

	private:
		void Create();
		unsigned int CreateShader(const char* shaderSource, const Insight::Render::ShaderType& type);
		void Cleanup(const std::vector<unsigned int>& shaders);

		unsigned int ShaderTypeToGLShaderType(const Insight::Render::ShaderType& type);

	private:
		std::vector<std::string> m_shaderPaths;
		glm::ivec2 m_extent;
		unsigned int m_id;
	};
}
#endif