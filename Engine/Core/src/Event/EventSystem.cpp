#include "Event/EventSystem.h"

#include "Core/Logger.h"

namespace Insight
{
	namespace Core
	{
		void EventSystem::AddEventListener(void* object, EventType eventType, EventFunc func)
		{
			auto& eventFuncItr = m_eventListeners[eventType];
			if (eventFuncItr.find(object) == eventFuncItr.end())
			{
				eventFuncItr[object] = func;
			}
			else
			{
				IS_CORE_ERROR(R"([EventSystem::AddEventListener] Trying to set an event func for event type '{}'. But one 
					already exists)", (int)eventType);
				return;
			}

		}

		void EventSystem::RemoveEventListener(void* object, EventType eventType)
		{
			auto eventFuncItr = m_eventListeners[eventType];
			if (auto eventItr = eventFuncItr.find(object); eventItr != eventFuncItr.end())
			{
				eventFuncItr.erase(object);
			}
		}

		void EventSystem::DispatchEvent(RPtr<Event> e)
		{
			m_queuedEvents.push_back(std::move(e));
		}

		void EventSystem::Update()
		{
			for (const RPtr<Event>& event : m_queuedEvents)
			{
				auto const& eventFuncItr = m_eventListeners[event->GetEventType()];
				for (auto const& func : eventFuncItr)
				{
					func.second(*event.Get());
				}
			}
			m_queuedEvents.clear();
		}
	}
}