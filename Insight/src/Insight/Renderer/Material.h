#pragma once

#include "Insight/Core.h"
#include "../vendor/glm/glm/glm.hpp"

namespace Insight
{
	namespace Render
	{
		class Shader;
	}
}

class IS_API Material
{
public:
	virtual ~Material() { }

	virtual void SetShader(Insight::Render::Shader* shader) = 0;
	virtual Insight::Render::Shader* GetShader() = 0;
	virtual void SetUniforms() = 0;
	virtual void UpdateMVPUniform(const glm::mat4& proj, const glm::mat4& view, const glm::mat4& model) = 0;
	virtual void UpdateUniform(const std::string& key, void* uniformData, size_t size, int binding) = 0;
	virtual void UpdateSampler2D(const std::string& key, void* imageView, void* sampler, int binding) = 0;

	static Material* Create();
};

