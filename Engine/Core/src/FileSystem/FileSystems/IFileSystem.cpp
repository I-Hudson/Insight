#include "FileSystem/FileSystems/IFileSystem.h"
#include "FileSystem/FileSystems/WindowsFileSystem.h"

#include "Core/Profiler.h"
#include "Core/Asserts.h"

namespace Insight
{
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
		PathToFileSystemPath(path);
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

	void IFileSystem::PathToFileSystemPath(std::string& path) const
	{
		IS_PROFILE_FUNCTION();
		std::replace(path.begin(), path.end(), '/', '\\');
		if (!path.empty() && path.back() == L'\\')
		{
			path.pop_back();
		}
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
}