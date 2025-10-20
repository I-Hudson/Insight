#pragma once

#include "FileSystem/File.h"
#include "FileSystem/FileSystemResult.h"

#include <string>
#include <unordered_map>

namespace Insight
{
	enum class FileSystemFileType
	{
		File,
		Directory
	};

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

		u32 OpenFilesCount() const;

		virtual bool IsAbsolutePath(std::string_view path) const;

		virtual void PathToAbsoltePath(std::string& path) const;
		virtual void PathToFlattenPath(std::string& path) const;

		std::string GetParentPath(std::string path) const;

		virtual FileSystemResult CreateDirectories(std::string path) const;
		virtual bool FileExists(const std::string_view path) const { return Exists(path, FileSystemFileType::File); }
		virtual bool DirectoryExists(const std::string_view path) const { return Exists(path, FileSystemFileType::Directory); }

	protected:
		virtual FileSystemResult CreateDirectory(const std::string_view path) const = 0;
		virtual bool Exists(const std::string_view path, const FileSystemFileType type) const = 0;

		virtual Ref<IFile> CreateFileHandle(const std::string_view path) = 0;
		virtual void OpenFileHandle(IFile* file) = 0;
		virtual void CloseFileHandle(Ref<IFile>& file) = 0;
		virtual bool DeleteFileHandle(IFile* file) = 0;

	private:
		Ref<IFile> GetOpenedFile(const std::string& path);
		void CheckAndPrintError(const FileSystemResult& result) const;

	protected:
		mutable Threading::SpinLock m_openedFilesLock;
		std::unordered_map<std::string, Ref<IFile>> m_openedFiles;
	};
}