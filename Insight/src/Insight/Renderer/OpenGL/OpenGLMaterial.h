#pragma once
#ifdef  IS_OPENGL
#include "Insight/Core.h"
#include "Insight/Renderer/Shader.h"
#include "Insight/Renderer/Material.h"
#include <string>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <unordered_map>

namespace Insight
{
	namespace Render
	{
		class OpenGLRenderer;

		struct UniformData
		{
			void* Data;
			int Size;
			int Binding;
		};

		struct SamplerData
		{
			int Binding;
		};

		struct MVPUniformBuffer
		{
			glm::mat4 u_model;
			glm::mat4 u_view;
			glm::mat4 u_proj;
		};

		class OpenGLMaterial : public Material
		{
		public:
			OpenGLMaterial();
			virtual ~OpenGLMaterial() override;

			virtual void SetShader(Shader* shader) override;
			virtual const Shader* GetShader() override;
			virtual void SetUniforms() override;
			virtual void UpdateMVPUniform(const glm::mat4& proj, const glm::mat4& view, const glm::mat4& model) override;
			virtual void UpdateUniform(const std::string& key, void* uniformData, size_t size, int binding) override;
			virtual void UpdateSampler2D(const std::string& key, void* imageView, void* sampler, int binding) override;

			void Bind();

		private:
			void DestroyUniformBuffers();

		private:
			Shader* m_shader;

			std::unordered_map<std::string, UniformData> m_uniformData;
			std::unordered_map<std::string, SamplerData> m_samplerData;

			static OpenGLRenderer* s_Renderer;
			friend OpenGLRenderer;
		};
	}
}
#endif // IS_OPENGL