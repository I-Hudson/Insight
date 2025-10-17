#include "FileSystem/FileSystems/IFileSystem.h"
#include "FileSystem/FileSystems/WindowsFileSystem.h"

#include "Core/Profiler.h"

namespace Insight
{
	static IFileSystem* g_FileSystem = nullptr;

	IFileSystem* GetNativeFileSystem()
	{
		if (!g_FileSystem)
		{
#if IS_PLATFORM_WINDOWS
			g_FileSystem = New<WindowsFileSystem>();
#endif
		}
		return g_FileSystem;
	}

	Ref<IFile> IFileSystem::OpenFile(std::string path, const bool openFile)
	{
		PathToFileSystemPath(path);
		Ref<IFile> file = GetOpenedFile(path);

		if (file)
		{
			return file;
		}

		file = OpenFileHandle(path, openFile);

		{
			Threading::ScopedLock lock(m_openedFilesLock);
			m_openedFiles[path] = file;
		}

		return file;
	}

	void IFileSystem::CloseFile(Ref<IFile>& file)
	{
		if (!file)
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