#include "ispch.h"
#include "GPUShader.h"
#include "Engine/Platform/Platform.h"
#include "Engine/FileSystem/FileSystem.h"
#include "Engine/Graphics/ShaderCompliation.h"
#include "spirv_cross/spirv_glsl.hpp"

GPUShaderStage::GPUShaderStage(const ShaderStages& stage, const std::string& str, const ShaderStageInput& input)
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
			glsl.get_type(resource.base_type_id).vecsize
		};
		m_inputs.push_back(binding);
	}

	m_parsed = true;
}

GPUShader* GPUShader::New()
{
	return nullptr;
}

GPUShader::GPUShader()
	: m_stages(
		std::array<GPUShaderStage, (size_t)ShaderStages::Count>{
		GPUShaderStage(ShaderStages::Vertex, "", ShaderStageInput::RawData) ,
		GPUShaderStage(ShaderStages::TessControl, "", ShaderStageInput::RawData) ,
		GPUShaderStage(ShaderStages::TessEvaluation, "", ShaderStageInput::RawData) ,
		GPUShaderStage(ShaderStages::Geometry, "", ShaderStageInput::RawData) ,
		GPUShaderStage(ShaderStages::Fragment, "", ShaderStageInput::RawData) ,
		GPUShaderStage(ShaderStages::Compute, "", ShaderStageInput::RawData) })
{
}

GPUShader::~GPUShader()
{
}
void GPUShader::SetStage(const GPUShaderStage& shaderStage)
{
	m_stages[(size_t)shaderStage.GetStage()] = shaderStage;
}

void GPUShader::SetStage(const ShaderStages& stage, const std::string& str, const ShaderStageInput& input)
{
	GPUShaderStage gpuStage(stage, str, input);
	SetStage(gpuStage);
}
