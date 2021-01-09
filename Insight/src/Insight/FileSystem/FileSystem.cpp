#include "ispch.h"
#include "FileSystem.h"
#include <filesystem>
#include <codecvt>

namespace Insight
{
	namespace FileSystem
	{
		FileSystemWatcher::FileSystemWatcher()
		{
			m_iocp.reset(CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 1));
			if (!m_iocp)
			{
				throw std::runtime_error("Error when creating IOCP.");
			}
		}

		FileSystemWatcher::~FileSystemWatcher()
		{
			StopWatching();
			m_loop.join();

		}

		void FileSystemWatcher::AddNewDirectory(U64 id, const std::string& directory)
		{
			HandlePtr dirHandle(CreateFileA(directory.c_str(),
				FILE_LIST_DIRECTORY,
				FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
				NULL,
				OPEN_EXISTING,
				FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
				NULL));
			if (dirHandle.get() == INVALID_HANDLE_VALUE)
			{
				std::cerr << "Cannot create directory handle: " << GetLastError() << std::endl;
			}
			// check if it is even a directory:
			{
				BY_HANDLE_FILE_INFORMATION fileInfo{};
				const BOOL res = GetFileInformationByHandle(dirHandle.get(), &fileInfo);
				if (res == FALSE)
				{
					IS_CORE_ERROR("[FileSystemWatcher::ProcessEvent] was unable to get file information from handle.");
				}
				else if (!(fileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					std::cerr << "Not a directory." << std::endl;
				}
			}
			// the "old" IOCP handle should not be freed, because:
			// https://devblogs.microsoft.com/oldnewthing/20130823-00/?p=3423
			if (!CreateIoCompletionPort(dirHandle.get(), m_iocp.get(), NULL, 1))
			{
				std::cerr << "Cannot create IOCP: " << GetLastError() << std::endl;
			}

			// create the internal data structures, and set up listening with RDC():
			{
				auto ov = std::make_unique<OVERLAPPED>();
				OVERLAPPED* const ovPtr = ov.get();
				{
					MutexLockGuard lock(m_watchInfoMutex);
					const auto info = m_watchInfos.emplace(std::piecewise_construct, std::forward_as_tuple(ovPtr),
						std::forward_as_tuple(id, std::move(ov), std::wstring(directory.begin(), directory.end()), std::move(dirHandle)));
					if (!info.second)
					{
						IS_CORE_ERROR("[FileSystemWatcher::ProcessEvent] event error");
					}
					if (m_stopped)
					{
						m_watchInfos.erase(info.first);
					}
					else {
						bool result = info.first->second.Listen();
						if (!result)
						{
							m_watchInfos.erase(info.first);
						}
					}
				}
			}
		}

		void FileSystemWatcher::RemoveOldDirectory(FileNotifyHandle id)
		{
			MutexLockGuard lock(m_watchInfoMutex);
			auto watchInfoIt = std::find_if(
				this->m_watchInfos.begin(), this->m_watchInfos.end(),
				[id](decltype(this->m_watchInfos)::const_reference value)
				{
					return value.second.rId == id;
				});
			if (watchInfoIt != this->m_watchInfos.end())
			{
				watchInfoIt->second.Stop();
			}
			// we should not emit events for this particular directory after this call
		}

		void FileSystemWatcher::StartWatching()
		{
			m_loop = std::thread([this]() { this->EventLoop(); });
		}

		void FileSystemWatcher::StopWatching()
		{
			{
				MutexLockGuard lock(m_watchInfoMutex);
				for (auto& watchInfo : m_watchInfos)
				{
					watchInfo.second.Stop();
				}
			}
			if (m_iocp.get() != INVALID_HANDLE_VALUE)
			{
				// send stop "magic packet"
				PostQueuedCompletionStatus(m_iocp.get(), 0, reinterpret_cast<ULONG_PTR>(this), nullptr);
			}
		}

		void FileSystemWatcher::EventLoop()
		{
			DWORD numOfBytes = 0;
			OVERLAPPED* ov = nullptr;
			ULONG_PTR compKey = 0;
			BOOL res = FALSE;
			while ((res = GetQueuedCompletionStatus(m_iocp.get(), &numOfBytes, &compKey, &ov, INFINITE)) != FALSE)
			{
				if (compKey != 0 && compKey == reinterpret_cast<ULONG_PTR>(this)) {
					// stop "magic packet" was sent, so we shut down:
					break;
				}
				else
				{
					ProcessEvent(numOfBytes, ov);
				}
			}

			m_stopped.store(true);
			if (res != FALSE) {
				// IOCP is intact, so we clean up outstanding calls:
				MutexLockGuard lock(m_watchInfoMutex);
				const auto hasPending = [&lock, this]()
				{
					return std::find_if(this->m_watchInfos.cbegin(), this->m_watchInfos.cend(),
						[](decltype(m_watchInfos)::const_reference value)
						{
							return value.second.state == WatchInfo::State::PendingClose;
						}) != this->m_watchInfos.cend();
				};
				while (hasPending()
					&& (res = GetQueuedCompletionStatus(
						m_iocp.get(), &numOfBytes, &compKey, &ov, INFINITE))
					!= FALSE)
				{
					const auto watchInfoIt = this->m_watchInfos.find(ov);
					if (watchInfoIt != this->m_watchInfos.end())
					{
						watchInfoIt->second.state = WatchInfo::State::PendingClose;
						this->m_watchInfos.erase(watchInfoIt);
					}
				}
			}
			else {
				const uint32_t errorCode = GetLastError();
				std::cerr << "There is something wrong with the IOCP: " << errorCode << std::endl;
				// alert all subscribers that they will not receive events from now on:
				MutexLockGuard lock(m_watchInfoMutex);
				for (auto& watchInfo : this->m_watchInfos)
				{
					//ErrorEvent(watchInfo.second.rId);
				}
			}
		}

		void FileSystemWatcher::ProcessEvent(DWORD numberOfBytesTrs, OVERLAPPED* overlapped)
		{
			MutexLockGuard lock(m_watchInfoMutex);
			// initialization:
			const auto watchInfoIt = m_watchInfos.find(overlapped);
			if (watchInfoIt == m_watchInfos.end()) {
				std::cerr << "WatchInfo was not found for filesystem event." << std::endl;
				return;
			}
			if (watchInfoIt->second.state == WatchInfo::State::Listening)
			{
				watchInfoIt->second.state = WatchInfo::State::Initialized;
			}

			// actual logic:
			if (numberOfBytesTrs == 0) {
				if (watchInfoIt->second.state == WatchInfo::State::PendingClose)
				{
					// this is the "closing" notification, se we clean up:
					m_watchInfos.erase(watchInfoIt);
				}
				else
				{
					IS_CORE_ERROR("[FileSystemWatcher::ProcessEvent] event error: '{0}'", watchInfoIt->second.rId);
				}
				return;
			}

			WatchInfo& watchInfo = watchInfoIt->second;

			// If we're already in PendingClose state, and receive a legitimate notification, then
			// we don't emit a change notification, however, we delete the WatchInfo, just like when
			// we receive a "closing" notification.

			if (watchInfo.CanRun()) 
			{
				auto notificationResult = watchInfo.ProcessNotifications();

				if (!notificationResult.empty())
				{
					ChangeEvent(watchInfo.rId, std::move(notificationResult));
				}
				auto res = watchInfo.Listen();
				if (!res) 
				{
					IS_CORE_ERROR("[FileSystemWatcher::ProcessEvent] event error: '{0}'", watchInfoIt->second.rId);
					m_watchInfos.erase(watchInfoIt);
				}
			}
			else 
			{
				m_watchInfos.erase(watchInfoIt);
			}
		}

		void FileSystemWatcher::ChangeEvent(U64 id, const std::set<std::pair<std::wstring, uint32_t>>& notifcations)
		{
			for (auto& change : notifcations)
			{
				std::string filePath = WStringToString(change.first.c_str());
				IS_CORE_INFO("[FileSystemWatcher::ChangeEvent] Action: '{0}', Data: {1}", change.second, filePath);
			}
		}

		/*
		* -------------------------------------------------------------------------------------------------------------------
		*/

		FileSystem::FileSystem(const std::string& rootDir)
		{
			auto getFileName = [](const std::string& filePath)
			{
				U64 dotPos = filePath.find_last_of('.');
				U64 lastSlash = filePath.find_last_of('\\');
				if (dotPos != std::string::npos && lastSlash != std::string::npos)
				{
					return filePath.substr(lastSlash + (U64)1, (dotPos - lastSlash) - (U64)1);
				}
				return filePath.substr(lastSlash + (U64)1);
			};
			auto getExtension = [](const std::string& filePath)
			{
				U64 dotPos = filePath.find_last_of('.');
				if (dotPos != std::string::npos)
				{
					return filePath.substr();
				}
				return std::string("");
			};

			for (const auto& entry : std::filesystem::recursive_directory_iterator::recursive_directory_iterator(rootDir))
			{
				if (entry.is_regular_file())
				{
					const std::string filePath = entry.path().u8string();

					FileHandle handle;
					handle.Info = FileInfo
					{
						filePath,
						getFileName(filePath),
						getExtension(filePath),
						static_cast<U64>(std::filesystem::file_size(entry))
					};

					FileNotifyHandle hash = std::hash<std::string>{}(handle.Info.Path);
					m_fileHandles[hash] = handle;
				}
			}

			m_fileNotifcationFuncs[FILE_ACTION_ADDED] = std::bind(&FileSystem::HandleFileNotifcationRenameOld, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
			m_fileNotifcationFuncs[FILE_ACTION_MODIFIED] = std::bind(&FileSystem::HandleFileNotifcationRenameOld, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
			m_fileNotifcationFuncs[FILE_ACTION_REMOVED] = std::bind(&FileSystem::HandleFileNotifcationRenameOld, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
			m_fileNotifcationFuncs[FILE_ACTION_RENAMED_OLD_NAME] = std::bind(&FileSystem::HandleFileNotifcationRenameOld, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
			m_fileNotifcationFuncs[FILE_ACTION_RENAMED_NEW_NAME] = std::bind(&FileSystem::HandleFileNotifcationRenameOld, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

			m_fileSystemWatcher.AddNewDirectory(0, rootDir);
			m_fileSystemWatcher.StartWatching();
		}

		FileSystem::~FileSystem()
		{
		}


		SharedPtr<Object> FileSystem::LoadObject(const std::string& filePath)
		{
			return SharedPtr<Object>();
		}

		void FileSystem::UnloadObject(const WeakPtr<Object> objectPtr)
		{
		}

		void FileSystem::Update()
		{
			//IS_CORE_INFO("File Notifcations: '{0}'", m_fileSystemWatcher.GetNotifcations().size());
		}

		void FileSystem::HandleFileNotifcationRenameOld(const FILE_NOTIFY_INFORMATION* fileInfo, DWORD* asyncBuffer, U64& offset)
		{
			offset += fileInfo->NextEntryOffset;
			const FILE_NOTIFY_INFORMATION* newInfo = reinterpret_cast<const FILE_NOTIFY_INFORMATION*>(
				reinterpret_cast<const uint8_t*>(asyncBuffer) + offset);

			std::string newPathName = WStringToString(fileInfo->FileName);
			FileNotifyHandle hash = std::hash<std::string>{}(newPathName);
			if (newInfo->Action == FILE_ACTION_RENAMED_NEW_NAME)
			{
				m_fileHandles[hash].Info.Path = newPathName;
			}
		}

	}
}