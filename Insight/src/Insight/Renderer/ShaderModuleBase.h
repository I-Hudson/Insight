#pragma once

#include "Insight/Core.h"

#ifdef IS_VULKAN
#include "Platform/Vulkan/VulkanHeader.h"
#include <shaderc/shaderc.hpp>
#endif

namespace Platform
{
	class Device;
}

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

			Sampler2D,
			Push_Constant,
			UniformDynamic
		};

		enum ShaderType
		{
			None = 0,
			VertexShader = 1,
			GeometryShader = 2,
			FragmentShader = 3,
			ComputeShader = 4
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
			ShaderAttributeType Type;
			int Binding;
			int Size;
			bool IsDynamic;

			ShaderUniformBlock()
				: Size(0)
				, IsDynamic(false)
			{ }
		};

		struct ParsedShadeData
		{
			ShaderAttributeBlock InAttri;
			ShaderAttributeBlock OutAttri;
			std::vector<ShaderUniformBlock> UniformBlocks;
			ShaderType ShaderType;

			std::string RawSource;

			ParsedShadeData()
			{
				InAttri.Size = 0;
				OutAttri.Size = 0;
			}
		};

		class IS_API ShaderModuleBase
		{
		public:
#ifdef IS_VULKAN
			ShaderModuleBase(const Platform::Device* device, const std::string& filepath);
			ShaderModuleBase(const Platform::Device* device, const std::string& filepath, const ShaderType& type);
#endif
			ShaderModuleBase(const std::string& filepath);
			ShaderModuleBase(const std::string& filepath, const ShaderType& type); 
			~ShaderModuleBase();

			void Destroy();

			const ParsedShadeData& GetData() const { return m_shaderData; }
			ShaderType GetShaderType() const;

#ifdef IS_VULKAN
			VkShaderStageFlagBits GetShaderStageBit() const;
			const VkShaderModule GetModule() const { return m_shaderModule; }
			static VkDescriptorType GetShaderDescriptorType(const ShaderAttributeType& type);
			std::vector<VkDescriptorSetLayoutBinding> GetPipelineLayoutCreateInfo();
			std::vector<VkVertexInputBindingDescription> GetVertexBindingDesc();
			std::vector<VkVertexInputAttributeDescription> GetAttributes();
			std::vector<VkPushConstantRange> GetPushContants();
#endif

		private:
			std::string GetSource(const std::string& filePath);
#ifdef IS_VULKAN
			VkFormat GetVertexFormat(const ShaderAttributeType& type);
			shaderc_shader_kind GetShaderStage(const std::string& stage);
			std::string PreprocessShader(const std::string& source_name, shaderc_shader_kind kind, const std::string& source, bool optimize = false);
			std::vector<uint32_t> ComplieToRaw(const std::string& source_name, shaderc_shader_kind kind, const std::string& source, bool optimize = false);
			const std::vector<uint32_t> CompileGLSL(const std::string& filePath);
#endif

			ShaderType GetShaderTypeFromPath(const std::string& string);

			std::string GetFilePath(const std::string& filePath);
			std::string GetSuffix(const std::string& name);

		private:
			ShaderType m_type;
			ParsedShadeData m_shaderData;

#ifdef IS_VULKAN
			const Platform::Device* m_device;
			VkShaderModule m_shaderModule;
#endif
		};
	}
}