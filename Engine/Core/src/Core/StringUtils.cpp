#include "Core/StringUtils.h"

#include <algorithm>


void RemoveWhilteSpaces(std::string& str)
{
	auto a = std::remove_if(str.begin(), str.end(), isspace);
}

void ToLowwer(std::string& str)
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

IS_CORE std::string RemoveString(const std::string& str, std::string_view strToRemove)
{
	if (uint64_t index = str.find(strToRemove); index != std::string::npos)
	{
	    return str.substr(strToRemove.size() + 1);
	}
	return str;
}

IS_CORE std::vector<std::string> SplitString(const std::string& str, const char splitCharacter)
{
	std::vector<std::string> strings;
	std::string s;
	for (const char& c : str)
	{
		if (c == splitCharacter
			&& !s.empty())
		{
			strings.push_back(s);
			s.clear();
		}
		else
		{
			s += c;
		}
	}

	if (!s.empty())
	{
		strings.push_back(s);
	}

	return strings;
}
