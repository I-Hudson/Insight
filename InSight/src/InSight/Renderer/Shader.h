#pragma once

#include "Insight/Core.h"
#include "Insight/Renderer/ShaderModuleBase.h"

namespace Insight
{
	namespace Render
	{
		class Device;
		struct ShaderData;

		class IS_API Shader
		{
		public:
			virtual ~Shader() { };

			virtual void Bind(void* context) = 0;
			virtual void Resize(int width, int height) = 0;

			virtual const ShaderData& GetData() const = 0;
			const std::vector<ParsedShadeData>& GetMetaData() const { return m_shaderMetaData; }

		protected:
			virtual ShaderModuleBase& GetShaderModule(const ShaderType& type, std::vector<ShaderModuleBase>& modules);

			std::vector<ParsedShadeData> m_shaderMetaData;
		};
	}
}