#include "ispch.h"
#include "Engine/Graphics/Shaders/GPUShader.h"
#include "Engine/Platform/Platform.h"
#include "Engine/FileSystem/FileSystem.h"
#include "Engine/Graphics/ShaderCompliation.h"

#include "Engine/Graphics/Graphics.h"
#include "Engine/GraphicsAPI/Vulkan/GPUShaderVulkan.h"
#include "spirv_reflect.h"

namespace Insight::Graphics
{
	u32 SpvFormatToByteSize(SpvReflectFormat format)
	{
		switch (format)
		{
			case SPV_REFLECT_FORMAT_UNDEFINED: ASSERT(false);
			case SPV_REFLECT_FORMAT_R32_UINT: return 4;
			case SPV_REFLECT_FORMAT_R32_SINT: return 4;
			case SPV_REFLECT_FORMAT_R32_SFLOAT: return 4;
			case SPV_REFLECT_FORMAT_R32G32_UINT: return 8;
			case SPV_REFLECT_FORMAT_R32G32_SINT: return 8;
			case SPV_REFLECT_FORMAT_R32G32_SFLOAT: return 8;
			case SPV_REFLECT_FORMAT_R32G32B32_UINT: return 12;
			case SPV_REFLECT_FORMAT_R32G32B32_SINT: return 12;
			case SPV_REFLECT_FORMAT_R32G32B32_SFLOAT: return 12;
			case SPV_REFLECT_FORMAT_R32G32B32A32_UINT: return 16;
			case SPV_REFLECT_FORMAT_R32G32B32A32_SINT: return 16;
			case SPV_REFLECT_FORMAT_R32G32B32A32_SFLOAT: return 16;
			case SPV_REFLECT_FORMAT_R64_UINT: return 8;
			case SPV_REFLECT_FORMAT_R64_SINT: return 8;
			case SPV_REFLECT_FORMAT_R64_SFLOAT: return 8;
			case SPV_REFLECT_FORMAT_R64G64_UINT: return 16;
			case SPV_REFLECT_FORMAT_R64G64_SINT: return 16;
			case SPV_REFLECT_FORMAT_R64G64_SFLOAT: return 16;
			case SPV_REFLECT_FORMAT_R64G64B64_UINT: return 24;
			case SPV_REFLECT_FORMAT_R64G64B64_SINT: return 24;
			case SPV_REFLECT_FORMAT_R64G64B64_SFLOAT: return 24;
			case SPV_REFLECT_FORMAT_R64G64B64A64_UINT: return 32;
			case SPV_REFLECT_FORMAT_R64G64B64A64_SINT: return 32;
			case SPV_REFLECT_FORMAT_R64G64B64A64_SFLOAT: return 32;
			default:  ASSERT(false);
		}
		return 0;
	}

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

		SpvReflectShaderModule module;
		SpvReflectResult result = spvReflectCreateShaderModule(sizeof(u32) * m_rawData.size(), m_rawData.data(), &module);
		ASSERT(result == SPV_REFLECT_RESULT_SUCCESS);

		// Enumerate and extract shader's input variables
		uint32_t var_count = 0;
		result = spvReflectEnumerateInputVariables(&module, &var_count, NULL);
		ASSERT(result == SPV_REFLECT_RESULT_SUCCESS);


		std::vector<SpvReflectInterfaceVariable*> input_vars;
		input_vars.resize(var_count);
		result = spvReflectEnumerateInputVariables(&module, &var_count, input_vars.data());
		ASSERT(result == SPV_REFLECT_RESULT_SUCCESS);
		std::sort(input_vars.begin(), input_vars.end(), [](const SpvReflectInterfaceVariable* o, const SpvReflectInterfaceVariable* o1)			  
				  {
					  return o->location < o1->location;
				  });
		for (auto& resource : input_vars)
		{
			if (resource->built_in != -1)
			{
				continue;
			}

			ShaderStageBindings binding
			{
				resource->name,
				resource->location,
				"",
				SpvFormatToByteSize(resource->format),
				resource->format,
				GetStride()
			};
			m_inputs.push_back(binding);
		}

		result = spvReflectEnumerateDescriptorSets(&module, &var_count, nullptr);
		ASSERT(result == SPV_REFLECT_RESULT_SUCCESS);

		std::vector<SpvReflectDescriptorSet*> descriptors;
		descriptors.resize(var_count);
		result = spvReflectEnumerateDescriptorSets(&module, &var_count, descriptors.data());
		ASSERT(result == SPV_REFLECT_RESULT_SUCCESS);
		for (auto& resource : descriptors)
		{
			for (u32 i = 0; i < resource->binding_count; ++i)
			{
				auto& binding = resource->bindings[i];
				if (binding->descriptor_type == SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
				{
					ShaderStageBuffer uniform
					{
						binding->name,
						binding->set,
						binding->binding,
						binding->block.size
					};
					m_uniforms.push_back(uniform);
				}
				else if (binding->descriptor_type == SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER)
				{
					ShaderStageBuffer storage
					{
						binding->name,
						binding->set,
						binding->binding,
						binding->block.size
					};
					m_storages.push_back(storage);
				}
				else if (binding->descriptor_type == SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
				{
					ShaderStageSampler2D sampler2D
					{
						binding->name,
						binding->set,
						binding->binding,
					};
					m_samplers.push_back(sampler2D);
				}
			}
		}

		result = spvReflectEnumeratePushConstantBlocks(&module, &var_count, nullptr);
		ASSERT(result == SPV_REFLECT_RESULT_SUCCESS);
		std::vector<SpvReflectBlockVariable*> pushConstants;
		pushConstants.resize(var_count);
		result = spvReflectEnumeratePushConstantBlocks(&module, &var_count, pushConstants.data());
		ASSERT(result == SPV_REFLECT_RESULT_SUCCESS);
		for (auto& pushConstant :  pushConstants)
		{
			m_pushConstants.push_back(ShaderStagePushConstant
									  {
										  pushConstant->size,
										  pushConstant->offset
									  });
		}

		// Destroy the reflection data when no longer required.
		spvReflectDestroyShaderModule(&module);
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