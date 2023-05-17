#pragma once

#include "Event/Event.h"
#include "Runtime/Defines.h"

#include <string>

namespace Insight
{
	namespace Runtime
	{
		class World;
	}

	struct IS_RUNTIME WorldSaveEvent : public Core::Event
	{
		WorldSaveEvent() { }
		WorldSaveEvent(Runtime::World* world)
			: World(world)
		{ }

		virtual std::string GetName() override { return "WorldSaveEvent"; }
		virtual Core::EventType GetEventType() override { return Core::EventType::WorldSave; }

		Runtime::World* World;
	};

	struct IS_RUNTIME WorldLoadEvent : public Core::Event
	{
		WorldLoadEvent() { }
		WorldLoadEvent(Runtime::World* world)
			: World(world)
		{ }

		virtual std::string GetName() override { return "WorldLoadEvent"; }
		virtual Core::EventType GetEventType() override { return Core::EventType::WorldLoad; }

		Runtime::World* World;
	};

	struct IS_RUNTIME WorldDestroyEvent : public Core::Event
	{
		WorldDestroyEvent() { }
		WorldDestroyEvent(Runtime::World* world)
			: World(world)
		{ }

		virtual std::string GetName() override { return "WorldDestroyEvent"; }
		virtual Core::EventType GetEventType() override { return Core::EventType::WorldDestroy; }

		Runtime::World* World;
	};
}