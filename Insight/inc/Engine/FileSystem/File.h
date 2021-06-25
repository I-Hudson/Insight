#pragma once

#include "Engine/Core/Common.h"
#include <vector>

namespace Insight
{
	class File
	{
	public:
		static bool FileExists(const std::string& path);

		static std::string WindowsToUinxFilePath(std::string path);

		static std::vector<Byte> ReadFileToVector(const std::string& filePath);
		static std::string FormatFilePathStringToUNIX(const std::string& filePath);

		static std::string GetFileName(const std::string& filePath);
		static std::string GetExtension(const std::string& filePath);
	};
}