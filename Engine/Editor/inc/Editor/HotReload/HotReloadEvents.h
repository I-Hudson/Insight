#pragma once

#include "Event/Event.h"

#include <string>

namespace Insight::Editor
{
    struct HotReloadLibraryUnLoaded : public Core::Event
    {
        HotReloadLibraryUnLoaded() { }
		virtual std::string GetName() override { return "HotReloadLibraryUnLoaded"; }
		virtual Core::EventType GetEventType() override { return Core::EventType::HotReloadLibraryUnLoaded; }
    };
    struct HotReloadLibraryLoaded : public Core::Event
    {
        HotReloadLibraryLoaded() { }
        virtual std::string GetName() override { return "HotReloadLibraryLoaded"; }
        virtual Core::EventType GetEventType() override { return Core::EventType::HotReloadLibraryLoaded; }
    };
}