#pragma once

#include "Insight/Core.h"
#include "Insight/Templates/TSingleton.h"
#include "Insight/Memory/MemoryManager.h"
#include "Insight/Module/Module.h"

#include <type_traits>

namespace Insight
{
	namespace Module
	{
		class IS_API ModuleManager : public TSingleton<ModuleManager>, public Module
		{
		public:
			ModuleManager(ModuleStartupData& startupData = ModuleStartupData());
			virtual ~ModuleManager() override;

			virtual void Update(const float& deltaTime) override;

			template <typename T>
			T* AddModule(ModuleStartupData& moduleData = ModuleStartupData());
			template <typename T>
			void RemoveModule();

			template<typename T>
			Module* GetModule();

		private:
			bool Exists(const char* moduleName);

		private:
			std::set<Module*> m_modules;
		};


		template<typename T>
		inline T* ModuleManager::AddModule(ModuleStartupData& moduleData)
		{
			if (!Exists(typeid(T).name()))
			{
				T* newModule = NEW_ON_STACK(T, moduleData);

				m_modules.insert(newModule);

				return newModule;
			}

			for (auto it = m_modules.begin(); it != m_modules.end(); ++it)
			{
				if (typeid(*it).name() == typeid(T).name())
				{
					Module* m = *it;
					return static_cast<T*>(m);
				}
			}

			return nullptr;
		}

		template<typename T>
		inline void ModuleManager::RemoveModule()
		{
			if (Exists(typeid(T).name()))
			{
				auto it = m_modules.begin();
				for (; it < m_modules.end(); ++it)
				{
					if (typeid(T).name() == typeid((*it)).name())
					{
						break;
					}
				}
				DELETE_ON_STACK(*it);
				m_modules.erase(it);
			}
		}

		template<typename T>
		inline Module* ModuleManager::GetModule()
		{
			if (Exists(typeid(T).name()))
			{
				return m_modules.find(typeid(T).name());
			}
		}
	}
}

