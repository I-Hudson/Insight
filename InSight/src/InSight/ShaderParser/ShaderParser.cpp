#include "ispch.h"
#include "ShaderParser.h"

#include "Insight/InsightAlias.h"

#include "Insight/Log.h"

namespace Insight
{
	std::vector<const char*> ShaderParser::m_keywords
	{
		"in", "uniform", "out"
	};

	std::vector<const char*> ShaderParser::m_dataTypes
	{
		"int", "float",
		"vec2", "vec3", "vec4",
		"mat2", "mat3", "mat4",
		"sampler2D"
	};

	bool ShaderParser::m_recordUniform = false;

	ShaderData Insight::ShaderParser::ParseShader(const std::string& filePath)
	{
		ShaderData data;
		std::ifstream cFile(filePath);
		if (cFile.is_open())
		{
			std::string line;
			while (std::getline(cFile, line))
			{
				if (line[0] == '/' || line.empty())
				{
					continue;
				}

				IS_CORE_INFO("{0}", line.c_str());

				KeywordFind k = FindKeyword(line);
				if (k != KeywordFind())
				{
					if (k.Keyword == "in" || k.Keyword == "out")
					{
						GetAttribute(k, data);
					}
					else if (k.Keyword == "uniform")
					{
						CreateUniformBlock(line, data);
						m_recordUniform = true;
					}
				}

				if (m_recordUniform)
				{
					GetUniformStruct(line, data);
				}
			}

			cFile.close();
		}

		return data;
	}

	void ShaderParser::GetAttribute(const KeywordFind& keyword, ShaderData& data)
	{
		ShaderAttributes attri;
		std::string line = keyword.Line;

		Size attiLocPos = keyword.Line.find('=');
		std::string attiLocation = "";
		for (std::string::iterator it = line.begin() + attiLocPos; it != line.end(); ++it)
		{
			int ascii = *it;
			if (ascii >= '0' && ascii <= '9')
			{
				attiLocation += *it;
			}
			
			if (*it == ')')
			{
				break;
			}
		}

		attri.Location = std::stoi(attiLocation);
		attri.Name = GetName(line, true);
		attri.Type = GetType(line);

		if (keyword.Keyword == "in")
		{
			data.InAttri.Attributes.push_back(attri);
		}
		else
		{
			data.OutAttri.Attributes.push_back(attri);
		}
	}

	void ShaderParser::CreateUniformBlock(std::string& line, ShaderData& data)
	{
		ShaderUniformBlock uniformBlock;

		std::string sValue = "binding = ";
		Size sLoc = line.find(sValue);
		sLoc += sValue.length();

		std::string returnValue;
		for (auto it = line.begin() + sLoc; it != line.end(); ++it)
		{
			char c = *it;
			if (c >= '0' && c <= '9')
			{
				returnValue += c;
			}
			else
			{
				break;
			}
		}
		uniformBlock.Binding = std::stoi(returnValue);

		uniformBlock.Name = GetName(line, false);

		data.UniformBlocks.push_back(uniformBlock);
	}

	void ShaderParser::GetUniformStruct(const std::string line, ShaderData& data)
	{
		ShaderAttributeType type = GetType(line);

		if (type != ShaderAttributeType::None)
		{
			ShaderUniform uniform;
			uniform.Name = GetName(line, true);
			uniform.Type = type;

			data.UniformBlocks[data.UniformBlocks.size() - 1].Uniforms.push_back(uniform);
		}

		if (line.find('}') != line.npos)
		{
			GetUniformSize(data.UniformBlocks[data.UniformBlocks.size() - 1]);
			m_recordUniform = false;
		}
	}

	KeywordFind ShaderParser::FindKeyword(std::string& line)
	{
		KeywordFind key;

		for (std::vector<const char*>::iterator it = m_keywords.begin(); it != m_keywords.end(); ++it)
		{
			std::string findKey = " ";
			findKey += *it;
			findKey += " ";
			Size pos = line.find(findKey.c_str());
			
			if (pos + 1 > 0 && pos - 1 < line.length())
			{
				if (pos != line.npos)
				{
					key.Loc = pos;
					key.KeywordLength = strlen(*it);
					key.Keyword = *it;
					key.Line = line;
					break;
				}
			}
		}
		return key;
	}

	ShaderAttributeType ShaderParser::GetType(const std::string& line)
	{
		int index = 0;
		for (std::vector<const char*>::iterator it = m_dataTypes.begin(); it != m_dataTypes.end(); ++it)
		{
			if (line.find(*it) != line.npos)
			{
				return (ShaderAttributeType)(index + 1);
			}
			++index;
		}
		return ShaderAttributeType::None;
	}

	std::string ShaderParser::GetName(const std::string& line, const bool& removeLastCharacter)
	{
		std::string sValue;
		for (auto it = line.rbegin(); it != line.rend(); ++it)
		{
			if (isspace(*it))
			{
				break;
			}
			else
			{
				sValue += *it;
			}
		}

		if (removeLastCharacter)
		{
			sValue.erase(sValue.begin(), sValue.begin() + 1);
		}

		// Swap character starting from two 
		// corners 
		for (int i = 0; i < sValue.length() / 2; i++)
		{
			std::swap(sValue[i], sValue[sValue.length() - i - 1]);
		}

		return sValue;
	}
	
	void ShaderParser::GetUniformSize(ShaderUniformBlock& data)
	{
		int totalSize = 0;
		for (auto it = data.Uniforms.begin(); it != data.Uniforms.end(); ++it)
		{
			totalSize += GetShaderAttributeTypeSize((*it).Type);
		}
		data.Size = totalSize;
	}

	int ShaderParser::GetShaderAttributeTypeSize(const ShaderAttributeType& type)
	{
		switch(type)
		{
			case ShaderAttributeType::Int: return 4;
			case ShaderAttributeType::Float: return 4;

			case ShaderAttributeType::Vec2: return 4 * 2;
			case ShaderAttributeType::Vec3: return 4 * 3;
			case ShaderAttributeType::Vec4: return 4 * 4;

			case ShaderAttributeType::Mat2: return 4 * 2 * 2;
			case ShaderAttributeType::Mat3: return 4 * 3 * 3;
			case ShaderAttributeType::Mat4: return 4 * 4 * 4;
		}
		return 0;
	}
}