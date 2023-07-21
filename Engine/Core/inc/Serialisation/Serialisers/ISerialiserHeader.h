#pragma once

#include "Serialisation/Serialiser.h"

namespace Insight
{
    namespace Serialisation
    {
        /// @brief Struct to be serialised.
        struct ISerialiserHeader
        {
            IS_SERIALISABLE_H(ISerialiserHeader);

            u8 Type;
        };
    }

    OBJECT_SERIALISER(Serialisation::ISerialiserHeader, 1, 
        SERIALISE_PROPERTY(u8, Type, 1, 0)
        )
}