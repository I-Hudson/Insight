#include "ispch.h"
#include "Shader.h"

namespace Insight
{
	namespace Render
	{
		ShaderModuleBase& Shader::GetShaderModule(const ShaderType& type, std::vector<ShaderModuleBase>& modules)
		{
			for (auto it = modules.begin(); it != modules.end(); ++it)
			{
				if ((*it).GetShaderType() == type)
				{
					return *it;
				}
			}
		}
	}
}