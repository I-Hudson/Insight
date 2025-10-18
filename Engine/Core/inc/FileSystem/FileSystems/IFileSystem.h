#pragma once

#include "FileSystem/File.h"

#include <string>
#include <unordered_map>

namespace Insight
{
	/// @brief Interface for all file systems. A file system can be anything that hold files. This would include the 
	/// native system (Windows, Linux) and archive files (.zip) as both of these store individual files.
	class IS_FILESYSTEM IFileSystem
	{
	public:
		IFileSystem() = default;
		virtual ~IFileSystem();

		void Shutdown();

		Ref<IFile> OpenFile(std::string path, const bool openFile = true);
		void CloseFile(Ref<IFile>& file);

		bool DeleteFile(Ref<IFile>& file);

		virtual void PathToFileSystemPath(std::string& path) const;

		virtual bool CreateDirectory(const std::string& path) const = 0;
		virtual bool FileExists(const std::string& path) const = 0;

	protected:
		virtual Ref<IFile> CreateFileHandle(const std::string& path) = 0;
		virtual void OpenFileHandle(IFile* file) = 0;
		virtual void CloseFileHandle(Ref<IFile>& file) = 0;
		virtual bool DeleteFileHandle(IFile* file) = 0;

	private:
		Ref<IFile> GetOpenedFile(const std::string& path);

	protected:
		Threading::SpinLock m_openedFilesLock;
		std::unordered_map<std::string, Ref<IFile>> m_openedFiles;
	};
}