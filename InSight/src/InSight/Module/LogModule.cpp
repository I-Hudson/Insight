#include "LogModule.h"

#include "spdlog/sinks/stdout_color_sinks.h"

namespace Insight
{
	namespace Module
	{
		std::shared_ptr<spdlog::logger> LogModule::s_CoreLogger;
		std::shared_ptr<spdlog::logger> LogModule::s_ClientLogger;
		bool LogModule::s_initalised = false;

		LogModule::LogModule()
		{
		}

		LogModule::~LogModule()
		{
		}

		void LogModule::Startup(const ModuleStartupData& startupData)
		{
			spdlog::set_pattern("%^[%T] %n: %v%$");

			s_CoreLogger = spdlog::stdout_color_mt("INSIGHT");
			s_CoreLogger->set_level(spdlog::level::trace);
			s_ClientLogger = spdlog::stdout_color_mt("APP");
			s_ClientLogger->set_level(spdlog::level::trace);

			s_initalised = true;
		}

		void LogModule::Shutdown()
		{
		}

		void LogModule::Update(const float& deltaTime)
		{
		}
	}
}
