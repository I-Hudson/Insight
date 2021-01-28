#include "ispch.h"
#include "FileSystem.h"

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
		//StopWatching();
		//m_loop.join();
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

	void FileSystemWatcher::RemoveOldDirectory(U64 id)
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
		m_loop = std::thread([this]() { IS_PROFILE_THREAD("FileSystemWatcher"); this->EventLoop(); });
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

	FileNotifcationQueue FileSystemWatcher::GetNotifications()
	{
		FileNotifcationQueue copy;
		if (m_notificationsQueue.TryLock())
		{
			copy = m_notificationsQueue.GetValue();
			m_notificationsQueue.GetValue().clear();
			m_notificationsQueue.Unlock();
		}
		else
		{
			IS_CORE_WARN("[FileSystemWatcher::GetNotifications] Notifications queue was in use.");
		}
		return copy;
	}

	void FileSystemWatcher::EventLoop()
	{
		IS_PROFILE_FUNCTION();
		DWORD numOfBytes = 0;
		OVERLAPPED* ov = nullptr;
		ULONG_PTR compKey = 0;
		BOOL res = FALSE;
		while ((res = GetQueuedCompletionStatus(m_iocp.get(), &numOfBytes, &compKey, &ov, INFINITE)) != FALSE)
		{
			IS_PROFILE_SCOPE("[FileSystemWatcher::EventLoop] GetQueuedCompletionStatus");

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
		if (res != FALSE)
		{
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
		else
		{
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
			auto newNotifications = watchInfo.ProcessNotifications();
			m_notificationsQueue.Lock();
			m_notificationsQueue.GetValue().insert(m_notificationsQueue.GetValue().end(), newNotifications.begin(), newNotifications.end());
			m_notificationsQueue.Unlock();

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
	{
		MutexUnqiueLock lock(m_mutex);
		for (auto& object : m_fileHandles)
		{
			::Delete(object.second.Object);
		}
		lock.unlock(); 
	}

	bool FileSystemManager::FileExists(const std::string& path)
	{
		MutexLockGuard lock(m_mutex);
		return std::filesystem::exists(std::filesystem::path(path));
	}

	void FileSystemManager::UnloadObject(const Object* objectPtr)
	{
		MutexLockGuard lock(m_mutex);
		auto it = m_fileHandles.begin();
		for (auto& object : m_fileHandles)
		{
			if (object.second.Object == objectPtr)
			{
				::Delete(object.second.Object);
				m_fileHandles.erase(it);
				return;
			}
			++it;
		}
	}

	void FileSystemManager::Update()
	{
		//auto notifications = m_fileSystemWatcher.GetNotifications();
		//// maybe this should be done on the other thread?
		////for (auto& n : Notifications)
		//{
		//	IS_CORE_INFO("File Notifications: '{0}'", notifications.size());
		//}
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
		U64 dotPos = filePath.find_last_of('.');
		U64 lastSlash = filePath.find_last_of('\\');
		if (dotPos != std::string::npos && lastSlash != std::string::npos)
		{
			return filePath.substr(lastSlash + (U64)1, (dotPos - lastSlash) - (U64)1);
		}
		return filePath.substr(lastSlash + (U64)1);
	}

	std::string FileSystemManager::GetExtension(const std::string& filePath)
	{
		U64 dotPos = filePath.find_last_of('.');
		if (dotPos != std::string::npos)
		{
			return filePath.substr();
		}
		return std::string("");
	}
}