#pragma once

#include "Insight/Core.h"

#include "Insight/Renderer/Lowlevel/ShaderModuleBase.h"

#include <string>
#include <vector>

namespace Insight
{
	namespace Render
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

		class IS_API ShaderParser
		{
		public:
			static ParsedShadeData ParseShader(const std::string& filePath, const Render::ShaderType shaderType);

		private:

			static void GetAttribute(const KeywordFind& keyword, ParsedShadeData& data);
			static void CreateUniformBlock(std::string& line, ParsedShadeData& data);
			static void GetUniformStruct(const std::string line, ParsedShadeData& data);

			static KeywordFind FindKeyword(std::string& line);

			static ShaderAttributeType GetType(const std::string& line);
			static std::string GetName(const std::string& line, const bool& removeLastCharacter);

			static void GetUniformSize(ShaderUniformBlock& data);
			static int GetShaderAttributeTypeSize(const ShaderAttributeType& type);

			static void PostProcess(ParsedShadeData& data);

			static std::vector<const char*> m_keywords;
			static std::vector<const char*> m_dataTypes;
			static bool m_recordUniform;
		};
	}
}