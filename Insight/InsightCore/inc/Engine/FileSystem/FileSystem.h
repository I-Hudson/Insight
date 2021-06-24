#pragma once

#include "Engine/Templates/TSingleton.h"
#include "Engine/Threading/TThreadSafe.h"
#include <map>
#include <filesystem>

namespace Insight::FileSystem
{
	class FileSystemManager : public Core::TSingleton<FileSystemManager>
	{
	public:
		FileSystemManager() = delete;
		FileSystemManager(const std::string& rootDir);
		~FileSystemManager();

		bool FileExists(const std::string& path);

		static std::string WindowsToUinxFilePath(std::string path);

		std::vector<Byte> ReadFileToVector(const std::string& filePath);
		std::string FormatFilePathStringToUNIX(const std::string& filePath);

	private:
		std::string GetFileName(const std::string& filePath);
		std::string GetExtension(const std::string& filePath);
	};
}