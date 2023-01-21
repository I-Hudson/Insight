#pragma once

#include "Serialisation/SerialiserBase.h"
#include "Serialisation/DeserialiserBasicTypes.h"

namespace Insight
{
#define OBJECT_SERIALISER(OBJECT_TYPE, CURRENT_VERSION, ...)                    \
static_assert(CURRENT_VERSION >= 1);                                            \
        template<>                                                              \
        struct ::Insight::Serialisation::SerialiserObject<OBJECT_TYPE>          \
        {                                                                       \
            using ObejctType = OBJECT_TYPE;                                     \
             SERIALISE_FUNC(OBJECT_TYPE, CURRENT_VERSION, __VA_ARGS__);         \
        };

#define SERIALISER_CALL(PROPERTY) std::string PPCAT(PROPERTY, SerialisedData) = PPCAT(PROPERTY, SerialiserProperty)(PPCAT(object., PROPERTY))

#define SERIALISER_METHOD(OBJECT_TYPE, PROPERTY)                                                                                                                        \
    if (PPCAT(PROPERTY, _VersionRemoved) == 0)                                                                                                                          \
    {                                                                                                                                                                   \
        SERIALISER_CALL(PROPERTY);                                                                                                                                      \
        if (PPCAT(PROPERTY, SerialisedData).empty()) { IS_CORE_ERROR(STRINGIZE([SerialiserObject<PROPERTY>] Unable to serialise.)); }                                   \
        serialisedData[STRINGIZE(PROPERTY)] = PPCAT(PROPERTY, SerialisedData);                                                                                          \
    }                                                                                                                                                                   \
    else                                                                                                                                                                \
    {                                                                                                                                                                   \
        serialisedData[STRINGIZE(PROPERTY)] = "REMOVED";                                                                                                                \
    }

    // Marco magic. Just stepup a SerialiserProperty for the type. Then try and serialise the property.
#define SERIALISER_OBJECT(OBJECT_TYPE, PROPERTY, VERSION_ADDED, VERSION_REMOVED)                                                                                \
        Serialisation::PropertySerialiser<OBJECT_TYPE> PPCAT(PROPERTY, SerialiserProperty);                                                                     \
        const u32  PPCAT(PROPERTY, _VersionRemoved) = VERSION_REMOVED;                                                                                          \
        SERIALISER_METHOD(OBJECT_TYPE, PROPERTY);

#define SERIALISER_VECTOR(OBJECT_TYPE, PROPERTY, VERSION_ADDED, VERSION_REMOVED)                                                                                \
        Serialisation::VectorSerialiser<OBJECT_TYPE> PPCAT(PROPERTY, SerialiserProperty);                                                                       \
        const u32  PPCAT(PROPERTY, _VersionRemoved) = VERSION_REMOVED;                                                                                          \
        SERIALISER_METHOD(std::vector<OBJECT_TYPE>, PROPERTY);


#define SERIALISE_FUNC(OBJECT_TYPE, CURRENT_VERSION, ...)            /*
        */std::string Serialise(OBJECT_TYPE const& object)           /*
        */{                                                          /*
        */    const u32 currentVersion = CURRENT_VERSION;            /*
        */    nlohmann::json serialisedData;                         /*
        */    __VA_ARGS__                                            /*
        */    return nlohmann::to_string(serialisedData);            /*
        */}





#define OBJECT_DESERIALISER(OBJECT_TYPE, CURRENT_VERSION, ...)                  \
static_assert(CURRENT_VERSION >= 1);                                            \
        template<>                                                              \
        struct ::Insight::Serialisation::DeserialiserObject<OBJECT_TYPE>        \
        {                                                                       \
            using ObejctType = OBJECT_TYPE;                                     \
            DESERIALISE_FUNC(OBJECT_TYPE, CURRENT_VERSION, __VA_ARGS__);        \
        };
        //*static_cast<OBJECT_TYPE*>()(static_cast<Byte*>(&returnObject) + offsetof(ObejctType, PROPERTY)) = PPCAT(PROPERTY, DeserialisedData)
#define DESERIALISER_CALL(OBJECT_TYPE, PROPERTY) OBJECT_TYPE PPCAT(PROPERTY, DeserialisedData) = PPCAT(PROPERTY, DeserialiserProperty)(json[STRINGIZE(PROPERTY)])
#define DESERIALISER_SET_OFFSET(OBJECT_TYPE, PROPERTY) *(OBJECT_TYPE*)(((Byte*)(&returnObject) + offsetof(ObejctType, PROPERTY))) = PPCAT(PROPERTY, DeserialisedData)

#define DESERIALISER_OBJECT(OBJECT_TYPE, PROPERTY, VERSION_ADDED, VERSION_REMOVED)\
        Serialisation::PropertyDeserialiser<OBJECT_TYPE> PPCAT(PROPERTY, DeserialiserProperty);\
        DESERIALISER_CALL(OBJECT_TYPE, PROPERTY);\
        DESERIALISER_SET_OFFSET(OBJECT_TYPE, PROPERTY);

#define DESERIALISER_VECTOR(OBJECT_TYPE, PROPERTY, VERSION_ADDED, VERSION_REMOVED)\
        Serialisation::VectorDeserialiser<OBJECT_TYPE> PPCAT(PROPERTY, DeserialiserProperty);\
        DESERIALISER_CALL(std::vector<OBJECT_TYPE>, PROPERTY);\
        DESERIALISER_SET_OFFSET(std::vector<OBJECT_TYPE>, PROPERTY);

#define DESERIALISE_FUNC(OBJECT_TYPE, CURRENT_VERSION, ...)\
        OBJECT_TYPE Deserialise(std::string const& data)\
        {\
            OBJECT_TYPE returnObject;\
            nlohmann::json json;\
            json = nlohmann::json::parse(data);\
            __VA_ARGS__\
            return returnObject;\
        }
}