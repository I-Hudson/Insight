#pragma once

#include "Core/Defines.h"

#include <string>
#include <vector>

IS_CORE void RemoveWhilteSpaces(std::string& str);
IS_CORE void ToLower(std::string& str);
IS_CORE void ToUpper(std::string& str);
IS_CORE std::string RemoveString(const std::string& str, std::string_view strToRemove);
IS_CORE std::vector<std::string> SplitString(const std::string& str, const char splitCharacter);
IS_CORE std::vector<std::string_view> SplitString(std::string_view str, const char splitCharacter);
