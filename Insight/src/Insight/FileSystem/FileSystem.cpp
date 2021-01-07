#include "ispch.h"
#include "FileSystem.h"
#include <filesystem>

namespace Insight
{
	namespace FileSystem
	{
		FileSystem::FileSystem(const std::string& rootDir)
		{
			for (const auto& entry : std::filesystem::recursive_directory_iterator::recursive_directory_iterator(rootDir))
			{
				if (entry.is_regular_file())
				{
					
				}
			}
			SetupRootWatcher(rootDir);
		}

		FileSystem::~FileSystem()
		{
			CloseHandle(m_rootDirHandle.Handle);
			CloseHandle(m_rootDirHandle.Event);
		}

		void FileSystem::Update()
		{
			IS_PROFILE_THREAD("FileSystem");
			IS_PROFILE_FUNCTION();

			bool newActions = true;
			if (WaitForSingleObject(m_rootDirHandle.Event, 0) != WAIT_OBJECT_0)
			{
				newActions = false;
			}

			DWORD bytes_returned;
			auto result = GetOverlappedResult(m_rootDirHandle.Handle, &m_rootDirHandle.Overlapped, &bytes_returned, FALSE);

			if (result)
			{
				size_t offset = 0;
				const FILE_NOTIFY_INFORMATION* info = nullptr;
				do
				{
					info = reinterpret_cast<const FILE_NOTIFY_INFORMATION*>(
						reinterpret_cast<const uint8_t*>(m_rootDirHandle.AsyncBuffer) + offset);

					offset += info->NextEntryOffset;
				} while (info->NextEntryOffset != 0);
			}

			AsyncFolder(m_rootDirHandle);
		}

		SharedPtr<Object> FileSystem::LoadObject(const std::string& filePath)
		{
			return SharedPtr<Object>();
		}

		void FileSystem::UnloadObject(const WeakPtr<Object> objectPtr)
		{
		}

		bool FileSystem::SetupRootWatcher(const std::string& path)
		{
			DirectoryListenHandle handle;
			handle.Handle = CreateFileA(path.c_str(),
				FILE_LIST_DIRECTORY,
				FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE,
				nullptr, OPEN_EXISTING,
				FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
				nullptr);
			if (handle.Handle == INVALID_HANDLE_VALUE)
			{
				IS_ERROR("[FileSystem::InstallNotifcationHandle] Failed to track '{0}'", path);
				return -1;
			}

			handle.Event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			if (handle.Event == nullptr)
			{
				CloseHandle(handle.Handle);
				return false;
			}

			if (!AsyncFolder(handle))
			{
				return false;
			}

			m_rootDirHandle = handle;
			return true;
		}

		void FileSystem::UninstallNotifcationHandle(FileNotifyHandle& handle)
		{
		}

		bool FileSystem::AsyncFolder(DirectoryListenHandle& handle)
		{
			handle.Overlapped = {};
			handle.Overlapped.hEvent = handle.Event;
			auto ret = ReadDirectoryChangesW(handle.Handle,
				handle.AsyncBuffer,
				sizeof(handle.AsyncBuffer), FALSE,
				FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_FILE_NAME,
				nullptr,
				&handle.Overlapped,
				nullptr);

			if (!ret && GetLastError() != ERROR_IO_PENDING)
			{
				LPVOID lpMsgBuf;
				DWORD errorCode = GetLastError();
				DWORD bufLen = FormatMessage(
					FORMAT_MESSAGE_ALLOCATE_BUFFER |
					FORMAT_MESSAGE_FROM_SYSTEM |
					FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL,
					errorCode,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					(LPTSTR)&lpMsgBuf,
					0, NULL);

				LPCSTR lpMsgStr = (LPCSTR)lpMsgBuf;
				std::string result(lpMsgStr, lpMsgStr + bufLen);

				LocalFree(lpMsgBuf);

				IS_ERROR("Failed to read directory changes async. Error: '{0}'", result);
				return false;
			}
			return true;
		}
	}
}