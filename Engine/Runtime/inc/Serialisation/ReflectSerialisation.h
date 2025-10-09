#pragma once

#include "Serialisation/PropertySerialiser.h"

#include <Reflect/Reflect.h>

namespace Insight
{
    namespace Serialisation
    {
        template<>
        struct PropertySerialiser<Reflect::Type>
        {
            std::string operator()(Reflect::Type const& v)
            {
                return std::string(v.GetTypeId().GetTypeName());
            }
        };

        template<>
        struct PropertyDeserialiser<Reflect::Type>
        {
            using InType = std::string;
            Reflect::Type operator()(std::string const& data) const
            {
                Reflect::TypeId typeId(data);
                return Reflect::TypeInfoRegistry::Instance().GetTypeInfo(typeId).GetType();
            }
        };
    }
}