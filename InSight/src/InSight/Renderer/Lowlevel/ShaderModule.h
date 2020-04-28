#pragma once

#include "Insight/Core.h"

#include "Insight/Renderer/Lowlevel/ShaderModuleBase.h"

namespace Insight
{
	namespace Render
	{
		template<ShaderType type>
		class IS_API ShaderModule : public ShaderModuleBase
		{
		public:
			ShaderModule() = delete;

			ShaderModule(const Device* device, const std::string& filepath)
				: ShaderModuleBase(device, filepath, type)
			{ }
		};
	}
	using VertexShader = Render::ShaderModule<Render::ShaderType::Vertex>;
	using GemetryShader = Render::ShaderModule<Render::ShaderType::Geometry>;
	using FragmentShader = Render::ShaderModule<Render::ShaderType::Fragment>;

	using ComputeShader = Render::ShaderModule<Render::ShaderType::Compute>;
}