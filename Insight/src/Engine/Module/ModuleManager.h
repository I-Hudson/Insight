#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Templates/TSingleton.h"
#include "Engine/Memory/MemoryManager.h"
#include "Engine/Module/Module.h"

#include <type_traits>

	namespace Module
	{
		class IS_API ModuleManager : public TSingleton<ModuleManager>, public Module
		{
		public:
			ModuleManager();
			virtual ~ModuleManager() override;

			virtual void Update(const float& deltaTime) override;

			template <typename T, typename... Args>
			SharedPtr<T> AddModule(Args... args);
			template <typename T>
			void RemoveModule();

			template<typename T>
			SharedPtr<T> GetModule();

			const std::unordered_map<std::string, SharedPtr<Module>> GetModules() const { return m_modules; }

		private:
			bool Exists(const std::string& moduleName);

		private:
			std::unordered_map<std::string, SharedPtr<Module>> m_modules;
		};

		template<typename T, typename ...Args>
		inline SharedPtr<T> ModuleManager::AddModule(Args... args)
		{
			std::string typeId = typeid(T).name();
			if (!Exists(typeId))
			{
				SharedPtr<T> newModule = CreateSharedPtr<T>(std::forward<Args>(args)...);
				StaticPointerCast<Module>(newModule)->OnCreate();
				m_modules[typeId] = newModule;

				return newModule;
			}
			return DynamicPointerCast<T>(m_modules[typeId]);
		}

		template<typename T>
		inline void ModuleManager::RemoveModule()
		{
			std::string typeId = typeid(T).name();
			if (Exists(typeId))
			{
				auto it = m_modules[typeId];
				*it.reset();
				m_modules.erase(it);
			}
		}

		template<typename T>
		inline SharedPtr<T> ModuleManager::GetModule()
		{
			std::string typeId = typeid(T).name();
			if (Exists(typeId))
			{
				return DynamicPointerCast<T>(m_modules[typeId]);
			}
			return {};
		}
	}