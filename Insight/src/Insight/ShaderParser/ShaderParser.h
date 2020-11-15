#pragma once

#include "Insight/Core.h"
#include "Platform/Vulkan/VulkanHeader.h"

namespace Insight
{
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
	};

	enum class ShaderUniformBlockType
	{
		UniformBuffer,
		UniformBufferDynamic,
		PushConstant,
		StorageBuffer,
		StorageBufferDynamic
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

		UniformBufferBlock()
			: IsDynamic(false)
		{ }

		virtual VkDescriptorType GetVulkanType() override
		{
			return IsDynamic ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
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

		VkShaderStageFlags GetVulkanShaderStage()
		{
			switch (ShaderType)
			{
				case Insight::VertexShader: return VK_SHADER_STAGE_VERTEX_BIT;
				case Insight::GeometryShader: return VK_SHADER_STAGE_GEOMETRY_BIT;
				case Insight::FragmentShader: return VK_SHADER_STAGE_FRAGMENT_BIT;
				case Insight::ComputeShader: return VK_SHADER_STAGE_COMPUTE_BIT;
			}
			return VK_SHADER_STAGE_ALL;
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

		static ShaderType ExtensionToShaderType(const std::string& fileName);

		static std::vector<const char*> m_keywords;
		static std::vector<const char*> m_dataTypes;
		static bool m_recordUniform;
		static UniformBlock* m_recordUniformBlock;
	};
}