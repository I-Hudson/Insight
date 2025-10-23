#if IS_PLATFORM_WINDOWS
#include "FileSystem/FileSystems/WindowsFileSystem.h"

#include "Platforms/Platform.h"

#include "Core/Logger.h"
#include "Core/Asserts.h"
#include "Core/Profiler.h"

#define _M_CEE
#include <Windows.h>

namespace Insight
{
	WindowsFileSystem::~WindowsFileSystem()
	{
	}

	FileSystemResult WindowsFileSystem::CreateDirectory(const std::string_view path) const
	{
		const std::string winStringTemp = PathToWindowsPath(path);

		FileSystemResult fileSystemResult;
		fileSystemResult.Result = CreateDirectoryA(winStringTemp.data(), NULL);
		if (!fileSystemResult)
		{
			fileSystemResult.ErrorMessage = Platform::NativeErrorToMessage(GetLastError());
		}
		return fileSystemResult;
	}

	bool WindowsFileSystem::Exists(const std::string_view path, const FileSystemFileType type) const
	{
		const std::string winStringTemp = PathToWindowsPath(path);
		const DWORD attrib = GetFileAttributesA(winStringTemp.data());

		switch (type)
		{
			case FileSystemFileType::File: return (attrib != INVALID_FILE_ATTRIBUTES && !(attrib & FILE_ATTRIBUTE_DIRECTORY));
			case FileSystemFileType::Directory: return (attrib != INVALID_FILE_ATTRIBUTES && (attrib & FILE_ATTRIBUTE_DIRECTORY));
			default:
			{
				IS_LOG_CORE_ERROR("{}, {}", Platform::NativeErrorToMessage(GetLastError()), GetLastError());
				return false;
			}
		}
		return false;
	}

	Ref<IFile> WindowsFileSystem::CreateFileHandle(const std::string_view path)
	{
		return Ref<WindowsFile>(::New<WindowsFile>(std::string(path)));
	}

	void WindowsFileSystem::OpenFileHandle(IFile* file)
	{
		WindowsFile* winFile = static_cast<WindowsFile*>(file);

		const std::string winStringTemp = PathToWindowsPath(file->GetPath());

		HANDLE win32Handle = CreateFileA(winStringTemp.c_str()
			, GENERIC_READ | GENERIC_WRITE
			, FILE_SHARE_READ | FILE_SHARE_WRITE
			, NULL
			, OPEN_ALWAYS
			, FILE_ATTRIBUTE_NORMAL
			, NULL);
		
		if (win32Handle == INVALID_HANDLE_VALUE)
		{
			const DWORD errorMessageID = GetLastError();
			LPSTR messageBuffer = nullptr;

			//Ask Win32 to give us the string version of that message ID.
			//The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
			size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
										 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

			//Copy the error message into a std::string.
			std::string message(messageBuffer, size);

			//Free the Win32's string's buffer.
			LocalFree(messageBuffer);

			IS_LOG_CORE_ERROR("[WindowsFileSystem::OpenFile] Unable to open/create file '{}'. Error: '{}'.", file->GetPath(), message);
		}

		{
			Threading::ScopedLock fileLock(winFile->m_lock);
			winFile->m_win32Handle = win32Handle;
			winFile->m_status = win32Handle != INVALID_HANDLE_VALUE ? FileStatus::Opened : FileStatus::Closed;
		}
	}

	void WindowsFileSystem::CloseFileHandle(Ref<IFile>& file)
	{
		ASSERT(file);
		Ref<WindowsFile> winFile = file.As<WindowsFile>();
		ASSERT(winFile);

		// There should be a maximum of 3 references.
		// 1. IFileSystem::m_openedFiles, 2. The object passed through initially to 'CloseFile', 3. 'winFile'.
		if (winFile->GetReferenceCount() > 3)
		{
			IS_LOG_CORE_WARN("[WindowsFileSystem::CloseFileHandle] Closing file handle to file '{}'. There are more than two references.", file->GetPath());
		}

		{
			Threading::ScopedLock fileLock(winFile->m_lock);
			CloseHandle(winFile->m_win32Handle);
			winFile->m_win32Handle = INVALID_HANDLE_VALUE;
			winFile->m_status = FileStatus::Closed;
		}
	}

	bool WindowsFileSystem::DeleteFileHandle(IFile* file)
	{
		if (!FileExists(file->GetPath()))
		{
			IS_LOG_CORE_WARN("[WindowsFileSystem::DeleteFileHandle] Trying to delete non-exist file '{}'.", file->GetPath());
			return false;
		}

		if (file->GetStatus() != FileStatus::Closed)
		{
			IS_LOG_CORE_WARN("[WindowsFileSystem::DeleteFileHandle] Trying to delete opened or deleted file '{}'.", (int)file->GetStatus());
			return false;
		}

		const std::string windowsPathTemp = PathToWindowsPath(file->GetPath());

		WindowsFile* winFile = static_cast<WindowsFile*>(file);
		{
			Threading::ScopedLock fileLock(winFile->m_lock);
			DeleteFileA(windowsPathTemp.c_str());
			winFile->m_status = FileStatus::Deleted;
		}
		return true;
	}

	std::string WindowsFileSystem::PathToWindowsPath(const std::string_view path) const
	{
		IS_PROFILE_FUNCTION();
		std::string windowsString(path.data(), path.size());

		std::replace(windowsString.begin(), windowsString.end(), '/', '\\');
		if (!path.empty() && path.back() == L'\\')
		{
			//path.pop_back();
		}

		return windowsString;
	}
}

#if IS_TESTING
#define DOCTEST_CONFIG_IMPLEMENTATION_IN_DLL
//#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest.h>
#include "FileSystem/FileManagerSystem.h"
namespace test
{
	using namespace Insight;
	class WindowsFileSystemFixture
	{
	public:
		WindowsFileSystemFixture()
			: fileSystem(*static_cast<WindowsFileSystem*>(Insight::FileManagerSystem::Instance().GetNativeFileSystem()))
		{ }

		~WindowsFileSystemFixture()
		{ }

		Insight::WindowsFileSystem& fileSystem;
	};

	TEST_SUITE("WindowsFileSystem")
	{
		TEST_CASE_FIXTURE(WindowsFileSystemFixture, "PathToWindowsPath")
		{
			std::string path = "C:/User/Home/Documents/PC/InsightEngine";
			const std::string windowsPathExpected = "C:\\User\\Home\\Documents\\PC\\InsightEngine";

			path = fileSystem.PathToWindowsPath(path);

			CHECK(path == windowsPathExpected);
		}
	}
}
#endif // IS_TESTING

#endif