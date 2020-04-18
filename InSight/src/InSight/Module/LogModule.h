#pragma once

#include "Insight/Core.h"
#include "Insight/Module/Module.h"

#include <memory>
#include "spdlog/spdlog.h"

namespace Insight
{
	namespace Module
	{
		class IS_API LogModule : public Module
		{
		public:
			LogModule();
			virtual ~LogModule() override;

			virtual void Startup(const ModuleStartupData& startupData) override;
			virtual void Shutdown() override;
			virtual void Update(const float& deltaTime) override;

			inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
			inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

		private:
			static std::shared_ptr<spdlog::logger> s_CoreLogger;
			static std::shared_ptr<spdlog::logger> s_ClientLogger;

			static bool s_initalised;
		};
	}
}

#define IS_CORE_TRACE(...)   ::Insight::Module::LogModule::GetCoreLogger()->trace(__VA_ARGS__)
#define IS_CORE_INFO(...)    ::Insight::Module::LogModule::GetCoreLogger()->info(__VA_ARGS__)
#define IS_CORE_WARN(...)    ::Insight::Module::LogModule::GetCoreLogger()->warn(__VA_ARGS__)
#define IS_CORE_ERROR(...)   ::Insight::Module::LogModule::GetCoreLogger()->error(__VA_ARGS__)
#define IS_CORE_FATEL(...)   ::Insight::Module::LogModule::GetCoreLogger()->error(__VA_ARGS__)

#define IS_TRACE(...)		 ::Insight::Module::LogModule::GetClientLogger()->trace(__VA_ARGS__)
#define IS_INFO(...)		 ::Insight::Module::LogModule::GetClientLogger()->info(__VA_ARGS__)
#define IS_WARN(...)		 ::Insight::Module::LogModule::GetClientLogger()->warn(__VA_ARGS__)
#define IS_ERROR(...)		 ::Insight::Module::LogModule::GetClientLogger()->error(__VA_ARGS__)
#define IS_FATEL(...)		 ::Insight::Module::LogModule::GetClientLogger()->error(__VA_ARGS__)
