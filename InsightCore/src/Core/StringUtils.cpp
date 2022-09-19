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
