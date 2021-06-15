#include "ispch.h"
#include "Engine/Graphics/ShaderCompliation.h"

#include "glslang/Public/ShaderLang.h"
#include "SPIRV/GlslangToSpv.h"
#include "StandAlone/DirStackFileIncluder.h"

EShLanguage GetShaderStage(const ShaderStage& stage)
{
	if (stage == ShaderStage::Vertex)
	{
		return EShLangVertex;
	}
	else if (stage == ShaderStage::TessControl)
	{
		return EShLangTessControl;
	}
	else if (stage == ShaderStage::TessEvaluation)
	{
		return EShLangTessEvaluation;
	}
	else if (stage == ShaderStage::Geometry)
	{
		return EShLangGeometry;
	}
	else if (stage == ShaderStage::Fragment)
	{
		return EShLangFragment;
	}
	else if (stage == ShaderStage::Compute) 
	{
		return EShLangCompute;
	}
	else 
	{
		ASSERT(false && "Unknown shader stage");
		return EShLangCount;
	}
}

const TBuiltInResource DefaultTBuiltInResource = {
	/* .MaxLights = */ 32,
	/* .MaxClipPlanes = */ 6,
	/* .MaxTextureUnits = */ 32,
	/* .MaxTextureCoords = */ 32,
	/* .MaxVertexAttribs = */ 64,
	/* .MaxVertexUniformComponents = */ 4096,
	/* .MaxVaryingFloats = */ 64,
	/* .MaxVertexTextureImageUnits = */ 32,
	/* .MaxCombinedTextureImageUnits = */ 80,
	/* .MaxTextureImageUnits = */ 32,
	/* .MaxFragmentUniformComponents = */ 4096,
	/* .MaxDrawBuffers = */ 32,
	/* .MaxVertexUniformVectors = */ 128,
	/* .MaxVaryingVectors = */ 8,
	/* .MaxFragmentUniformVectors = */ 16,
	/* .MaxVertexOutputVectors = */ 16,
	/* .MaxFragmentInputVectors = */ 15,
	/* .MinProgramTexelOffset = */ -8,
	/* .MaxProgramTexelOffset = */ 7,
	/* .MaxClipDistances = */ 8,
	/* .MaxComputeWorkGroupCountX = */ 65535,
	/* .MaxComputeWorkGroupCountY = */ 65535,
	/* .MaxComputeWorkGroupCountZ = */ 65535,
	/* .MaxComputeWorkGroupSizeX = */ 1024,
	/* .MaxComputeWorkGroupSizeY = */ 1024,
	/* .MaxComputeWorkGroupSizeZ = */ 64,
	/* .MaxComputeUniformComponents = */ 1024,
	/* .MaxComputeTextureImageUnits = */ 16,
	/* .MaxComputeImageUniforms = */ 8,
	/* .MaxComputeAtomicCounters = */ 8,
	/* .MaxComputeAtomicCounterBuffers = */ 1,
	/* .MaxVaryingComponents = */ 60,
	/* .MaxVertexOutputComponents = */ 64,
	/* .MaxGeometryInputComponents = */ 64,
	/* .MaxGeometryOutputComponents = */ 128,
	/* .MaxFragmentInputComponents = */ 128,
	/* .MaxImageUnits = */ 8,
	/* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
	/* .MaxCombinedShaderOutputResources = */ 8,
	/* .MaxImageSamples = */ 0,
	/* .MaxVertexImageUniforms = */ 0,
	/* .MaxTessControlImageUniforms = */ 0,
	/* .MaxTessEvaluationImageUniforms = */ 0,
	/* .MaxGeometryImageUniforms = */ 0,
	/* .MaxFragmentImageUniforms = */ 8,
	/* .MaxCombinedImageUniforms = */ 8,
	/* .MaxGeometryTextureImageUnits = */ 16,
	/* .MaxGeometryOutputVertices = */ 256,
	/* .MaxGeometryTotalOutputComponents = */ 1024,
	/* .MaxGeometryUniformComponents = */ 1024,
	/* .MaxGeometryVaryingComponents = */ 64,
	/* .MaxTessControlInputComponents = */ 128,
	/* .MaxTessControlOutputComponents = */ 128,
	/* .MaxTessControlTextureImageUnits = */ 16,
	/* .MaxTessControlUniformComponents = */ 1024,
	/* .MaxTessControlTotalOutputComponents = */ 4096,
	/* .MaxTessEvaluationInputComponents = */ 128,
	/* .MaxTessEvaluationOutputComponents = */ 128,
	/* .MaxTessEvaluationTextureImageUnits = */ 16,
	/* .MaxTessEvaluationUniformComponents = */ 1024,
	/* .MaxTessPatchComponents = */ 120,
	/* .MaxPatchVertices = */ 32,
	/* .MaxTessGenLevel = */ 64,
	/* .MaxViewports = */ 16,
	/* .MaxVertexAtomicCounters = */ 0,
	/* .MaxTessControlAtomicCounters = */ 0,
	/* .MaxTessEvaluationAtomicCounters = */ 0,
	/* .MaxGeometryAtomicCounters = */ 0,
	/* .MaxFragmentAtomicCounters = */ 8,
	/* .MaxCombinedAtomicCounters = */ 8,
	/* .MaxAtomicCounterBindings = */ 1,
	/* .MaxVertexAtomicCounterBuffers = */ 0,
	/* .MaxTessControlAtomicCounterBuffers = */ 0,
	/* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
	/* .MaxGeometryAtomicCounterBuffers = */ 0,
	/* .MaxFragmentAtomicCounterBuffers = */ 1,
	/* .MaxCombinedAtomicCounterBuffers = */ 1,
	/* .MaxAtomicCounterBufferSize = */ 16384,
	/* .MaxTransformFeedbackBuffers = */ 4,
	/* .MaxTransformFeedbackInterleavedComponents = */ 64,
	/* .MaxCullDistances = */ 8,
	/* .MaxCombinedClipAndCullDistances = */ 8,
	/* .MaxSamples = */ 4,
	/* .maxMeshOutputVerticesNV = */ 0,
	/* .maxMeshOutputPrimitivesNV = */  0,
	/* .maxMeshWorkGroupSizeX_NV = */  0,
	/* .maxMeshWorkGroupSizeY_NV = */ 0,
	/* .maxMeshWorkGroupSizeZ_NV = */ 0,
	/* .maxTaskWorkGroupSizeX_NV = */ 0,
	/* .maxTaskWorkGroupSizeY_NV = */ 0,
	/* .maxTaskWorkGroupSizeZ_NV = */ 0,
	/* .maxMeshViewCountNV = */ 0,
	/* .maxDualSourceDrawBuffersEXT = */ 0,
	/* .limits = */ {
		/* .nonInductiveForLoops = */ 1,
		/* .whileLoops = */ 1,
		/* .doWhileLoops = */ 1,
		/* .generalUniformIndexing = */ 1,
		/* .generalAttributeMatrixVectorIndexing = */ 1,
		/* .generalVaryingIndexing = */ 1,
		/* .generalSamplerIndexing = */ 1,
		/* .generalVariableIndexing = */ 1,
		/* .generalConstantMatrixVectorIndexing = */ 1,
	}
};

