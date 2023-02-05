#pragma once

#include "Core/TypeAlias.h"

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
                if (std::is_enum_v<T>)
                {
                    return static_cast<int>(const_cast<T&>(v));
                }
                else
                {
                    assert(false);
                }
                return 0;
            }
        };

        template<typename T>
        struct PropertyDeserialiser
        {
            using InType = u8;
            using OutType = u8;
            OutType operator()(InType const data)
            {
                assert(false);
                return data;
            }
        };
    }
}