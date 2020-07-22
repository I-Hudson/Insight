#include "ispch.h"
#ifdef IS_OPENGL
#include "Insight/Renderer/OpenGL/OpenGL.h"
#include "Insight/Renderer/OpenGL/OpenGLShader.h"

namespace Insight
{
	namespace Render
	{
		OpenGLShader::OpenGLShader(ShaderData& data)
			: m_shaderData(data)
		{
			Create(m_shaderData);
		}

		OpenGLShader::~OpenGLShader()
		{
			glDeleteProgram(m_id);
			UNTRACK_OBJECT(&m_id);
		}

		void OpenGLShader::Bind(void* context)
		{
			glUseProgram(m_id);
		}

		void OpenGLShader::Resize(int width, int height)
		{

		}

		void OpenGLShader::Create(ShaderData& data)
		{
			std::vector<unsigned int> shaders;
			for (auto it = data.ModuleNames.begin(); it != data.ModuleNames.end(); ++it)
			{
				ShaderModuleBase shaderModule((*it).c_str());
				unsigned int shader = CreateShader(shaderModule.GetData().RawSource.c_str(), shaderModule.GetShaderType());
				if (shader == GL_FALSE)
				{
					Cleanup(shaders);
					return;
				}

				shaders.push_back(shader);
				m_shaderMetaData.push_back(shaderModule.GetData());
			}

			m_id = glCreateProgram();
			TRACK_OBJECT(&m_id);

			for (auto it = shaders.begin(); it != shaders.end(); ++it)
			{
				glAttachShader(m_id, (*it));
			}

			glLinkProgram(m_id);

			int success;
			glGetProgramiv(m_id, GL_LINK_STATUS, &success);
			if (!success) 
			{
				GLint maxLength = 0;
				glGetProgramiv(m_id, GL_INFO_LOG_LENGTH, &maxLength);

				// The maxLength includes the NULL character
				std::vector<GLchar> infoLog(maxLength);
				glGetProgramInfoLog(m_id, maxLength, &maxLength, &infoLog[0]);

				Cleanup(shaders);

				IS_CORE_ERROR("{0}", infoLog.data());
				IS_CORE_ASSERT(false, "OpenGL Program failed to compile!");

				return;
			}

			Cleanup(shaders);
		}

		unsigned int OpenGLShader::CreateShader(const char* shaderSource, const ShaderType& type)
		{
			unsigned int shaderType = ShaderTypeToGLShaderType(type);

			unsigned int shader = glCreateShader(shaderType);
			glShaderSource(shader, 1, &shaderSource, 0);

			glCompileShader(shader);

			int isCompiled;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
			if (isCompiled == GL_FALSE)
			{
				GLint maxLength = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

				// The maxLength includes the NULL character
				std::vector<GLchar> infoLog(maxLength);
				glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

				glDeleteShader(shader);

				IS_CORE_ERROR("{0}", infoLog.data());
				IS_CORE_ASSERT(false, "Vertex BaseShader compilation failure!");

				return GL_FALSE;
			}
			return shader;
		}

		void OpenGLShader::Cleanup(const std::vector<unsigned int>& shaders)
		{
			for (auto it = shaders.begin(); it != shaders.end(); ++it)
			{
				glDeleteShader((*it));
			}
		}

		unsigned int OpenGLShader::ShaderTypeToGLShaderType(const ShaderType& type)
		{
			switch (type)
			{
			case ShaderType::VertexShader:  return GL_VERTEX_SHADER;
			case ShaderType::FragmentShader:  return GL_FRAGMENT_SHADER;
			case ShaderType::GeometryShader:  return GL_GEOMETRY_SHADER;
			case ShaderType::ComputeShader:  return GL_COMPUTE_SHADER;
			default: return 0;
			}
		}
	}
}
#endif