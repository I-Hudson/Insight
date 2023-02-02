#pragma once

namespace Insight
{
    namespace Serialisation
    {
        class ISerialiser;

        class ISerialisable
        {
        public:
            virtual void Serialise(ISerialiser* serialiser) = 0;
            virtual void Deserialise(ISerialiser* serialiser) = 0;
        };
    }
}
