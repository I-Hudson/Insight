#pragma once

#include "Engine/Core/Core.h"
#include "Platform/Vulkan/VulkanHeader.h"

	struct KeywordFind
	{
		int Loc;
		int KeywordLength;
		std::string Keyword;
		std::string Line;

		KeywordFind()
		{
			Loc = 0;
			KeywordLength = 0;
			Keyword = std::string("");
			Line = std::string("");
		}

		bool operator!=(const KeywordFind& k)
		{
			return Loc != k.Loc || KeywordLength != k.KeywordLength || Keyword != k.Keyword || Line != k.Line;
		}
	};

#define SHADERATTRIBUTETYPE_TO_STRING(x) return ##x;
	enum class ShaderAttributeType
	{
		None,

		Int,
		Float,
		Double,

		Vec2,
		Vec3,
		Vec4,

		IVec2,
		IVec3,
		IVec4,

		Mat2,
		Mat3,
		Mat4,

		Sampler2D,
	};

	enum class ShaderUniformBlockType
	{
		Sampler2D,
		UniformBuffer,
		UniformBufferDynamic,
		PushConstant,
		StorageBuffer,
		StorageBufferDynamic
	};

	enum class ShaderType
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

	struct UniformBlock
	{
		std::vector<ShaderUniform> Uniforms;
		std::string Name;
		int Binding;
		int Size;
		ShaderUniformBlockType Type;

		UniformBlock()
			: Binding(0)
			, Size(0)
		{ }
		virtual VkDescriptorType GetVulkanType() { return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; }
	};

	struct UniformBufferBlock : public UniformBlock
	{
		bool IsDynamic;
		int Set;

		UniformBufferBlock()
			: IsDynamic(false)
			, Set(0)
		{ }

		virtual VkDescriptorType GetVulkanType() override
		{
			if (IsDynamic)
			{
				return Type == ShaderUniformBlockType::UniformBufferDynamic ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
			}
			else
			{
				switch (Type)
				{
					case ShaderUniformBlockType::Sampler2D: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					case ShaderUniformBlockType::UniformBuffer: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					case ShaderUniformBlockType::UniformBufferDynamic: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
					case ShaderUniformBlockType::StorageBuffer: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
					case ShaderUniformBlockType::StorageBufferDynamic: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
				}
			}
			return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		}
	};

	struct PushConstantBlock : public UniformBlock
	{
		int Offset;

		PushConstantBlock()
			: Offset(0)
		{ }
	};

	struct ParsedShadeData
	{
		ShaderAttributeBlock InAttri;
		ShaderAttributeBlock OutAttri;
		std::vector<UniformBufferBlock> UniformBlocks;
		std::vector<PushConstantBlock> PushConstants;
		ShaderType ShaderType;

		std::string RawSource;

		ParsedShadeData()
		{
			InAttri.Size = 0;
			OutAttri.Size = 0;
		}

		VkShaderStageFlagBits GetVulkanShaderStage()
		{
			switch (ShaderType)
			{
			case ShaderType::VertexShader: return VK_SHADER_STAGE_VERTEX_BIT;
			case ShaderType::GeometryShader: return VK_SHADER_STAGE_GEOMETRY_BIT;
			case ShaderType::FragmentShader: return VK_SHADER_STAGE_FRAGMENT_BIT;
			case ShaderType::ComputeShader: return VK_SHADER_STAGE_COMPUTE_BIT;
			}
			return VK_SHADER_STAGE_ALL;
		}

		VkFormat GetVulkanFormatType(ShaderAttributeType& type)
		{
			switch (type)
			{
			case ShaderAttributeType::Int: return VK_FORMAT_R32_SINT;
			case ShaderAttributeType::Float: return VK_FORMAT_R32_SFLOAT;
			case ShaderAttributeType::Double: return VK_FORMAT_R32_SFLOAT;
			case ShaderAttributeType::Vec2: return VK_FORMAT_R32G32_SFLOAT;
			case ShaderAttributeType::Vec3: return VK_FORMAT_R32G32B32_SFLOAT;
			case ShaderAttributeType::Vec4: return VK_FORMAT_R32G32B32A32_SFLOAT;
			case ShaderAttributeType::IVec2: return VK_FORMAT_R32G32_SINT;
			case ShaderAttributeType::IVec3: return VK_FORMAT_R32G32B32_SINT;
			case ShaderAttributeType::IVec4: return VK_FORMAT_R32G32B32A32_SINT;
			}
			return VK_FORMAT_R32G32B32A32_SFLOAT;
		}


		std::vector<VkVertexInputBindingDescription> VertexInputBinding;
		std::vector<VkVertexInputAttributeDescription> VertexInputAttribute;
		VkPipelineVertexInputStateCreateInfo GetVertexInputState()
		{
			if (InAttri.Attributes.size() == 0)
			{
				return vks::initializers::pipelineVertexInputStateCreateInfo();
			}

			VkVertexInputBindingDescription inputBinding;
			inputBinding.binding = 0;
			inputBinding.stride = InAttri.Size;
			inputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			VertexInputBinding.push_back(inputBinding);

			int location = 0;
			for (auto& in : InAttri.Attributes)
			{
				VertexInputAttribute.push_back(vks::initializers::vertexInputAttributeDescription(0, in.Location, GetVulkanFormatType(in.Type), in.Stride));
			}
			return vks::initializers::pipelineVertexInputStateCreateInfo(VertexInputBinding, VertexInputAttribute);
		}
	};

	class IS_API ShaderParser
	{
	public:
		static ParsedShadeData ParseShader(const std::string& filePath, const ShaderType shaderType);
		static ParsedShadeData ParseShader(const std::string& filePath);

	private:

		static void GetAttribute(const KeywordFind& keyword, ParsedShadeData& data);

		static UniformBlock* CreateUniformBlock(std::string& line, ParsedShadeData& data);
		static UniformBlock* CreatePushConstatnt(std::string& line, ParsedShadeData& data);

		static void GetUniformStruct(const std::string line, ParsedShadeData& data, UniformBlock& uniformBlock, const bool& startUniform);

		static KeywordFind FindKeyword(std::string& line);

		static ShaderAttributeType GetType(const std::string& line);
		static std::string GetName(const std::string& line, const bool& removeLastCharacter);

		static void GetUniformSize(UniformBlock& data);
		static int GetShaderAttributeTypeSize(const ShaderAttributeType& type);

		static void PostProcess(ParsedShadeData& data);

		static int GetIntValue(const std::string& stringKey, const std::string& line);
		static ShaderType ExtensionToShaderType(const std::string& fileName);

		static std::vector<const char*> m_keywords;
		static std::vector<const char*> m_dataTypes;
		static bool m_recordUniform;
		static UniformBlock* m_recordUniformBlock;
	};