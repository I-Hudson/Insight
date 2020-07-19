#pragma once

#include "Insight/Core.h"

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
	virtual const Insight::Render::Shader* GetShader() = 0;
	virtual void UpdateUniforms() = 0;
	virtual void UpdateLoadUniforms() = 0;
	virtual void UpdateLoadUniforms(const std::string& key, void* uniformData, size_t size, int binding, int index) = 0;

	static Material* Create();
};

