#pragma once

#include "FileSystem/Defines.h"

#include "Threading/SpinLock.h"
#include "Threading/ScopedLock.h"
#include "Core/ReferencePtr.h"
#include <string>

namespace Insight
{
	class IFileSystem;
	enum class FileStatus
	{
		Opened,
		Closed,
		Deleted,
	};

	/// @brief High level class used to interact with any form of filesystem be that a native OS on or an archive one.
	/// This class should be 
	class IS_FILESYSTEM IFile : public Core::RefCount
	{
	public:
		IFile() = delete;
		IFile(const std::string& path) : m_path(path)
		{ }
		virtual ~IFile() override 
		{
		}

		const std::string& GetPath() const { return m_path; }
		FileStatus GetStatus() const { return m_status; }

		// Seek
		// CurrnetPosition
		// Read
		// Write

	protected:
		std::string m_path;
		std::atomic<FileStatus> m_status = FileStatus::Closed;

		Threading::SpinLock m_lock;

		friend class IFileSystem;
	};
}