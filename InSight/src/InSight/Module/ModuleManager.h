#pragma once

#include "Insight/Core.h"
#include "Insight/Module/Module.h"

#include <typeinfo>
#include <unordered_map>

namespace Insight
{
	namespace Module
	{
		class IS_API ModuleManager : public Module
		{
		public:
			ModuleManager();
			virtual ~ModuleManager() override;

			virtual void Startup(const ModuleStartupData& startupData = ModuleStartupData()) override;
			virtual void Shutdown() override;
			virtual void Update(const float& deltaTime) override;

			template <typename T>
			void AddModule(ModuleStartupData& moduleData = ModuleStartupData());
			template <typename T>
			void RemoveModule();

			template<typename T>
			Module* GetModule();

		private:
			bool Exists(const char* moduleName);

		private:
			std::unordered_map<const char*, Module*> m_modules;
		};


		template<typename T>
		inline void ModuleManager::AddModule(ModuleStartupData& moduleData)
		{
			if (!Exists(typeid(T).name()))
			{
				T* newModule = new T();
				auto castModule = static_cast<Module*>(newModule);

				if (castModule != nullptr)
				{
					castModule->Startup(moduleData);
					castModule->m_manuallUpdate = moduleData.ManuallUpdate;
				}

				m_modules.insert(std::pair<const char*, Module*>(typeid(T).name(), newModule));
			}
		}

		template<typename T>
		inline void ModuleManager::RemoveModule()
		{
			if (Exists(typeid(T).name()))
			{
				auto it = m_modules.find(typeid(T).name());
				delete it->second;

				m_modules.erase(it);
			}
		}

		template<typename T>
		inline Module* ModuleManager::GetModule()
		{
			if (Exists(typeid(T).name()))
			{
				return m_modules.find(typeid(T).name())->second;
			}
		}
	}
}

