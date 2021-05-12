#pragma once

#include "FileHandle.h"
#include "WatchInfo.h"
#include "Engine/Templates/TSingleton.h"
#include "Engine/Threading/TThreadSafe.h"
#include <map>
#include <filesystem>

	namespace FileSystem
	{
		using DirectoryHash = u64;
		using FileHash = u64;

		class FileSystemWatcher
		{
		public:
			FileSystemWatcher();
			~FileSystemWatcher();

			void AddNewDirectory(u64 id, const std::string& directory);
			void RemoveOldDirectory(u64 id);

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

			bool FileExists(const std::string& path);

			template<typename T, typename... Args>
			T* LoadObject(const std::string& filePath, Args&&... args);
			template<typename T, typename... Args>
			T* LoadObject(const std::string& filePath, const std::function<SharedPtr<T>(const std::string&)>& createFunction);
			void UnloadObject(const Object* objectPtr);

			void Update();

			std::vector<Byte> ReadFileToVector(const std::string& filePath);
			std::string FormatFilePathStringToUNIX(const std::string& filePath);

		private:
			//void HandleFileNotifcationRenameOld(const FILE_NOTIFY_INFORMATION* fileInfo, DWORD* asyncBuffer, u64& offset);
			std::string GetFileName(const std::string& filePath);
			std::string GetExtension(const std::string& filePath);

		private:
			std::mutex m_mutex;
			std::unordered_map<FileHash, FileHandle> m_fileHandles;
			FileSystemWatcher m_fileSystemWatcher;
			
			//std::unordered_map<DWORD, std::function<void(const FILE_NOTIFY_INFORMATION*, DWORD*, u64&)>> m_fileNotifcationFuncs;
		};

		template<typename T, typename... Args>
		inline T* FileSystemManager::LoadObject(const std::string& filePath, Args&&... args)
		{
			IS_CORE_STATIC_ASSERT((std::is_base_of_v<Object, T>), "[FileSystemManager::LoadObject] 'LoadObject' can only be used on Object types.");

			FileHash fileHash = std::hash<std::string>{}(filePath);
			auto it = m_fileHandles.find(fileHash);
			if (it != m_fileHandles.end())
			{
				return dynamic_cast<T*>((*it).second.Object);
			}

			std::filesystem::path path = filePath;
			FileHandle handle;
			handle.Info = FileInfo
			{
				filePath,
				GetFileName(filePath),
				GetExtension(filePath),
				0//static_cast<u64>(std::filesystem::file_size(path))
			};
			handle.Object = ::New<T>(filePath, std::forward<Args>(args)...);

			MutexUnqiueLock lock(m_mutex);
			m_fileHandles[fileHash] = handle;
			lock.unlock();

			return dynamic_cast<T*>(handle.Object);
		}

		template<typename T, typename ...Args>
		inline T* FileSystemManager::LoadObject(const std::string& filePath, const std::function<SharedPtr<T>(const std::string&)>& createFunction)
		{
			IS_CORE_STATIC_ASSERT((std::is_base_of_v<Object, T>), "[FileSystemManager::LoadObject] 'LoadObject' can only be used on Object types.");

			FileHash fileHash = std::hash<std::string>{}(filePath);
			auto it = m_fileHandles.find(fileHash);
			if (it != m_fileHandles.end())
			{
				return dynamic_cast<T*>((*it).second.Object);
			}

			std::filesystem::path path = filePath;
			FileHandle handle;
			handle.Info = FileInfo
			{
				filePath,
				GetFileName(filePath),
				GetExtension(filePath),
				0//static_cast<u64>(std::filesystem::file_size(path))
			};
			handle.Object = createFunction(filePath);

			MutexUnqiueLock lock(m_mutex);
			m_fileHandles[fileHash] = handle;
			lock.unlock();

			return dynamic_cast<T*>(handle.Object);
		}
	}