#include "Engine/FileSystem/File.h"

#include <filesystem>

namespace Insight
{
	bool File::FileExists(const std::string& path)
	{
		return std::filesystem::exists(std::filesystem::path(path));
	}

	std::string File::WindowsToUinxFilePath(std::string path)
	{
		std::replace(path.begin(), path.end(), '\\', '/');
		return path;
	}

	std::vector<Byte> File::ReadFileToVector(const std::string& filePath)
	{
		std::ifstream file(filePath);
		if (file.is_open())
		{
			std::vector<Byte> vec;
			std::copy(
				std::istreambuf_iterator<char>(file),
				std::istreambuf_iterator<char>(),
				std::back_inserter(vec));
			return vec;
		}
		return std::vector<Byte>();
	}

	std::string File::FormatFilePathStringToUNIX(const std::string& filePath)
	{
		std::string temp = filePath;
		std::replace(temp.begin(), temp.end(), '\\', '/');
		return temp;
	}

	std::string File::GetFileName(const std::string& filePath)
	{
		u64 dotPos = filePath.find_last_of('.');
		u64 lastSlash = filePath.find_last_of('\\');
		if (dotPos != std::string::npos && lastSlash != std::string::npos)
		{
			return filePath.substr(lastSlash + (u64)1, (dotPos - lastSlash) - (u64)1);
		}
		return filePath.substr(lastSlash + (u64)1);
	}

	std::string File::GetExtension(const std::string& filePath)
	{
		u64 dotPos = filePath.find_last_of('.');
		if (dotPos != std::string::npos)
		{
			return filePath.substr();
		}
		return std::string("");
	}
}