#pragma once

#include <assert.h>
#include <stdexcept>
#include "spdlog/spdlog.h"

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

#pragma warning(push)
#pragma warning(disable : 4005)
#if defined(IS_DEBUG)
#define IS_CORE_TRACE(...)	Log::GetCoreLogger()->trace(__VA_ARGS__)
#define IS_CORE_INFO(...)	Log::GetCoreLogger()->info(__VA_ARGS__)
#define IS_CORE_WARN(...)	Log::GetCoreLogger()->warn(__VA_ARGS__)
#define IS_CORE_ERROR(...)	Log::GetCoreLogger()->error(__VA_ARGS__)
#define IS_CORE_FATEL(...)	Log::GetCoreLogger()->critical(__VA_ARGS__)

#define IS_TRACE(...)		Log::GetClientLogger()->trace(__VA_ARGS__)
#define IS_INFO(...)		Log::GetClientLogger()->info(__VA_ARGS__)
#define IS_WARN(...)		Log::GetClientLogger()->warn(__VA_ARGS__)
#define IS_ERROR(...)		Log::GetClientLogger()->error(__VA_ARGS__)
#define IS_FATEL(...)		Log::GetClientLogger()->critical(__VA_ARGS__)

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
#pragma warning(pop)

#define IS_CORE_ASSERT(expr, msg)					__assert(#expr, expr, __FILE__, __LINE__, msg, "Core")
#define IS_CORE_STATIC_ASSERT(expr, msg)			static_assert(expr, msg);
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

template<typename... Args>
std::string StringFormat(const std::string& format, Args... args)
{

	int size = snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
	if (size <= 0) { throw std::runtime_error("Error during formatting."); }
	std::unique_ptr<char[]> buf(new char[size]);
	snprintf(buf.get(), size, format.c_str(), args ...);
	return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}
