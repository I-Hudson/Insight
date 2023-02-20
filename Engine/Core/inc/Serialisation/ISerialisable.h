#pragma once

#include "Core/Defines.h"

namespace Insight
{
    namespace Serialisation
    {
        class ISerialiser;

        class IS_CORE ISerialisable
        {
        public:
            virtual void BeforeSerialise(ISerialiser* serialiser) { }
            virtual void Serialise(ISerialiser* serialiser) = 0;
            virtual void AfterSerialise(ISerialiser* serialiser) { }

            virtual void BeforeDeserialise(ISerialiser* serialiser) { }
            virtual void Deserialise(ISerialiser* serialiser) = 0;
            virtual void AfterDeserialise(ISerialiser* serialiser) { }
        };
    }
}
