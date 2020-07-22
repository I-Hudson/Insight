#pragma once

#include "Insight/Core.h"

#include "Insight/Renderer/ShaderModuleBase.h"

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
	using VertexShader = Render::ShaderModule<Render::ShaderType::VertexShader>;
	using GemetryShader = Render::ShaderModule<Render::ShaderType::GeometryShader>;
	using FragmentShader = Render::ShaderModule<Render::ShaderType::FragmentShader>;

	using ComputeShader = Render::ShaderModule<Render::ShaderType::ComputeShader>;
}