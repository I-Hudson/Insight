#pragma once

#include <string>

namespace Insight
{
	struct FileSystemResult
	{
		operator bool() const { return Result; }

		bool Result;
		std::string ErrorMessage;
	};
}