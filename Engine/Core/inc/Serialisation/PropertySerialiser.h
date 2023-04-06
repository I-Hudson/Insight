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
                if constexpr (std::is_enum_v<T>)
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

        template<typename TypeSerialiser>
        struct PropertyDeserialiser
        {
            using InType = u8;
            u8 operator()(InType const v) const
            {
                if constexpr (std::is_enum_v<TypeSerialiser>)
                {
                    return v;
                }
                else
                {
                    assert(false);
                    return 0;
                }
            }
        };
    }
}