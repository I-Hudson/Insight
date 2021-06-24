#pragma once

#include "Engine/Core/Common.h"
#include "Enums.h"

namespace ShaderCompliation
{
	std::vector<u32> CompileGLSLToSpirV(const ShaderStage& stage, const std::string& shaderData, bool optimise = false);
}