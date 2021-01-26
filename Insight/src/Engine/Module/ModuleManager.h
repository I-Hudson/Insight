#pragma once

#include "Engine/Core/Common.h"
#include "Engine/Templates/TSingleton.h"
#include "Engine/Module/Module.h"
#include "Engine/Core/Templates.h"

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
		bool Exists(const Type& type);

	private:
		std::unordered_map<Type, Module*> m_modules;
	};

	template<typename T, typename ...Args>
	inline T* ModuleManager::AddModule(Args... args)
	{
		Type type;
		type.SetType<T>();
		if (!Exists(type))
		{
			T* modulePtr = ::New<T>(std::forward<Args>(args)...);
			static_cast<Module*>(modulePtr)->OnCreate();
			m_modules[type] = modulePtr;
			return modulePtr;
		}
		return dynamic_cast<T*>(m_modules.at(type));
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
	inline T* ModuleManager::GetModule()
	{
		Type type;
		type.SetType<T>();
		if (Exists(type))
		{
			return dynamic_cast<T*>(m_modules.at(type));
		}
		return {};
	}
}