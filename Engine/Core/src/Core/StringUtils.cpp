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
