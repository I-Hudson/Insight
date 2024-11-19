#pragma once

#include "Core/Defines.h"

#pragma warning( push )
#pragma warning( disable : 4275 )
#include "spdlog/spdlog.h"
#include "spdlog/sinks/base_sink.h"
#pragma warning( pop )

namespace Insight
{
	namespace Core
	{
		//// <summary>
		//// Logger. Setup spdlog to be used within core systems and client systems.
		//// </summary>
		class IS_CORE Logger
		{
		public:
			static void Init();
			static void Destroy();

			static std::shared_ptr<spdlog::logger>& GetCoreFileLogger() { return s_CoreLogger; }
			static std::shared_ptr<spdlog::logger>& GetClientFileLogger() { return s_ClientLogger; }

		private:
			static std::shared_ptr<spdlog::logger> s_CoreLogger;
			static std::shared_ptr<spdlog::logger> s_ClientLogger;
		};

		class ConsoleSink : public spdlog::sinks::base_sink<std::mutex>
		{
		protected:
			virtual void sink_it_(const spdlog::details::log_msg& msg) override;
			virtual void flush_() override;
		};
	}
}

#define SET_SPDLOG_LOGGERS()\
std::shared_ptr<spdlog::logger> coreLogger = ::Insight::Core::Logger::GetCoreFileLogger();\
std::shared_ptr<spdlog::logger> clientLogger = ::Insight::Core::Logger::GetClientFileLogger();\
spdlog::register_logger(coreLogger);\
spdlog::register_logger(clientLogger);

#pragma warning(push)
#pragma warning(disable : 4005)
#define IS_LOGGING_ENABLED
#if defined(IS_LOGGING_ENABLED)
#if defined(IS_CORE)
#define IS_LOG_CORE_TRACE(format, ...)	::Insight::Core::Logger::GetCoreFileLogger()->trace(format, __VA_ARGS__)
#define IS_LOG_CORE_INFO(format, ...)	::Insight::Core::Logger::GetCoreFileLogger()->info(format, __VA_ARGS__)
#define IS_LOG_CORE_WARN(format, ...)	::Insight::Core::Logger::GetCoreFileLogger()->warn(format, __VA_ARGS__)
#define IS_LOG_CORE_ERROR(format, ...)	::Insight::Core::Logger::GetCoreFileLogger()->error(format, __VA_ARGS__)
#define IS_LOG_CORE_FATEL(format, ...)	::Insight::Core::Logger::GetCoreFileLogger()->critical(format, __VA_ARGS__)
#else 
#define IS_LOG_CORE_TRACE(...) IS_UNUSED(0)
#define IS_LOG_CORE_INFO(...) IS_UNUSED(0)
#define IS_LOG_CORE_WARN(...) IS_UNUSED(0)
#define IS_LOG_CORE_ERROR(...) IS_UNUSED(0)
#define IS_LOG_CORE_FATEL(...) IS_UNUSED(0)
#endif

#define IS_LOG_TRACE(...)		::Insight::Core::Logger::GetClientFileLogger()->trace(__VA_ARGS__)
#define IS_LOG_INFO(...)		::Insight::Core::Logger::GetClientFileLogger()->info(__VA_ARGS__)
#define IS_LOG_WARN(...)		::Insight::Core::Logger::GetClientFileLogger()->warn(__VA_ARGS__)
#define IS_LOG_ERROR(...)		::Insight::Core::Logger::GetClientFileLogger()->error(__VA_ARGS__)
#define IS_LOG_FATEL(...)		::Insight::Core::Logger::GetClientFileLogger()->critical(__VA_ARGS__)
#else 

#if defined(IS_CORE)
#define IS_LOG_CORE_TRACE(...) IS_UNUSED(0)
#define IS_LOG_CORE_INFO(...) IS_UNUSED(0)
#define IS_LOG_CORE_WARN(...) IS_UNUSED(0)
#define IS_LOG_CORE_ERROR(...) IS_UNUSED(0)
#define IS_LOG_CORE_FATEL(...) IS_UNUSED(0)
#else 
#define IS_LOG_CORE_TRACE(...) IS_UNUSED(0)
#define IS_LOG_CORE_INFO(...) IS_UNUSED(0)
#define IS_LOG_CORE_WARN(...) IS_UNUSED(0)
#define IS_LOG_CORE_ERROR(...) IS_UNUSED(0)
#define IS_LOG_CORE_FATEL(...) IS_UNUSED(0)
#endif
#define IS_LOG_TRACE(...) IS_UNUSED(0)
#define IS_LOG_INFO(...) IS_UNUSED(0)
#define IS_LOG_WARN(...) IS_UNUSED(0)
#define IS_LOG_ERROR(...) IS_UNUSED(0)
#define IS_LOG_FATEL(...) IS_UNUSED(0)
#endif
#pragma warning(pop)