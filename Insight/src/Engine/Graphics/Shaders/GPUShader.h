#pragma once

#include "Engine/Graphics/GPUResource.h"
#include "Engine/Graphics/Enums.h"

enum ShaderStageInput
{
	RawData,
	FilePath
};

/// <summary>
/// Define the bindings within a single shader stage. This should contain each input and output
/// for all the bindings. For each binding needs a Name, binding, size
/// </summary>
struct ShaderStageBindings
{
	std::string Name;
	U32 Binding;
	U32 Size;
};

/// <summary>
/// Define a single uniform within a shader stage. This includes the name, set, binding, overall size 
/// and all the bindings for the uniform. The bindings are optional.
/// </summary>
struct ShaderStageUniform
{
	std::string Name;
	U32 Set;
	U32 Binding;
	U32 Size;
	std::vector<ShaderStageBindings> Bindings;
};

/// <summary>
/// Define a single stage within a shader. This should define a stage not store any resources.
/// </summary>
class GPUShaderStage
{
public:
	GPUShaderStage() = delete;
	GPUShaderStage(const ShaderStages& stage, const std::string& str, const ShaderStageInput& input);

	bool IsValid() const { return m_parsed; }

	/// <summary>
	/// Parse this shader stage to get all the information from the raw data.
	/// </summary>
	void Parse();

	const ShaderStages& GetStage() const { return m_stage; }

	// Compile this shader stage for the graphic's API format.
	// [ReturnValue] Compile();
	// Return the bindings in the graphic's API format.
	// [ReturnValue] GetBindings();

protected:
	ShaderStages m_stage;

	bool m_parsed;

	/// <summary>
	/// Load the shader data loaded from file as a string.
	/// </summary>
	std::string m_loadedShaderString;

	/// <summary>
	/// Store the raw shader data. This is used for runtime compilation.
	/// </summary>
	std::vector<U32> m_rawData;

	/// <summary>
	/// Shader stage bindings.
	/// </summary>
	std::vector<ShaderStageBindings> m_inputs;

	/// <summary>
	/// Shader stage uniforms.
	/// </summary>
	std::vector<ShaderStageUniform> m_uniforms;
};

/// <summary>
/// Define a single shader. This includes all the stage needed for a complete pipeline.
/// </summary>
class GPUShader : public GPUResource
{
public:
	static GPUShader* New();

	GPUShader();
	virtual ~GPUShader();

	/// <summary>
	/// Compile this shader into the format needed for the graphic's API being used.
	/// </summary>
	virtual void Compile() = 0;

	/// <summary>
	/// Set a shader stage.
	/// </summary>
	/// <param name="shaderStage"></param>
	void SetStage(const GPUShaderStage& shaderStage);
	/// <summary>
	/// Set a shader stage.
	/// </summary>
	/// <param name="shaderStage"></param>
	void SetStage(const ShaderStages& stage, const std::string& str, const ShaderStageInput& input);

	/// <summary>
	/// Get a single stage from this shader.
	/// </summary>
	/// <param name="stage"></param>
	/// <returns></returns>
	const GPUShaderStage& GetStage(const ShaderStages& stage) { return m_stages.at((U32)stage); }
	
	// [GPUResource]
	virtual ResourceType GetResourceType() const override { return ResourceType::Shader; }

protected:
	std::array<GPUShaderStage, (size_t)ShaderStages::Count> m_stages;
};

