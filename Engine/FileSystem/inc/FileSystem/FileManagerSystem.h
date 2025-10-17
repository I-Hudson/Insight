#pragma once

#include "FileSystem/Defines.h"
#include "FileSystem/FileSystems/IFileSystem.h"

#include "Core/Singleton.h"
#include "Core/ISysytem.h"

namespace Insight
{
	class IS_FILESYSTEM FileManagerSystem : public Core::ISystem, public Core::Singleton<FileManagerSystem>
	{
	public:
		FileManagerSystem();
		virtual ~FileManagerSystem() override;

		IS_SYSTEM(FileManagerSystem);

		virtual void Initialise() override;
		virtual void Shutdown() override;

		IFileSystem* GetNativeFileSystem() const { return m_nativeFileSystem; }

	private:
		IFileSystem* m_nativeFileSystem = nullptr;
	};
}