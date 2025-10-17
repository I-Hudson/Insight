#include "FileSystem/FileManagerSystem.h"

#include "FileSystem/FileSystems/WindowsFileSystem.h"

#include "Core/Memory.h"

namespace Insight
{
	FileManagerSystem::FileManagerSystem()
	{ }

	FileManagerSystem::~FileManagerSystem()
	{ }

	void FileManagerSystem::Initialise()
	{
		ASSERT(m_nativeFileSystem == nullptr);
#if IS_PLATFORM_WINDOWS
		m_nativeFileSystem = New<WindowsFileSystem>();
#endif
		m_state = Core::SystemStates::Initialised;
	}

	void FileManagerSystem::Shutdown()
	{
		m_nativeFileSystem->Shutdown();
		Delete(m_nativeFileSystem);
		m_state = Core::SystemStates::Not_Initialised;
	}
}