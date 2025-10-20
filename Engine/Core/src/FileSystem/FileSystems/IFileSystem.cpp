#include "FileSystem/FileSystems/IFileSystem.h"
#include "FileSystem/FileSystems/WindowsFileSystem.h"

#include "Core/Profiler.h"
#include "Core/Asserts.h"
#include "Core/Logger.h"

#include "Platforms/Platform.h"

#include <filesystem>

namespace Insight
{
	constexpr const char kSeperator = '/';
	constexpr const char kSeperatorSize = 1;

	IFileSystem::~IFileSystem()
	{
		ASSERT(m_openedFiles.empty());
	}

	void IFileSystem::Shutdown()
	{
		for (auto& [path, file] : m_openedFiles)
		{
			CloseFile(file);
		}

		m_openedFiles.clear();
	}

	Ref<IFile> IFileSystem::OpenFile(std::string path, const bool openFile)
	{
		PathToAbsoltePath(path);

		Ref<IFile> file = GetOpenedFile(path);

		if (!file)
		{
			file = CreateFileHandle(path);
			Threading::ScopedLock lock(m_openedFilesLock);
			m_openedFiles[path] = file;
		}

		if (openFile)
		{
			OpenFileHandle(file.Ptr());
		}

		return file;
	}

	void IFileSystem::CloseFile(Ref<IFile>& file)
	{
		if (!file || file->GetStatus() != FileStatus::Opened)
		{
			return;
		}

		CloseFileHandle(file);

		if (file->GetReferenceCount() == 1)
		{
			//Threading::ScopedLock lock(m_openedFilesLock);
			//m_openedFiles.erase(file->GetPath());
		}
	}

	bool IFileSystem::DeleteFile(Ref<IFile>& file)
	{
		if (!file)
		{
			return false;
		}

		return DeleteFileHandle(file.Ptr());
	}

	u32 IFileSystem::OpenFilesCount() const
	{
		Threading::ScopedLock lock(m_openedFilesLock);
		return m_openedFiles.size();
	}

	bool IFileSystem::IsAbsolutePath(std::string_view path) const
	{
		// Just check the first three characters.
		if (path.find(":") == 1)
		{
			return true;
		}
		return false;
	}

	void IFileSystem::PathToAbsoltePath(std::string& path) const
	{
		if (IsAbsolutePath(path))
		{
			PathToFlattenPath(path);
			// We have a drive letter so must go from the drive to the destination folder.
			return;
		}
		
		//TODO: Change 'GetExecuteablePath' to return what ever format the OS returns and not make it a Unix path.
		const std::string exePath = Platform::GetExecuteablePath();
		//std::string exeParent = GetParentPath(exePath);
		path = exePath + kSeperator + (path.front() == kSeperator ? path.substr(1) : path);

		PathToFlattenPath(path);
	}

	void IFileSystem::PathToFlattenPath(std::string& path) const
	{
		if (!IsAbsolutePath(path))
		{
			IS_LOG_CORE_ERROR("[FileSystem::FlattenAbsolutePath] Path '{}', isn't an absolute path. Can't flatten.", path);
			return;
		}

		const std::string c_upDirectoryStr = std::string("..") + kSeperator;
		u64 upDirectory = path.find(c_upDirectoryStr);

		while (upDirectory != std::string::npos)
		{
			const u64 upDirectoryStart = path.rfind(kSeperator, upDirectory);
			ASSERT(upDirectory > 0);
			ASSERT(upDirectoryStart > 0);
			ASSERT((upDirectory - upDirectoryStart) == 1);

			const u64 redundentDirStart = path.rfind(kSeperator, upDirectoryStart - 1);
			path.erase(path.begin() + redundentDirStart + 1, path.begin() + upDirectory + c_upDirectoryStr.size());

			upDirectory = path.find(c_upDirectoryStr);
		}

		if (path.back() == kSeperator)
		{
			path.pop_back();
		}
	}

	std::string IFileSystem::GetParentPath(std::string path) const
	{
		const u64 parentIndex = path.find_last_of(kSeperator);

		if (parentIndex == std::string::npos)
		{
			return "";
		}
		return path.substr(0, parentIndex);
	}

