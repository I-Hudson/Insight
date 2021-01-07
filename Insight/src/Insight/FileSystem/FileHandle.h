#pragma once

#include "Insight/Core/Core.h"

namespace Insight
{
	namespace FileSystem
	{
		struct FileInfo
		{
			std::string Path;
			std::string Name;
			std::string Extension;
			U64 Size;
		};

		struct DirectoryListenHandle
		{
			HANDLE Handle;
			HANDLE Event;
			OVERLAPPED Overlapped;
			DWORD AsyncBuffer[1024];
		};

		struct FileHandle
		{
			SharedPtr<Object> Object;
			FileInfo Info;
		};
	}
}