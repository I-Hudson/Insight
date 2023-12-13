#pragma once

#include "Runtime/Defines.h"
#include "Core/Singleton.h"
#include "Serialisation/ISerialisable.h"

#include <string>

namespace Insight
{
    namespace Runtime
    {
        struct IS_RUNTIME RuntimeSettings : Core::Singleton<RuntimeSettings>, Serialisation::ISerialisable
        {
            IS_SERIALISABLE_H(RuntimeSettings)

            std::string InitialWorldPath;
            u32 EngineVersion;
        };
    }

    OBJECT_SERIALISER(Runtime::RuntimeSettings, 1,
        SERIALISE_PROPERTY(std::string, InitialWorldPath, 1, 0)
        SERIALISE_PROPERTY(u32, EngineVersion, 1, 0)
    )
}