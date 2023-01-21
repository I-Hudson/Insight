#pragma once

#include "Core/Defines.h"
#include "Core/TypeAlias.h"
#include "Core/Logger.h"

#include <nlohmann/json.hpp>

#include <string>
#include <sstream>
#include <vector>

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

        template<typename T>
        struct SerialiserVector
        {
            std::string operator()(std::vector<T> const& object)
            {
                SerialiserProperty<T> serialiserProperty;
                nlohmann::json json;
                for (size_t i = 0; i < object.size(); ++i)
                {
                    json.push_back(serialiserProperty(object.at(i)));
                }
                return nlohmann::to_string(json);
            }
        };
    }

    // CURRENT_VERSION must start from 1.
#define SERIALISER_BEGIN(OBJECT, CURRENT_VERSION)                           \
        static_assert(CURRENT_VERSION >= 1);                                \
        template<>                                                          \
        struct ::Insight::Serialisation::SerialiserObject<OBJECT>           \
        {                                                                   \
            std::string operator()(OBJECT const& object, bool serialise)    \
            {                                                               \
                const u32 currentVersion = CURRENT_VERSION;                 \
                nlohmann::json serialisedData;                              \

#define SERIALISER_END() return nlohmann::to_string(serialisedData); } };

#define SERIALISER_CALL(PROPERTY) std::string PPCAT(PROPERTY, SerialisedData) = PPCAT(PROPERTY, SerialiserProperty)(PPCAT(object., PROPERTY))
#define SERIALISER_METHOD(PROPERTY)                                                                                                     \
    if (PPCAT(PROPERTY, _VersionRemoved) == 0)                                                                                          \
    {                                                                                                                                   \
        SERIALISER_CALL(PROPERTY);                                                                                                      \
        if (PPCAT(PROPERTY, SerialisedData).empty()) { IS_CORE_ERROR(STRINGIZE([SerialiserObject<PROPERTY>] Unable to serialise.)); }   \
        serialisedData[STRINGIZE(PROPERTY)] = PPCAT(PROPERTY, SerialisedData);                                                      \
    }                                                                                                                                   \
    else                                                                                                                                \
    {                                                                                                                                   \
        serialisedData[STRINGIZE(PROPERTY)] = "REMOVED";                                                                                \
    }

    // Marco magic. Just stepup a SerialiserProperty for the type. Then try and serialise the property.
#define SERIALISER_OBJECT(TYPE, PROPERTY, VERSION_ADDED, VERSION_REMOVED)                                                                   \
        SerialiserProperty<TYPE> PPCAT(PROPERTY, SerialiserProperty);                                                                       \
        const u32  PPCAT(PROPERTY, _VersionRemoved) = VERSION_REMOVED;                                                                      \
        SERIALISER_METHOD(PROPERTY);

#define SERIALISER_VECTOR(TYPE, PROPERTY, VERSION_ADDED, VERSION_REMOVED)                                                                   \
        SerialiserVector<TYPE> PPCAT(PROPERTY, SerialiserProperty);                                                                         \
        const u32  PPCAT(PROPERTY, _VersionRemoved) = VERSION_REMOVED;                                                                      \
        SERIALISER_METHOD(PROPERTY);


#define OBJECT_SERIALISER(ObjectType, CurrentVersion, ...) \
        struct PPCAT(ObjectType, _ObjectSerialiser) \
        { \
            \
        }
#define PROPERTY_SERIALISE(ObjectType, Property, VersionAdded, VersionRemoved)
}