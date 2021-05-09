#pragma once

#include "Engine/Graphics/GPUResource.h"
#include "Engine/Graphics/Enums.h"
#include "spirv_cross/spirv_glsl.hpp"

namespace Insight::Graphics
{
	class GPURenderGraphPass;

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
		spirv_cross::SPIRType Type;
		U32 VecSize;
		U32 ByteSize;
		U32 Stride;
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
	class IS_API GPUShaderStage
	{
	public:
		GPUShaderStage() = delete;
		GPUShaderStage(const ShaderStage& stage, const std::string& str, const ShaderStageInput& input);

		bool IsValid() const { return m_parsed; }

		/// <summary>
		/// Parse this shader stage to get all the information from the raw data.
		/// </summary>
		void Parse();

		const ShaderStage& GetStage() const { return m_stage; }
		const std::vector<ShaderStageBindings>& GetInputs() const { return m_inputs; }
		const std::vector<ShaderStageUniform>& GetUniforms() const { return m_uniforms; }

		const U32 GetInputsSize() const;
		const std::vector<U32>& GetRawData() const { return m_rawData; }

		// Compile this shader stage for the graphic's API format.
		// [ReturnValue] Compile();
		// Return the bindings in the graphic's API format.
		// [ReturnValue] GetBindings();

	protected:
		ShaderStage m_stage;

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

	private:
		U32 GetStride();
	};

	/// <summary>
	/// Define a single shader. This includes all the stage needed for a complete pipeline.
	/// </summary>
	class IS_API GPUShader : public GPUResource
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
		void SetStage(const ShaderStage& stage, const std::string& str, const ShaderStageInput& input);

		/// <summary>
		/// Get a single stage from this shader.
		/// </summary>
		/// <param name="stage"></param>
		/// <returns></returns>
		GPUShaderStage& GetStage(const ShaderStage& stage) { return m_stages.at((U32)stage); }

		/// <summary>
		/// Release all the gpu resources before the program quits.
		/// </summary>
		virtual void ReleaseGPUResoucesEarly() = 0;

		// [GPUResource]
		virtual ResourceType GetResourceType() const override { return ResourceType::Shader; }
		virtual ObjectType GetObjectType() const override { return ObjectType::Other; }

	protected:
		std::array<GPUShaderStage, (size_t)ShaderStage::Count> m_stages;
	};

	class GPUPipeline : public GPUResource
	{
	public:
		static GPUPipeline* New();

		GPUPipeline() { }
		virtual ~GPUPipeline() { }

		virtual void SetShader(Graphics::GPUShader* shader) = 0;
		virtual void BuildPipeline(GPURenderGraphPass* graphPass) = 0;

		// [GPUResource]
		virtual ResourceType GetResourceType() const override { return ResourceType::PipelineState; }
		virtual ObjectType GetObjectType() const override { return ObjectType::Other; }

	protected:
		GPUShader* m_shader;
	};
}