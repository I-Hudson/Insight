#pragma once

#include "Insight/Core.h"

#include "Insight/Renderer/Vulkan.h"

#include <shaderc/shaderc.hpp>

namespace Insight
{
	namespace Render
	{
		enum class ShaderAttributeType
		{
			None,

			Int,
			Float,

			Vec2,
			Vec3,
			Vec4,

			Mat2,
			Mat3,
			Mat4,

			Sampler2D
		};

		struct ShaderAttributes
		{
			int Location;
			ShaderAttributeType Type;
			std::string Name;
			int Stride;
			int Size;
		};

		struct ShaderUniform
		{
			ShaderAttributeType Type;
			std::string Name;
		};

		struct ShaderAttributeBlock
		{
			std::vector<ShaderAttributes> Attributes;
			int Size;
		};

		struct ShaderUniformBlock
		{
			std::vector<ShaderUniform> Uniforms;
			std::string Name;
			int Binding;
			int Size;
		};

		struct ParsedShadeData
		{
			ShaderAttributeBlock InAttri;
			ShaderAttributeBlock OutAttri;
			std::vector<ShaderUniformBlock>UniformBlocks;

			ParsedShadeData()
			{
				InAttri.Size = 0;
				OutAttri.Size = 0;
			}
		};

		enum ShaderType
		{
			None = 0,
			Vertex,
			Geometry,
			Fragment,
			Compute
		};

		class Device;

		class IS_API ShaderModuleBase
		{
		public:
			ShaderModuleBase(const Device* device, const std::string& filepath, const ShaderType& type);
			~ShaderModuleBase();

			void Destroy();

			const VkShaderModule GetModule() const { return m_shaderModule; }
			const ParsedShadeData& GetData() const { return m_shaderData; }
			VkShaderStageFlagBits GetShaderStageBit() const;
			ShaderType GetShaderType() const;

			VkPipelineVertexInputStateCreateInfo GetVertexInputCreateInfo();
			std::vector<VkDescriptorSetLayoutBinding> GetPipelineLayoutCreateInfo();

		private:
			VkVertexInputBindingDescription GetVertexBindingDesc();
			std::vector<VkVertexInputAttributeDescription> GetAttributes();
			VkFormat GetVertexFormat(const ShaderAttributeType& type);

			const std::vector<uint32_t> CompileGLSL(const std::string& filePath);

			std::string PreprocessShader(const std::string& source_name, shaderc_shader_kind kind, const std::string& source, bool optimize = false);
			std::vector<uint32_t> ComplieToRaw(const std::string& source_name, shaderc_shader_kind kind, const std::string& source, bool optimize = false);


			std::string GetFilePath(const std::string& filePath);
			std::string GetSuffix(const std::string& name);
			shaderc_shader_kind GetShaderStage(const std::string& stage);

		private:
			const Device* m_device;
			VkShaderModule m_shaderModule;
			ShaderType m_type;
			ParsedShadeData m_shaderData;
		};
	}
}