	FileSystemResult IFileSystem::CreateDirectories(std::string path) const
	{
		PathToAbsoltePath(path);

		std::error_code errorCode;
		FileSystemResult lastResult;
		std::filesystem::create_directories(path, errorCode);
		
		lastResult.Result = !errorCode.value();
		if (!lastResult.Result)
		{
			IS_LOG_CORE_ERROR("[FileSystem::CreateFolder] Error code: '{}', Message: '{}'.", errorCode.value(), errorCode.message());
			lastResult.ErrorMessage = errorCode.message();
		}
		
		return lastResult;
#if 0

		ASSERT(path.find(":") == 1);
		u64 directoySplit = path.find(kSeperator, kSeperatorSize + 1);

		FileSystemResult lastResult;
		if (directoySplit == std::string::npos)
		{
			lastResult = CreateDirectory(path);
			if (!lastResult)
			{
				IS_LOG_CORE_ERROR("[IFileSystem::CreateDirectories] Path: '{}', Error: '{}'.", path, lastResult.ErrorMessage);
			}
			return lastResult;
		}

		std::string_view currPath = std::string_view(path.c_str(), directoySplit);
		while (currPath != path)
		{
			if (!DirectoryExists(currPath))
			{

				lastResult = CreateDirectory(path);
				if (!lastResult)
				{
					IS_LOG_CORE_ERROR("[IFileSystem::CreateDirectories] Error: '{}'.", lastResult.ErrorMessage);
					return lastResult;
				}
			}

			directoySplit = path.find(kSeperator, directoySplit + kSeperatorSize);
			if (directoySplit != std::string::npos)
			{
				currPath = std::string_view(path.c_str(), directoySplit);
			}
			else
			{
				currPath = std::string_view(path.c_str());
			}
		}

		return lastResult;
#endif
	}

	Ref<IFile> IFileSystem::GetOpenedFile(const std::string& path)
	{
		if (const auto iter = m_openedFiles.find(path);
			iter != m_openedFiles.end())
		{
			return iter->second;
		}
		return Ref<IFile>();
	}

	void IFileSystem::CheckAndPrintError(const FileSystemResult& result) const
	{
		if (!result)
		{
			IS_LOG_CORE_ERROR("[IFileSystem] Error: '{}'.", result.ErrorMessage);
		}
	}
}

#if IS_TESTING
#include "FileSystem/FileManagerSystem.h"
#define DOCTEST_CONFIG_IMPLEMENTATION_IN_DLL
#include <doctest.h>
namespace test
{
	class IFileSystemTestFixture
	{
	public:
		IFileSystemTestFixture()
			:fileManager(Insight::FileManagerSystem::Instance())
		{ }
		~IFileSystemTestFixture()
		{ }

		Insight::FileManagerSystem& fileManager;
	};

	TEST_CASE_FIXTURE(IFileSystemTestFixture, "IsAbsolutePath")
	{
		CHECK_FALSE(fileManager.GetNativeFileSystem()->IsAbsolutePath("dummyPath"));
		CHECK(fileManager.GetNativeFileSystem()->IsAbsolutePath("C:/dummyPath"));
		CHECK(fileManager.GetNativeFileSystem()->IsAbsolutePath("C:\\dummyPath"));
	}

	TEST_CASE_FIXTURE(IFileSystemTestFixture, "PathToAbsoltePath")
	{
		std::string localPath = "dummyPath";
		const std::string exePath = Insight::Platform::GetExecuteablePath();
		const std::string localAbsPath = exePath + Insight::kSeperator + "dummyPath";

		fileManager.GetNativeFileSystem()->PathToAbsoltePath(localPath);
		CHECK(localPath == localAbsPath);
	}
	TEST_CASE_FIXTURE(IFileSystemTestFixture, "PathToFlattenPath")
	{
		std::string localPath = "/dummyPath";
		const std::string exePath = Insight::Platform::GetExecuteablePath();
		std::string localAbsPath = exePath + "/dummyPath/dummyPath1/dummyPath2/../../";

		std::string flattenAbsPath = localAbsPath;

		fileManager.GetNativeFileSystem()->PathToFlattenPath(flattenAbsPath);
		fileManager.GetNativeFileSystem()->PathToAbsoltePath(localAbsPath);
		fileManager.GetNativeFileSystem()->PathToAbsoltePath(localPath);
		CHECK(localPath == flattenAbsPath);
		CHECK(flattenAbsPath == localAbsPath);
		CHECK(localAbsPath == localPath);
	}
}
#endif