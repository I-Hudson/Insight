#pragma once

#include "Engine/Core/Common.h"
#include "Enums.h"

namespace ShaderCompliation
{
	std::vector<U32> CompileGLSLToSpirV(const ShaderStage& stage, const std::string& shaderData);
}