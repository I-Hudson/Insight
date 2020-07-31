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
			ModuleManager();
			virtual ~ModuleManager() override;

			virtual void Update(const float& deltaTime) override;

			template <typename T, typename... Args>
			T* AddModule(Args... args);
			template <typename T>
			void RemoveModule();

			template<typename T>
			T* GetModule();

		private:
			bool Exists(const std::string& moduleName);

		private:
			std::unordered_map<std::string, Module* > m_modules;
		};

		template<typename T, typename ...Args>
		inline T* ModuleManager::AddModule(Args... args)
		{
			std::string typeId = typeid(T).name();
			if (!Exists(typeId))
			{
				T* newModule = NEW_ON_HEAP(T, std::forward<Args>(args)...);

				m_modules[typeId] = newModule;

				return newModule;
			}
			return dynamic_cast<T*>(m_modules[typeId]);
		}

		template<typename T>
		inline void ModuleManager::RemoveModule()
		{
			std::string typeId = typeid(T).name();
			if (Exists(typeId))
			{
				auto it = m_modules[typeId];
				DELETE_ON_HEAP(*it);
				m_modules.erase(it);
			}
		}

		template<typename T>
		inline T* ModuleManager::GetModule()
		{
			std::string typeId = typeid(T).name();
			if (Exists(typeId))
			{
				return dynamic_cast<T*>(m_modules[typeId]);
			}
		}
	}
}