bool glslangInit = false;

std::vector<u32> ShaderCompliation::CompileGLSLToSpirV(const ShaderStage& stage, const std::string& shaderData)
{
	if (!glslangInit)
	{
		glslang::InitializeProcess();
		glslangInit = true;
	}

	const char* glslRaw = shaderData.data();
	EShLanguage ShaderType = GetShaderStage(stage);
	glslang::TShader Shader(ShaderType);
	Shader.setStrings(&glslRaw, 1);

	int ClientInputSemanticsVersion = 450; // maps to, say, #define VULKAN 100
	glslang::EShTargetClientVersion VulkanClientVersion = glslang::EShTargetVulkan_1_1;
	glslang::EShTargetLanguageVersion TargetVersion = glslang::EShTargetSpv_1_3;

	Shader.setEnvInput(glslang::EShSourceGlsl, ShaderType, glslang::EShClientVulkan, ClientInputSemanticsVersion);
	Shader.setEnvClient(glslang::EShClientVulkan, VulkanClientVersion);
	Shader.setEnvTarget(glslang::EShTargetSpv, TargetVersion);

	TBuiltInResource Resources;
	Resources = DefaultTBuiltInResource;
	EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);
	const int DefaultVersion = 100;

	DirStackFileIncluder Includer;

	std::string PreprocessedGLSL;
	if (!Shader.preprocess(&Resources, DefaultVersion, ENoProfile, false, false, messages, &PreprocessedGLSL, Includer))
	{
		IS_CORE_ERROR("GLSL preprocess Failed.");
		IS_CORE_ERROR("{0}", Shader.getInfoLog());
		IS_CORE_ERROR("{0}", Shader.getInfoDebugLog());
	}

	if (!Shader.parse(&Resources, 100, false, messages))
	{
		IS_CORE_ERROR("GLSL parse Failed.");
		IS_CORE_ERROR("{0}", Shader.getInfoLog());
		IS_CORE_ERROR("{0}", Shader.getInfoDebugLog());
	}

	glslang::TProgram Program;
	Program.addShader(&Shader);
	if (!Program.link(messages))
	{
		IS_CORE_ERROR("GLSL Linking Failed.");
		IS_CORE_ERROR("{0}", Shader.getInfoLog());
		IS_CORE_ERROR("{0}", Shader.getInfoDebugLog());
	}

	std::vector<uint32_t> SpirV;
	spv::SpvBuildLogger logger;
	glslang::GlslangToSpv(*Program.getIntermediate(ShaderType), SpirV, &logger);

	return SpirV;
}
