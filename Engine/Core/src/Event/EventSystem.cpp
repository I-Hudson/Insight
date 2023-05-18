#include "Event/EventSystem.h"

#include "Core/Logger.h"

#include "Algorithm/Vector.h"

namespace Insight
{
	namespace Core
	{
		void EventSystem::AddEventListener(void* object, EventType eventType, EventFunc func)
		{
			std::lock_guard eventListenerLock(m_eventListenersLock);
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
			std::lock_guard eventListenerLock(m_eventListenersLock);
			auto& eventFuncItr = m_eventListeners[eventType];
			if (auto& eventItr = eventFuncItr.find(object); 
				eventItr != eventFuncItr.end())
			{
				eventFuncItr.erase(object);
			}
		}

		void EventSystem::DispatchEvent(RPtr<Event> e)
		{
			std::lock_guard queuedEventsLock(m_queuedEventsLock);
			m_queuedEvents.push_back(std::move(e));
		}

		void EventSystem::DispatchEventNow(RPtr<Event> e)
		{
			std::lock_guard eventListenerLock(m_eventListenersLock);
			auto const& eventFuncItr = m_eventListeners[e->GetEventType()];
			for (auto const& func : eventFuncItr)
			{
				func.second(*e.Get());
			}
		}

		void EventSystem::Update()
		{
			std::vector<RPtr<Event>> eventsToEvaluate;
			{
				std::lock_guard queuedEventsLock(m_queuedEventsLock);
				eventsToEvaluate = std::move(m_queuedEvents);
			}
			DiscardOutOfDateEvente(eventsToEvaluate);

			for (const RPtr<Event>& event : eventsToEvaluate)
			{
				auto const& eventFuncItr = m_eventListeners[event->GetEventType()];
				for (auto const& func : eventFuncItr)
				{
					func.second(*event.Get());
				}
			}
		}

		void EventSystem::DiscardOutOfDateEvente(std::vector<RPtr<Event>>& events)
		{
			std::unordered_map<EventType, std::vector<RPtr<Event>>> eventsToRemove;
			for (int i = static_cast<int>(events.size()) - 1; i >= 0; --i)
			{
				if (!EventTypeMultiplePerFrame[static_cast<u64>(events.at(i)->GetEventType())])
				{
					// Sort all events into their types. Newest to oldest.
					eventsToRemove[events.at(i)->GetEventType()].push_back(events.at(i));
				}
			}

			for (auto const& pair : eventsToRemove)
			{	
				if(pair.second.size() <= 1)
				{
					continue;
				}

				// Remove all old events. Event at index 0 is the Newest event so remove 
				// all events after that one.
				for (size_t i = 1; i < pair.second.size(); ++i)
				{
					Algorithm::VectorRemove(events, pair.second.at(i));
				}
			}
		}
	}
}