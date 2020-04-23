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

						m_recordUniform = true;
					}
					IS_CORE_WARN("Pos: {0}, Word: {1}, Length {2}", k.Loc, k.Keyword, k.KeywordLength);
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

		std::string name;
		for (auto it = line.rbegin(); it != line.rend(); ++it)
		{
			if (isspace(*it))
			{
				break;
			}
			else
			{
				name += *it;
			}
		}
		name.erase(name.begin(), name.begin() + 1);

		// Swap character starting from two 
		// corners 
		for (int i = 0; i < name.length() / 2; i++)
		{
			std::swap(name[i], name[name.length() - i - 1]);
		}

		attri.Location = std::stoi(attiLocation);
		attri.Name = name;
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

	void ShaderParser::GetUniformStruct(const std::string line, ShaderData& data)
	{
		ShaderAttributeType type = GetType(line);
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
				return (ShaderAttributeType)index;
			}
			++index;
		}
		return ShaderAttributeType::None;
	}
}