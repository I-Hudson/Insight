#pragma once

#include "Core/Singleton.h"
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

		class IS_CORE EventManager : public Singleton<EventManager>
		{
		public:

			void AddEventListener(void* object, EventType eventType, EventFunc func);
			void RemoveEventListener(void* object, EventType eventType);
			
			void DispatchEvent(RPtr<Event> e);

			void Update();

		private:
			std::unordered_map<EventType, std::unordered_map<void*, EventFunc>> m_eventListeners;
			std::vector<RPtr<Event>> m_queuedEvents;
		};
	}
}