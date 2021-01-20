#pragma once

#include "Insight/Core/Core.h"
#include "Event.h"
#include <typeinfo>
#include <functional>

	class IS_API EventManager
	{
	public:
		using EventFunc = std::function<void(const Event&)>;

		static void Bind(const EventType type, const std::string& className, EventFunc listener);
		static void Unbind(const EventType type, const std::string& className);
		template<typename T>
		static void Dispatch(const EventType type, T event);

	private:
		static EventManager& Get()
		{
			static EventManager sInstance;
			return sInstance;
		}

		static bool HasBoundFunction(const EventType type, const std::string& className);

	private:
		struct Listener
		{
			EventFunc Func;
			size_t Hascode;
		};

		std::unordered_map<EventType, std::vector<Listener>> m_listeners;
	};

	template<typename T>
	inline void EventManager::Dispatch(const EventType type, T event)
	{
		for (auto it = Get().m_listeners[type].begin(); it != Get().m_listeners[type].end(); ++it)
		{
			(*it).Func(event);
		}
	}

#define BIND_FUNC(func, classIns) (std::bind(&func, classIns, std::placeholders::_1))

#define REG_EVENT_HANDLE(type, func) EventManager::Bind(type, typeid(this).name(), BIND_FUNC(func, this))
#define UNREG_EVENT_HANDLE(type) EventManager::Unbind(type, typeid(this).name())

#define EVENT_DISPATCH(type, e) EventManager::Dispatch(type, e)
