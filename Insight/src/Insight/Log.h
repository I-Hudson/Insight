#pragma once

#include <assert.h>
#include <stdexcept>
#include "spdlog/spdlog.h"

namespace Insight
{
	class Log
	{
	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
}

#if defined(IS_DEBUG)
#define IS_CORE_TRACE(...)							::Insight::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define IS_CORE_INFO(...)							::Insight::Log::GetCoreLogger()->info(__VA_ARGS__)
#define IS_CORE_WARN(...)							::Insight::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define IS_CORE_ERROR(...)							::Insight::Log::GetCoreLogger()->error(__VA_ARGS__)
#define IS_CORE_FATEL(...)							::Insight::Log::GetCoreLogger()->critical(__VA_ARGS__)

#define IS_TRACE(...)								::Insight::Log::GetClientLogger()->trace(__VA_ARGS__)
#define IS_INFO(...)								::Insight::Log::GetClientLogger()->info(__VA_ARGS__)
#define IS_WARN(...)								::Insight::Log::GetClientLogger()->warn(__VA_ARGS__)
#define IS_ERROR(...)								::Insight::Log::GetClientLogger()->error(__VA_ARGS__)
#define IS_FATEL(...)								::Insight::Log::GetClientLogger()->critical(__VA_ARGS__)

#else 
#define IS_CORE_TRACE(...)
#define IS_CORE_INFO(...)
#define IS_CORE_WARN(...)
#define IS_CORE_ERROR(...)
#define IS_CORE_FATEL(...)

#define IS_TRACE(...)
#define IS_INFO(...)
#define IS_WARN(...)
#define IS_ERROR(...)
#define IS_FATEL(...)
#endif

#define IS_CORE_ASSERT(expr, msg)					__assert(#expr, expr, __FILE__, __LINE__, msg, "Core")
#define IS_CORE_STATIC_ASSERT(base, derived, msg)	static_assert(std::is_base_of<base, derived>::value, msg);
#define IS_ASSERT(expr, msg)						__assert(#expr, expr, __FILE__, __LINE__, msg, "Application")

inline void __assert(const std::string& expr_str, bool expr, const std::string& file, int line, const std::string& msg, const std::string& engineAsset)
{
	if (!expr)
	{
		std::string errorMsg = engineAsset + " Assert failed:\t" + msg + '\n' + "Expected:\t" + expr_str + '\n' + "Source:\t\t" + file + ", line " + std::to_string(line) + "\n";
		
		IS_CORE_ERROR(errorMsg);
		__debugbreak();
	}
}
