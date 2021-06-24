#include "Engine/FileSystem/FileSystem.h"
#include <fstream>

namespace Insight::FileSystem
{
	/*
	* -------------------------------------------------------------------------------------------------------------------
	*/

	FileSystemManager::FileSystemManager(const std::string& rootDir)
	{
		//m_fileNotifcationFuncs[FILE_ACTION_ADDED] = std::bind(&FileSystemManager::HandleFileNotifcationRenameOld, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
		//m_fileNotifcationFuncs[FILE_ACTION_MODIFIED] = std::bind(&FileSystemManager::HandleFileNotifcationRenameOld, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
		//m_fileNotifcationFuncs[FILE_ACTION_REMOVED] = std::bind(&FileSystemManager::HandleFileNotifcationRenameOld, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
		//m_fileNotifcationFuncs[FILE_ACTION_RENAMED_OLD_NAME] = std::bind(&FileSystemManager::HandleFileNotifcationRenameOld, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
		//m_fileNotifcationFuncs[FILE_ACTION_RENAMED_NEW_NAME] = std::bind(&FileSystemManager::HandleFileNotifcationRenameOld, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

		//m_fileSystemWatcher.AddNewDirectory(0, rootDir);
		//m_fileSystemWatcher.StartWatching();
	}

	FileSystemManager::~FileSystemManager()
	{ }

	bool FileSystemManager::FileExists(const std::string& path)
	{
		return std::filesystem::exists(std::filesystem::path(path));
	}

	std::string FileSystemManager::WindowsToUinxFilePath(std::string path)
	{
		std::replace(path.begin(), path.end(), '\\', '/');
		return path;
	}

	std::vector<Byte> FileSystemManager::ReadFileToVector(const std::string& filePath)
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

	std::string FileSystemManager::FormatFilePathStringToUNIX(const std::string& filePath)
	{
		std::string temp = filePath;
		std::replace(temp.begin(), temp.end(), '\\', '/');
		return temp;
	}

	std::string FileSystemManager::GetFileName(const std::string& filePath)
	{
		u64 dotPos = filePath.find_last_of('.');
		u64 lastSlash = filePath.find_last_of('\\');
		if (dotPos != std::string::npos && lastSlash != std::string::npos)
		{
			return filePath.substr(lastSlash + (u64)1, (dotPos - lastSlash) - (u64)1);
		}
		return filePath.substr(lastSlash + (u64)1);
	}

	std::string FileSystemManager::GetExtension(const std::string& filePath)
	{
		u64 dotPos = filePath.find_last_of('.');
		if (dotPos != std::string::npos)
		{
			return filePath.substr();
		}
		return std::string("");
	}
}