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
			OVERLAPPED Overlapped;

			DirectoryListenHandle()
				: Handle(nullptr)
				, Overlapped({})
			{ }
		};

		struct FileHandle
		{
			SharedPtr<Object> Object;
			FileInfo Info;
		};
	}
}