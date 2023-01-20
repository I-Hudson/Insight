#pragma once

#include "Core/Defines.h"
#include "Core/TypeAlias.h"
#include "Core/Logger.h"

#include <nlohmann/json.hpp>

#include <string>
#include <sstream>

namespace Insight
{
    namespace Serialisation
    {
        /// @brief Empty template struct for the serialiser macros to be used to define SerialiserObjects for different types.
        /// @tparam T 
        template<typename T>
        struct SerialiserObject
        { };

        template<typename T>
        struct SerialiserProperty
        {
            std::string operator()(T const& object)
            { 
                std::stringstream ss;
                ss << object;
                if (ss.fail())
                {
                    return "";
                }
                return ss.str();
            }
        };
    }

    // CURRENT_VERSION must start from 1.
#define SERIALISER_BEGIN(OBJECT, CURRENT_VERSION)                       \
        static_assert(CURRENT_VERSION >= 1);                            \
        template<>                                                      \
        struct ::Insight::Serialisation::SerialiserObject<OBJECT>       \
        {                                                               \
            std::string operator()(OBJECT const& object)                \
            {                                                           \
                const u32 currentVersion = CURRENT_VERSION;             \
                nlohmann::json serialisedData;                          \

#define SERIALISER_END() return nlohmann::to_string(serialisedData); } };

    // Marco magic. Just step a SerialiserProperty for the type. Then try and serialise the property.
#define SERIALISER_PROPERTY(TYPE, PROPERTY, VERSION_ADDED, VERSION_REMOVED)                                                                 \
        SerialiserProperty<TYPE> PPCAT(PROPERTY, SerialiserProperty);                                                                       \
        if (VERSION_REMOVED == 0)                                                                                                           \
        {                                                                                                                                   \
            std::string PPCAT(PROPERTY, SerialisedData) = PPCAT(PROPERTY, SerialiserProperty)(PPCAT(object., PROPERTY));                    \
            if (PPCAT(PROPERTY, SerialisedData).empty()) { IS_CORE_ERROR(STRINGIZE([SerialiserObject<PROPERTY>] Unable to serialise.)); }   \
            serialisedData[STRINGIZE(PROPERTY)] = PPCAT(PROPERTY, SerialisedData);                                                          \
        }                                                                                                                                   \
        else                                                                                                                                \
        {                                                                                                                                   \
            serialisedData[STRINGIZE(PROPERTY)] = "REMOVED";                                                                                \
        }

}