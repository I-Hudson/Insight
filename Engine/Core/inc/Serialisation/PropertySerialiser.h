#pragma once

#include "Core/TypeAlias.h"

#include <assert.h>

namespace Insight
{
    namespace Serialisation
    {
        template<typename T>
        struct PropertySerialiser
        {
            u32 operator()(T const& v)
            {
                static_assert(std::is_enum_v<T>);

                if constexpr (std::is_enum_v<T>)
                {
                    return static_cast<u32>(const_cast<T&>(v));
                }
                else
                {
                    assert(false);
                }
                return 0u;
            }
        };

        template<typename TypeSerialiser>
        struct PropertyDeserialiser
        {
            using InType = u32;
            u32 operator()(InType const v) const
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

        template<typename Type, typename Class, typename Object>
        struct MigrationDeserialiser
        {
            bool IsVoid = std::is_same_v<Type, void>;

            void operator()(Class* inClass, Object* inObject)
            {
                assert(false);
            }
        };
    }
}