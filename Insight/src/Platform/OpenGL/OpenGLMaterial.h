#pragma once

#include "Insight/Core.h"
#include "Insight/Renderer/Shader.h"
#include "Insight/Renderer/Material.h"
#include <string>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <unordered_map>

namespace Platform
{
	class OpenGLRenderer;

	struct UniformDataOpenGL
	{
		void* Data;
		int Size;
		int Binding;
	};

	struct SamplerDataOpenGL
	{
		int Binding;
	};

	struct MVPUniformBufferOpenGL
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

		virtual void SetShader(Insight::Render::Shader* shader) override;
		virtual Insight::Render::Shader* GetShader() override;
		virtual void SetUniforms() override;
		virtual void UpdateMVPUniform(const glm::mat4& proj, const glm::mat4& view, const glm::mat4& model) override;
		virtual void UpdateUniform(const std::string& key, void* uniformData, size_t size, int binding) override;
		virtual void UpdateSampler2D(const std::string& key, void* imageView, void* sampler, int binding) override;

		virtual MaterialRenderData IncrementUsageCount(const MeshComponent* meshComponent) override;
		virtual void DecrementUsageCount(const MeshComponent* meshComponent) override;

		void Bind();

	private:
		void DestroyUniformBuffers();

	private:
		Insight::Render::Shader* m_shader;

		std::unordered_map<std::string, UniformDataOpenGL> m_uniformData;
		std::unordered_map<std::string, SamplerDataOpenGL> m_samplerData;

		static OpenGLRenderer* s_Renderer;
		friend OpenGLRenderer;
	};
}