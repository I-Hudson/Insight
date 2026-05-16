#include "Core/StringUtils.h"

#include <algorithm>


void RemoveWhilteSpaces(std::string& str)
{
	IS_UNUSED(std::remove_if(str.begin(), str.end(), isspace));
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

std::vector<std::string> SplitString(const std::string& str, const char* splitCharacter)
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

std::vector<std::string_view> SplitString(std::string_view str, const char* splitCharacter)
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

void ReplaceAll(std::string& str, const std::string& oldVal, const std::string& newVal)
{
	std::string newString;
	newString.reserve(str.length());  // avoids a few memory allocations

	std::string::size_type lastPos = 0;
	std::string::size_type findPos;

	while (std::string::npos != (findPos = str.find(oldVal, lastPos)))
	{
		newString.append(str, lastPos, findPos - lastPos);
		newString += newVal;
		lastPos = findPos + oldVal.length();
	}

	// Care for the rest after last occurrence
	newString.append(str, lastPos, str.length() - lastPos);

	str.swap(newString);
}
