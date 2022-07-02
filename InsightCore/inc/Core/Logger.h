#pragma once

#include "Core/Defines.h"
#include "spdlog/spdlog.h"

namespace Insight
{
	namespace Core
	{
		class IS_CORE Logger
		{
		public:
			static void Init();

			inline static std::shared_ptr<spdlog::logger>& GetCoreFileLogger() { return s_CoreLogger; }
			inline static std::shared_ptr<spdlog::logger>& GetClientFileLogger() { return s_ClientLogger; }

		private:
			static std::shared_ptr<spdlog::logger> s_CoreLogger;
			static std::shared_ptr<spdlog::logger> s_ClientLogger;
		};
	}
}

#pragma warning(push)
#pragma warning(disable : 4005)
#define IS_LOGGING_ENABLED
#if defined(IS_LOGGING_ENABLED)
#if defined(IS_CORE)
#define IS_CORE_TRACE(...)	::Insight::Core::Logger::GetCoreFileLogger()->trace(__VA_ARGS__)
#define IS_CORE_INFO(...)	::Insight::Core::Logger::GetCoreFileLogger()->info(__VA_ARGS__)
#define IS_CORE_WARN(...)	::Insight::Core::Logger::GetCoreFileLogger()->warn(__VA_ARGS__)
#define IS_CORE_ERROR(...)	::Insight::Core::Logger::GetCoreFileLogger()->error(__VA_ARGS__)
#define IS_CORE_FATEL(...)	::Insight::Core::Logger::GetCoreFileLogger()->critical(__VA_ARGS__)
#else 
#define IS_CORE_TRACE(...) IS_UNUSED(0)
#define IS_CORE_INFO(...) IS_UNUSED(0)
#define IS_CORE_WARN(...) IS_UNUSED(0)
#define IS_CORE_ERROR(...) IS_UNUSED(0)
#define IS_CORE_FATEL(...) IS_UNUSED(0)
#endif

#define IS_TRACE(...)		::Insight::Core::Logger::GetClientFileLogger()->trace(__VA_ARGS__)
#define IS_INFO(...)		::Insight::Core::Logger::GetClientFileLogger()->info(__VA_ARGS__)
#define IS_WARN(...)		::Insight::Core::Logger::GetClientFileLogger()->warn(__VA_ARGS__)
#define IS_ERROR(...)		::Insight::Core::Logger::GetClientFileLogger()->error(__VA_ARGS__)
#define IS_FATEL(...)		::Insight::Core::Logger::GetClientFileLogger()->critical(__VA_ARGS__)
#else 

#if defined(IS_CORE)
#define IS_CORE_TRACE(...) IS_UNUSED(0)
#define IS_CORE_INFO(...) IS_UNUSED(0)
#define IS_CORE_WARN(...) IS_UNUSED(0)
#define IS_CORE_ERROR(...) IS_UNUSED(0)
#define IS_CORE_FATEL(...) IS_UNUSED(0)
#else 
#define IS_CORE_TRACE(...) IS_UNUSED(0)
#define IS_CORE_INFO(...) IS_UNUSED(0)
#define IS_CORE_WARN(...) IS_UNUSED(0)
#define IS_CORE_ERROR(...) IS_UNUSED(0)
#define IS_CORE_FATEL(...) IS_UNUSED(0)
#endif
#define IS_TRACE(...) IS_UNUSED(0)
#define IS_INFO(...) IS_UNUSED(0)
#define IS_WARN(...) IS_UNUSED(0)
#define IS_ERROR(...) IS_UNUSED(0)
#define IS_FATEL(...) IS_UNUSED(0)
#endif
#pragma warning(pop)