#pragma once
#if IS_PLATFORM_WINDOWS

#include "FileSystem/FileSystems/IFileSystem.h"

namespace Insight
{
	class WindowsFileSystem;

	class WindowsFile : public IFile
	{
	public:
		WindowsFile(const std::string& path) : IFile(path)
		{
		}

		virtual const char* GetTypeName() const { return GetPath().c_str(); }


	private:
		void* m_win32Handle = nullptr;

		friend class WindowsFileSystem;
	};

	class WindowsFileSystem : public IFileSystem
	{
	public:
		WindowsFileSystem() = default;
		virtual ~WindowsFileSystem();

		// { IFileSystem
	protected:
		virtual FileSystemResult CreateDirectory(const std::string_view path) const override;
		virtual bool Exists(const std::string_view path, const FileSystemFileType type) const override;

		virtual Ref<IFile> CreateFileHandle(const std::string_view path) override;
		virtual void OpenFileHandle(IFile* file) override;
		virtual void CloseFileHandle(Ref<IFile>& file) override;
		virtual bool DeleteFileHandle(IFile* file) override;
		// }

#if IS_TESTING
	public:
#else
	private:
#endif
		std::string PathToWindowsPath(const std::string_view path) const;
	};
}
#endif