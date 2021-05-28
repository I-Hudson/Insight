#pragma once

#include "Engine/Core/Common.h"
#include "Engine/Templates/TSingleton.h"
#include "Engine/Module/Module.h"
#include "Engine/Core/Templates.h"

namespace Insight::Module
{
	class ModuleManager : public Core::TSingleton<ModuleManager>, public Module
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

		const std::vector<Module*>& GetAllModules() const { return m_modules; }

	private:
		bool Exists(const Type& type);

	private:
		std::vector<Module*> m_modules;
		std::unordered_map<Type, u32> m_typeToModule;
	};

	template<typename T, typename ...Args>
	inline T* ModuleManager::AddModule(Args... args)
	{
		Type type;
		type.SetType<T>();
		if (!Exists(type))
		{
			T* modulePtr = ::New<T>(std::forward<Args>(args)...);
			u32 index = (u32)m_modules.size();
			m_typeToModule[type] = index;
			m_modules.push_back(modulePtr);
			return modulePtr;
		}
		return dynamic_cast<T*>(m_modules.at(m_typeToModule.at(type)));
	}

	template<typename T>
	inline void ModuleManager::RemoveModule()
	{
		Type type;
		type.SetType<T>();
		if (Exists(type))
		{
			auto& it = m_modules.at(m_typeToModule.at(type));
			::Delete(it);
			m_modules.erase(m_modules.begin() + m_typeToModule.at(type));
		}
	}

	template<typename T>
	inline T* ModuleManager::GetModule()
	{
		Type type;
		type.SetType<T>();
		if (Exists(type))
		{
			return dynamic_cast<T*>(m_modules.at(m_typeToModule.at(type)));
		}
		return {};
	}
}