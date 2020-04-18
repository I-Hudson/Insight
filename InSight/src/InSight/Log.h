#pragma once

#include <memory>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace Framework
{
	class Log
	{
	public:
		static void Init();
		static void Destroy();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_coreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_clientLogger; }

	private:
		static std::shared_ptr<spdlog::logger> s_coreLogger;
		static std::shared_ptr<spdlog::logger> s_clientLogger;
	};
}

#define EN_CORE_TRACE(...)   ::Framework::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define EN_CORE_INFO(...)    ::Framework::Log::GetCoreLogger()->info(__VA_ARGS__)
#define EN_CORE_WARN(...)    ::Framework::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define EN_CORE_ERROR(...)   ::Framework::Log::GetCoreLogger()->error(__VA_ARGS__)
#define EN_CORE_FATEL(...)   ::Framework::Log::GetCoreLogger()->error(__VA_ARGS__)

#define EN_TRACE(...)   ::Framework::Log::GetClientLogger()->trace(__VA_ARGS__)
#define EN_INFO(...)    ::Framework::Log::GetClientLogger()->info(__VA_ARGS__)
#define EN_WARN(...)    ::Framework::Log::GetClientLogger()->warn(__VA_ARGS__)
#define EN_ERROR(...)   ::Framework::Log::GetClientLogger()->error(__VA_ARGS__)
#define EN_FATEL(...)   ::Framework::Log::GetClientLogger()->error(__VA_ARGS__)