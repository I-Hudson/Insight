#pragma once

#include "Insight/Core.h"
#include "Insight/Renderer/ShaderModuleBase.h"
#include "Insight/Memory/MemoryManager.h"
#include "Insight/UUID.h"

namespace Insight
{
	namespace Render
	{
		class Device;
		struct ShaderData;

		class IS_API Shader : public Insight::UUID
		{
		public:
			Shader() : Insight::UUID() { TRACK_CLASS(); }
			virtual ~Shader() { UNTRACK_CLASS(); };

			virtual void Bind(void* context) = 0;
			virtual void Resize(int width, int height) = 0;

			const std::vector<ParsedShadeData>& GetMetaData() const { return m_shaderMetaData; }

		protected:
			virtual ShaderModuleBase& GetShaderModule(const ShaderType& type, std::vector<ShaderModuleBase>& modules);

			std::vector<ParsedShadeData> m_shaderMetaData;
		};
	}
}