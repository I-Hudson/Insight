#include "ispch.h"
#include "Engine/Graphics/Shaders/GPUShader.h"
#include "Engine/Platform/Platform.h"
#include "Engine/FileSystem/FileSystem.h"
#include "Engine/Graphics/ShaderCompliation.h"

#include "Engine/Graphics/Graphics.h"
#include "Engine/GraphicsAPI/Vulkan/GPUShaderVulkan.h"

namespace Insight::Graphics
{
	GPUShaderStage::GPUShaderStage(const ShaderStage& stage, const std::string& str, const ShaderStageInput& input)
		: m_stage(stage)
		, m_parsed(false)
	{
		ASSERT(input == ShaderStageInput::RawData || input == ShaderStageInput::FilePath);

		if (input == ShaderStageInput::RawData)
		{
			m_loadedShaderString = str;
		}
		else if (input == ShaderStageInput::FilePath)
		{
			auto data = FileSystem::FileSystemManager::Instance()->ReadFileToVector(str);
			m_loadedShaderString = std::string(data.begin(), data.end());
		}
	}

	void GPUShaderStage::Parse()
	{
		if (m_loadedShaderString.empty())
		{
			return;
		}

		// Parse the shader stage raw data and get all the info we need.
		m_rawData = ShaderCompliation::CompileGLSLToSpirV(m_stage, m_loadedShaderString);

		spirv_cross::CompilerGLSL glsl(m_rawData);
		spirv_cross::ShaderResources resources = glsl.get_shader_resources();

		for (auto& resource : resources.stage_inputs)
		{
			ShaderStageBindings binding
			{
				resource.name,
				glsl.get_decoration(resource.id, spv::Decoration::DecorationLocation),
				glsl.get_type(resource.base_type_id),
				glsl.get_type(resource.base_type_id).vecsize,
				4 * glsl.get_type(resource.base_type_id).vecsize,
				GetStride()
			};
			m_inputs.push_back(binding);
		}

		for (auto& resource : resources.uniform_buffers)
		{
			ShaderStageUniform uniform
			{
				resource.name,
				glsl.get_decoration(resource.id, spv::Decoration::DecorationDescriptorSet),
				glsl.get_decoration(resource.id, spv::Decoration::DecorationBinding),
				(u32)glsl.get_declared_struct_size(glsl.get_type(resource.base_type_id))
			};
			m_uniforms.push_back(uniform);
		}

		for (auto& resource : resources.sampled_images)
		{
			ShaderStageSampler2D sampler2D
			{
				resource.name,
				glsl.get_decoration(resource.id, spv::Decoration::DecorationDescriptorSet),
				glsl.get_decoration(resource.id, spv::Decoration::DecorationBinding)
			};
			m_samplers.push_back(sampler2D);
		}

		m_parsed = true;
	}

	const u32 GPUShaderStage::GetInputsSize() const
	{
		u32 size = 0;
		for (auto& input : m_inputs)
		{
			size += input.ByteSize;
		}
		return size;
	}

	u32 GPUShaderStage::GetStride()
	{
		u32 stride = 0;
		for (auto& input : m_inputs)
		{
			stride += input.ByteSize;
		}
		return stride;
	}

	GPUShader* GPUShader::New()
	{
		if (::Graphics::IsVulkan())
		{
			return ::New<GraphicsAPI::Vulkan::GPUShaderVulkan>();
		}
		ASSERT(false && "No GPUShader for graphics API exists.");
		return nullptr;
	}

	GPUShader::GPUShader()
		: m_stages(
			std::array<GPUShaderStage, (u64)ShaderStage::Count>{
			GPUShaderStage(ShaderStage::Vertex, "", ShaderStageInput::RawData),
			GPUShaderStage(ShaderStage::TessControl, "", ShaderStageInput::RawData),
			GPUShaderStage(ShaderStage::TessEvaluation, "", ShaderStageInput::RawData),
			GPUShaderStage(ShaderStage::Geometry, "", ShaderStageInput::RawData),
			GPUShaderStage(ShaderStage::Fragment, "", ShaderStageInput::RawData),
			GPUShaderStage(ShaderStage::Compute, "", ShaderStageInput::RawData) })
	{
	}

	GPUShader::~GPUShader()
	{
	}
	void GPUShader::SetStage(const GPUShaderStage& shaderStage)
	{
		GPUShaderStage& stage = m_stages.at((u32)shaderStage.GetStage());
		stage = shaderStage;
	}

	void GPUShader::SetStage(const ShaderStage& stage, const std::string& str, const ShaderStageInput& input)
	{
		GPUShaderStage gpuStage(stage, str, input);
		SetStage(gpuStage);
	}

	///
	// GPUPipeline
	///
	GPUPipeline* GPUPipeline::New()
	{
		if (::Graphics::IsVulkan())
		{
			return ::New<GraphicsAPI::Vulkan::GPUPipelineVulkan>();
		}
		ASSERT(false && "No GPUPipeline for graphics API exists.");
		return nullptr;
	}
}