#include "Core/Logger.h"
#include "Core/Memory.h"

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/base_sink.h"
#include "spdlog/sinks/basic_file_sink.h"

#include <filesystem>

namespace Insight
{
	namespace Core
	{
		std::shared_ptr<spdlog::logger> Logger::s_CoreLogger;
		std::shared_ptr<spdlog::logger> Logger::s_ClientLogger;

		void Logger::Init()
		{
			spdlog::set_pattern("%^[%T] %n: %v%$");

			constexpr const char* InsightFile = "Insight.txt";
			constexpr const char* AppFile = "App.txt";

			std::filesystem::remove(InsightFile);
			std::filesystem::remove(AppFile);

			std::vector<spdlog::sink_ptr> sinks;
			sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(InsightFile));
			sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());

			s_CoreLogger = std::make_shared<spdlog::logger>("Insight", begin(sinks), end(sinks));
			s_CoreLogger->set_error_handler([](const std::string& msg)
				{
					throw std::runtime_error(msg);
				});

			sinks[0] = std::make_shared<spdlog::sinks::basic_file_sink_mt>(AppFile);
			s_ClientLogger = std::make_shared<spdlog::logger>("App", begin(sinks), end(sinks));
			s_ClientLogger->set_error_handler([](const std::string& msg)
				{
					throw std::runtime_error(msg);
				});

			IS_CORE_INFO("CORE LOG INIT!");
			IS_INFO("APP LOG INIT!");
		}
	}
}