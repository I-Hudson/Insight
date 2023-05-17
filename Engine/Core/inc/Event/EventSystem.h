#pragma once

#include "Core/Singleton.h"
#include "Core/ISysytem.h"
#include "Event/Event.h"
#include "Core/Memory.h"

#include <string>
#include <unordered_map>
#include <functional>

namespace Insight
{
	namespace Core
	{
		///using EventFunc = void(*)(Event& e);
		using EventFunc = std::function<void(Event& e)>;

		class IS_CORE EventSystem : public Singleton<EventSystem>, public Core::ISystem
		{
			THREAD_SAFE;
		public:
			IS_SYSTEM(EventSystem);

			void AddEventListener(void* object, EventType eventType, EventFunc func);
			void RemoveEventListener(void* object, EventType eventType);
			
			void DispatchEvent(RPtr<Event> e);
			void DispatchEventNow(RPtr<Event> e);

			void Update();

		private:
			/// @brief Take a vector of events and remove duplicate events of the same type.
			// E.G: Two window size changes. The first size change is no longer up to date so 
			// should be discarded.
			void DiscardOutOfDateEvente(std::vector<RPtr<Event>>& events);

		private:
			std::mutex m_eventListenersLock;
			std::unordered_map<EventType, std::unordered_map<void*, EventFunc>> m_eventListeners;
			std::mutex m_queuedEventsLock;
			std::vector<RPtr<Event>> m_queuedEvents;
		};
	}
}