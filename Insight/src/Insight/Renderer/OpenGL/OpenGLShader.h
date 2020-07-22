#pragma once
#ifdef IS_OPENGL
#include "Insight/Core.h"
#include "Insight/Renderer/Shader.h"

#include "glm/glm.hpp"

namespace Insight
{
	namespace Render
	{
		struct ShaderData
		{
			const std::vector<std::string> ModuleNames;
			glm::vec2 Extent;
		};

		class IS_API OpenGLShader : public Shader
		{
		public:
			OpenGLShader(ShaderData& data);
			virtual ~OpenGLShader() override;

			virtual void Bind(void* context) override;
			virtual void Resize(int width, int height) override;

			virtual const ShaderData& GetData() const override { return m_shaderData; }

		private:
			void Create(ShaderData& data);
			unsigned int CreateShader(const char* shaderSource, const ShaderType& type);
			void Cleanup(const std::vector<unsigned int>& shaders);

			unsigned int ShaderTypeToGLShaderType(const ShaderType& type);

		private:
			ShaderData m_shaderData;
			unsigned int m_id;
		};
	}
}
#endif