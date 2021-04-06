#include "ispch.h"
#include "Engine/Config/Config.h"

#include <filesystem>

void Config::Parse(const std::string& filePath)
{
	std::ifstream cFile(filePath);
	if (cFile.is_open())
	{
		m_configFile = filePath;
		std::string line;
		while (std::getline(cFile, line))
		{
			//line.erase(std::remove_if(line.begin(), line.end(), isspace), line.end());
			if (line[0] == '#' || line.empty())
			{
				continue;
			}
			int delimiterPos = static_cast<int>(line.find('='));
			std::string key = GetKey(line, delimiterPos);
			std::string val = GetVal(line, delimiterPos);
			SetConfigVal(key, val);
		}
		cFile.close();
	}
}

void Config::ParseInFolder(const std::string& folderPath)
{
	using fs = std::filesystem::recursive_directory_iterator;

	for (const auto& entry : std::filesystem::recursive_directory_iterator::recursive_directory_iterator(folderPath))
	{
		std::string extension = entry.path().u8string();
		extension = extension.substr(extension.find_last_of("."));

		if (extension == ".txt")
		{
			Parse(entry.path().u8string());
		}
	}
}

void Config::SetVal(const std::string& key, const std::string_view& val)
{
	if (m_configFile.empty())
	{
		return;
	}

	std::ifstream cFile(m_configFile);
	if (cFile.is_open())
	{
		std::string line;
		std::vector<std::string> lines;
		while (std::getline(cFile, line))
		{
			//line.erase(std::remove_if(line.begin(), line.end(), isspace), line.end());
			if (line[0] == '#' || line.empty())
			{
				lines.push_back(line);
				continue;
			}

			int delimiterPos = static_cast<int>(line.find('='));
			std::string fileKey = GetKey(line, delimiterPos);
			if (fileKey == key)
			{
				std::stringstream newLine;
				newLine << key << '=' << val;

				line = newLine.str();
				SetConfigVal(key, val);
			}

			lines.push_back(line);
		}
		cFile.close();

		std::ofstream outputFile(m_configFile);
		if (outputFile.is_open())
		{
			for (auto line : lines)
			{
				outputFile << line << '\n';
			}
			outputFile.close();
		}
	}
}

const std::string Config::GetKey(const std::string& line, const int delimiterPos)
{
	return line.substr(0, delimiterPos);
}

const std::string Config::GetVal(const std::string& line, const int delimiterPos)
{
	return line.substr(delimiterPos + 1);
}

void Config::SetConfigVal(const std::string& key, const std::string_view& val)
{
	ICVar* cVar = m_cvarsRegistry.Find(key);
	if (cVar != nullptr)
	{
		cVar->SetVal(val);
	}
}
