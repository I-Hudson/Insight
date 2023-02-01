#pragma once

#include <string>
#include <sstream>
#include <assert.h>

namespace Insight
{
    namespace Serialisation
    {
        template<typename T>
        struct PropertySerialiser
        {
            auto operator()(T const& v)
            {
                assert(false);
                return 0;
            }
        };

        template<typename T>
        struct PropertyDeserialiser
        { };
    }
}