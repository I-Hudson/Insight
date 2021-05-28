#pragma once

#include "Engine/Core/Common.h"

namespace Insight::FileSystem
{
	struct FileInfo
	{
		std::string Path;
		std::string Name;
		std::string Extension;
		u64 Size;
	};

	struct DirectoryListenHandle
	{
		HANDLE Handle;
		OVERLAPPED Overlapped;

		DirectoryListenHandle()
			: Handle(nullptr)
			, Overlapped({})
		{ }
	};

	struct FileHandle
	{
		Object* Object;
		FileInfo Info;
	};
}