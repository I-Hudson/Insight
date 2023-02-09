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

        template<typename TypeSerialiser, typename OutType>
        struct PropertyDeserialiser
        {
            using InType = u8;
            OutType operator()(InType const v) const
            {
                if constexpr (std::is_enum_v<OutType>)
                {
                    return static_cast<OutType>(v);
                }
                else
                {
                    assert(false);
                }
                return OutType();
            }
        };
    }
}