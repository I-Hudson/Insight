#pragma once

#include "Engine/Core/Common.h"
#include "GPUShader.h"

namespace ShaderCompliation
{
	std::vector<U32> CompileGLSLToSpirV(const ShaderStages& stage, const std::string& shaderData);
}