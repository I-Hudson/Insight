#include "Core/StringUtils.h"

#include <algorithm>


void RemoveWhilteSpaces(std::string& str)
{
	auto a = std::remove_if(str.begin(), str.end(), isspace);
}

void ToLower(std::string& str)
{
	std::transform(str.begin(), str.end(), str.begin(), [](char c)
		{
			return std::tolower(c);
		});
}

void ToUpper(std::string& str)
{
	std::transform(str.begin(), str.end(), str.begin(), [](char c)
		{
			return std::toupper(c);
		});
}

std::string RemoveString(const std::string& str, std::string_view strToRemove)
{
	if (uint64_t index = str.find(strToRemove); index != std::string::npos)
	{
	    return str.substr(strToRemove.size() + 1);
	}
	return str;
}

std::vector<std::string> SplitString(const std::string& str, const char splitCharacter)
{
	std::vector<std::string> strings;

	std::string s = str;
	uint64_t index = s.find(splitCharacter);
	while (index != std::string::npos)
	{
		strings.push_back(s.substr(0, index));
		s = s.substr(index + 1);
		index = s.find(splitCharacter);
	}
	if (!s.empty())
	{
		strings.push_back(s);
	}
	return strings;

	//for (const char& c : str)
	//{
	//	if (c == splitCharacter)
	//	{
	//		strings.push_back(s);
	//		s.clear();
	//	}
	//	else
	//	{
	//		s += c;
	//	}
	//}

	//if (!s.empty())
	//{
	//	strings.push_back(s);
	//}

	return strings;
}

std::vector<std::string_view> SplitString(std::string_view str, const char splitCharacter)
{
	std::vector<std::string_view> strings;

	std::string_view s = str;
	uint64_t index = s.find(splitCharacter);
	while (index != std::string::npos)
	{
		strings.push_back(s.substr(0, index));
		s = s.substr(index + 1);
		index = s.find(splitCharacter);
	}
	if (!s.empty())
	{
		strings.push_back(s);
	}
	return strings;
}
