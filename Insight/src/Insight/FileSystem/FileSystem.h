#pragma once
#include "Insight/Core/Core.h"
#include "FileHandle.h"
#include "FileSystem/WatchInfo.h"
#include "Insight/Templates/TSingleton.h"
#include "Insight/Threading/TThreadSafe.h"
#include <map>

namespace Insight
{
	namespace FileSystem
	{
		using FileNotifyHandle = U64;

		class FileSystemWatcher
		{
		public:
			FileSystemWatcher();
			~FileSystemWatcher();

			void AddNewDirectory(U64 id, const std::string& directory);
			void RemoveOldDirectory(FileNotifyHandle id);

			void StartWatching();
			void StopWatching();

		private:
			void EventLoop();
			void ProcessEvent(DWORD numberOfBytesTrs, OVERLAPPED* overlapped);
			void ChangeEvent(U64 id, const std::set<std::pair<std::wstring /* path */, uint32_t /* action */>>& notifcations);

		private:
			std::map<OVERLAPPED*, WatchInfo> m_watchInfos;
			std::mutex m_watchInfoMutex;

			// I/O completion port (IOCP)
			// It should be only "read" (using GetQueuedCompletionStatus()) from one thread.
			HandlePtr m_iocp;
			// has the "event loop" stopped?
			std::atomic<bool> m_stopped;
			// runs the "event loop", that checks the IOCP
			std::thread m_loop;
		};

		class FileSystem : public TSingleton<FileSystem>
		{
		public:
			FileSystem() = delete;
			FileSystem(const std::string& rootDir);
			~FileSystem();

			SharedPtr<Object> LoadObject(const std::string& filePath);
			void UnloadObject(const WeakPtr<Object> objectPtr);

			void Update();

		private:
			void HandleFileNotifcationRenameOld(const FILE_NOTIFY_INFORMATION* fileInfo, DWORD* asyncBuffer, U64& offset);

		private:
			std::mutex m_mutex;
			std::unordered_map<FileNotifyHandle, FileHandle> m_fileHandles;
			std::unordered_map<DWORD, std::function<void(const FILE_NOTIFY_INFORMATION*, DWORD*, U64&)>> m_fileNotifcationFuncs;
			FileSystemWatcher m_fileSystemWatcher;
			std::thread m_fileWatcherThread;
		};
	}
}