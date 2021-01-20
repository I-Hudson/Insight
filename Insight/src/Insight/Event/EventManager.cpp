#include "ispch.h"
#include "EventManager.h"

#include "Insight/Core/Log.h"

	void EventManager::Bind(const EventType type, const std::string& className, EventFunc listener)
	{
		if (HasBoundFunction(type, className))
		{
			IS_CORE_WARN("Event '{0}' already bound to a function in '{1}', please Unbind before trying to bind again.", EventTypeToString(type), className);
			return;
		}

		Get().m_listeners[type].push_back(Listener
			{
				listener,
				std::hash<std::string>{}(className)
			});
	}
	void EventManager::Unbind(const EventType type, const std::string& className)
	{
		for (auto it = Get().m_listeners[type].begin(); it != Get().m_listeners[type].end(); ++it)
		{
			if ((*it).Hascode == std::hash<std::string>{}(className))
			{
				Get().m_listeners[type].erase(it);
				return;
			}
		}
	}

	bool EventManager::HasBoundFunction(const EventType type, const std::string& className)
	{
		for (auto it = Get().m_listeners[type].begin(); it != Get().m_listeners[type].end(); ++it)
		{
			if ((*it).Hascode == std::hash<std::string>{}(className))
			{
				return true;
			}
		}
		return false;
	}