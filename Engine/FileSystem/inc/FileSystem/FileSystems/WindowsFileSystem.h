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
		virtual bool CreateDirectory(const std::string& path) const override;
		virtual bool FileExists(const std::string& path) const override;

	protected:
		virtual Ref<IFile> CreateFileHandle(const std::string& path) override;
		virtual void OpenFileHandle(IFile* file) override;
		virtual void CloseFileHandle(Ref<IFile>& file) override;
		virtual bool DeleteFileHandle(IFile* file) override;
		// }
	};
}
#endif