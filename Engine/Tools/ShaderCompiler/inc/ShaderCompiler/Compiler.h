#pragma once

#include "Graphics/Enums.h"
#include "Graphics/ShaderDesc.h"
#include "Graphics/Descriptors.h"

#include <string>
#include <vector>

struct IDxcBlob;
struct IDxcUtils;
struct IDxcCompiler3;
struct IDxcResult;

namespace Insight::ShaderCompiler
{
	enum class ShaderCompilerLanguage
	{
		Spirv,
		Hlsl
	};

	// TODO Low: Replace IDxc* with a ref counter object. (This needs to be custom due to compile time)
    class Compiler
    {
		Compiler();
		Compiler(const Compiler& other) = delete;
		Compiler(Compiler&& other) = delete;
		~Compiler();

		std::string StageToFuncName(Graphics::ShaderStageFlagBits stage);
		std::string StageToProfileTarget(Graphics::ShaderStageFlagBits stage);

		IDxcBlob* Compile(Graphics::ShaderStageFlagBits stage, std::string_view filePath, ShaderCompilerLanguage languageToCompileTo);
		IDxcBlob* Compile(Graphics::ShaderStageFlagBits stage, std::string name, const std::vector<Byte>& shaderData, ShaderCompilerLanguage languageToCompileTo);

		void GetDescriptorSets(Graphics::ShaderStageFlagBits stage, std::vector<Graphics::DescriptorSet>& descriptor_sets, Graphics::PushConstant& push_constant);
		std::vector<Graphics::ShaderInputLayout> GetInputLayout();

		Graphics::DescriptorType SpvReflectDescriptorTypeToDescriptorType(u32 type);
		Graphics::DescriptorResourceType SpvReflectDescriptorResourceTypeToDescriptorResourceType(u32 type);

		ShaderCompilerLanguage m_languageToCompileTo;

		IDxcUtils* DXUtils = nullptr;
		IDxcCompiler3* DXCompiler = nullptr;

		IDxcResult* ShaderCompileResults = nullptr;
		IDxcResult* ShaderReflectionResults = nullptr;
	};
}