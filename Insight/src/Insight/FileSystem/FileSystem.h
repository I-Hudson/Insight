#pragma once
#include "Insight/Core/Core.h"
#include "FileHandle.h"

namespace Insight
{
	namespace FileSystem
	{
		using FileNotifyHandle = int;

		class FileSystem
		{
		public:
			FileSystem() = delete;
			FileSystem(const std::string& rootDir);
			~FileSystem();

			void Update();

			SharedPtr<Object> LoadObject(const std::string& filePath);
			void UnloadObject(const WeakPtr<Object> objectPtr);

		private:
			bool SetupRootWatcher(const std::string& path);
			void UninstallNotifcationHandle(FileNotifyHandle& handle);

			bool AsyncFolder(DirectoryListenHandle& handle);

		private:
			std::unordered_map<FileNotifyHandle, FileHandle> m_fileHandles;
			DirectoryListenHandle m_rootDirHandle;
		};
	}
}