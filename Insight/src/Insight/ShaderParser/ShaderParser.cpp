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
		"int", "float", "double",
		"vec2", "vec3", "vec4",
		"ivec2", "ivec3", "ivec4",
		"mat2", "mat3", "mat4",
		"sampler2D"
	};

	bool ShaderParser::m_recordUniform = false;
	UniformBlock* ShaderParser::m_recordUniformBlock;

	ParsedShadeData ShaderParser::ParseShader(const std::string& filePath, const ShaderType shaderType)
	{
		ParsedShadeData data;
		data.ShaderType = shaderType;

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

				bool startUniform = false;

				KeywordFind k = FindKeyword(line);
				if (k != KeywordFind())
				{
					if (k.Keyword == "in")
					{
						GetAttribute(k, data);
					}
					else if (k.Keyword == "out")
					{
						GetAttribute(k, data);
					}
					else if (k.Keyword == "uniform")
					{
						if (line.find("push_constant") != std::string::npos)
						{
							m_recordUniformBlock = CreatePushConstatnt(line, data);
							startUniform = true;
							m_recordUniform = true;
						}
						else
						{
							m_recordUniformBlock = CreateUniformBlock(line, data);
							startUniform = true;
							m_recordUniform = true;
						}
					}
				}

				if (m_recordUniform)
				{
					GetUniformStruct(line, data, *m_recordUniformBlock, startUniform);
				}
			}

			PostProcess(data);

			cFile.close();
		}

		return data;
	}

	ParsedShadeData ShaderParser::ParseShader(const std::string& filePath)
	{
		return ParseShader(filePath, ExtensionToShaderType(filePath));
	}

	void ShaderParser::GetAttribute(const KeywordFind& keyword, ParsedShadeData& data)
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
		attri.Size = GetShaderAttributeTypeSize(attri.Type);

		if (keyword.Keyword == "in")
		{
			data.InAttri.Size += attri.Size;
			data.InAttri.Attributes.push_back(attri);
		}
		else
		{
			data.OutAttri.Size += attri.Size;
			data.OutAttri.Attributes.push_back(attri);
		}
	}

	UniformBlock* ShaderParser::CreateUniformBlock(std::string& line, ParsedShadeData& data)
	{
		UniformBufferBlock uniformBlock;

		uniformBlock.Type = ShaderUniformBlockType::UniformBuffer;
		if (line.find("sampler2D") != std::string::npos)
		{
			uniformBlock.Type = ShaderUniformBlockType::Sampler2D;
		}

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

		if (!returnValue.empty())
		{
			uniformBlock.Binding = std::stoi(returnValue);
		}

		uniformBlock.Name = GetName(line, false);

		sLoc = line.find("//");
		sLoc += 2;
		if (line[sLoc] == '#')
		{
			if (line.substr(sLoc + 1) == "dynamic")
			{
				uniformBlock.IsDynamic = true;
				uniformBlock.Type = ShaderUniformBlockType::UniformBufferDynamic;
			}
		}

		data.UniformBlocks.push_back(uniformBlock);

		return &data.UniformBlocks[data.UniformBlocks.size() - 1];
	}

	UniformBlock* ShaderParser::CreatePushConstatnt(std::string& line, ParsedShadeData& data)
	{
		PushConstantBlock pushConstantBlock;
		pushConstantBlock.Type = ShaderUniformBlockType::PushConstant;

		pushConstantBlock.Name = GetName(line, false);

		if (data.PushConstants.size() > 0)
		{
			int offset = 0;
			for (auto& pc : data.PushConstants)
			{
				offset += pc.Size;
			}
			pushConstantBlock.Offset = offset;
		}

		data.PushConstants.push_back(pushConstantBlock);

		return &data.PushConstants[data.PushConstants.size() - 1];
	}

	void ShaderParser::GetUniformStruct(const std::string line, ParsedShadeData& data, UniformBlock& uniformBlock, const bool& startUniform)
	{
		ShaderAttributeType type = GetType(line);

		if (type != ShaderAttributeType::None)
		{
			ShaderUniform uniform;
			uniform.Name = GetName(line, true);
			uniform.Type = type;

			uniformBlock.Uniforms.push_back(uniform);
		}

		if (startUniform && line.find(';') != line.npos)
		{
			m_recordUniform = false;
			return;
		}

		if (line.find('}') != line.npos)
		{
			GetUniformSize(uniformBlock);
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
			int pos = static_cast<int>(line.find(findKey.c_str()));

			if (pos + 1 > 0 && pos - 1 < line.length())
			{
				if (pos != line.npos)
				{
					key.Loc = pos;
					key.KeywordLength = static_cast<int>(strlen(*it));
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
		bool wordStart = false;
		for (auto it = line.rbegin(); it != line.rend(); ++it)
		{
			if (isspace(*it) || *it == '{')
			{
				if (wordStart)
				{
					break;
				}
				continue;
			}
			else
			{
				wordStart = true;
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

	void ShaderParser::GetUniformSize(UniformBlock& data)
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
		switch (type)
		{
			case ShaderAttributeType::Int: return 4;
			case ShaderAttributeType::Float: return 4;

			case ShaderAttributeType::Vec2: return 4 * 2;
			case ShaderAttributeType::Vec3: return 4 * 3;
			case ShaderAttributeType::Vec4: return 4 * 4;

			case ShaderAttributeType::IVec2: return 4 * 2;
			case ShaderAttributeType::IVec3: return 4 * 3;
			case ShaderAttributeType::IVec4: return 4 * 4;

			case ShaderAttributeType::Mat2: return 4 * 2 * 2;
			case ShaderAttributeType::Mat3: return 4 * 3 * 3;
			case ShaderAttributeType::Mat4: return 4 * 4 * 4;
		}
		return 0;
	}

	void ShaderParser::PostProcess(ParsedShadeData& data)
	{
		int stride = 0;
		for (auto it = data.InAttri.Attributes.begin(); it != data.InAttri.Attributes.end(); ++it)
		{
			(*it).Stride = stride;
			stride += (*it).Size;
		}
	}

	ShaderType ShaderParser::ExtensionToShaderType(const std::string& fileName)
	{
		std::string extension = fileName.substr(fileName.find_last_of('.') + 1);
		std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c)
					   {
						   return std::tolower(c);
					   });

		if (extension == "vert")
		{
			return ShaderType::VertexShader;
		}
		if (extension == "geom")
		{
			return ShaderType::GeometryShader;
		}
		if (extension == "frag")
		{
			return ShaderType::FragmentShader;
		}
		if (extension == "comp")
		{
			return ShaderType::ComputeShader;
		}
		return ShaderType::None;
	}
}