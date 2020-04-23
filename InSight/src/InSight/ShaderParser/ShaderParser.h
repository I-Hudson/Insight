#pragma once

#include "Insight/Core.h"

#include <string>
#include <vector>

namespace Insight
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

	struct ShaderData
	{
		ShaderAttributeBlock InAttri;
		ShaderAttributeBlock OutAttri;
		std::vector<ShaderUniformBlock>UniformBlocks;
	};

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

	class IS_API ShaderParser
	{
	public:

		static ShaderData ParseShader(const std::string& filePath);

	private:

		static void GetAttribute(const KeywordFind& keyword, ShaderData& data);
		static void CreateUniformBlock(std::string& line, ShaderData& data);
		static void GetUniformStruct(const std::string line, ShaderData& data);

		static KeywordFind FindKeyword(std::string& line);

		static ShaderAttributeType GetType(const std::string& line);
		static std::string GetName(const std::string& line, const bool& removeLastCharacter);

		static void GetUniformSize(ShaderUniformBlock& data);
		static int GetShaderAttributeTypeSize(const ShaderAttributeType& type);

		static std::vector<const char*> m_keywords;
		static std::vector<const char*> m_dataTypes;
		static bool m_recordUniform;
	};
}