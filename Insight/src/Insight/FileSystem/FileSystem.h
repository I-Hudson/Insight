#pragma once
#include "Insight/Core/Core.h"
#include "FileHandle.h"
#include "WatchInfo.h"
#include "Insight/Templates/TSingleton.h"
#include "Insight/Threading/TThreadSafe.h"
#include <map>
#include <filesystem>

namespace Insight
{
	namespace FileSystem
	{
		using DirectoryHash = U64;
		using FileHash = U64;

		class FileSystemWatcher
		{
		public:
			FileSystemWatcher();
			~FileSystemWatcher();

			void AddNewDirectory(U64 id, const std::string& directory);
			void RemoveOldDirectory(U64 id);

			void StartWatching();
			void StopWatching();

			FileNotifcationQueue GetNotifications();

		private:
			void EventLoop();
			void ProcessEvent(DWORD numberOfBytesTrs, OVERLAPPED* overlapped);

		private:
			std::map<OVERLAPPED*, WatchInfo> m_watchInfos;
			std::mutex m_watchInfoMutex;
			TThreadSafe<FileNotifcationQueue> m_notificationsQueue;

			// I/O completion port (IOCP)
			// It should be only "read" (using GetQueuedCompletionStatus()) from one thread.
			HandlePtr m_iocp;
			// has the "event loop" stopped?
			std::atomic<bool> m_stopped;
			// runs the "event loop", that checks the IOCP
			std::thread m_loop;
		};

		class FileSystemManager : public TSingleton<FileSystemManager>
		{
		public:
			FileSystemManager() = delete;
			FileSystemManager(const std::string& rootDir);
			~FileSystemManager();

			template<typename T, typename... Args>
			SharedPtr<Object> LoadObject(const std::string& filePath, Args&&... args);

			void UnloadObject(const WeakPtr<Object> objectPtr);

			void Update();

		private:
			//void HandleFileNotifcationRenameOld(const FILE_NOTIFY_INFORMATION* fileInfo, DWORD* asyncBuffer, U64& offset);
			std::string GetFileName(const std::string& filePath);
			std::string GetExtension(const std::string& filePath);

		private:
			std::mutex m_mutex;
			std::unordered_map<FileHash, FileHandle> m_fileHandles;
			FileSystemWatcher m_fileSystemWatcher;
			
			//std::unordered_map<DWORD, std::function<void(const FILE_NOTIFY_INFORMATION*, DWORD*, U64&)>> m_fileNotifcationFuncs;
		};

		template<typename T, typename... Args>
		inline SharedPtr<Object> FileSystemManager::LoadObject(const std::string& filePath, Args&&... args)
		{
			IS_CORE_STATIC_ASSERT((std::is_base_of_v<Object, T>), "[FileSystemManager::LoadObject] 'LoadObject' can only be used on Object types.");

			FileHash fileHash = std::hash<std::string>{}(filePath);
			auto it = m_fileHandles.find(fileHash);
			if (it != m_fileHandles.end())
			{
				return DynamicPointerCast<T>((*it).second.Object);
			}

			std::filesystem::path path = filePath;
			FileHandle handle;
			handle.Info = FileInfo
			{
				filePath,
				GetFileName(filePath),
				GetExtension(filePath),
				static_cast<U64>(std::filesystem::file_size(path))
			};
			handle.Object = Object::CreateObject<T>(std::forward<Args>(args)...);
			m_fileHandles[fileHash] = handle;

			return DynamicPointerCast<T>(handle.Object);
		}
	}
}