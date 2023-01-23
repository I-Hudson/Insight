#pragma once

#include "Serialisation/SerialiserBase.h"
#include "Serialisation/DeserialiserBasicTypes.h"

namespace Insight
{
#define SERIALISE_CALL_PARENT_OBJECT(OBJECT_TYPE, OBJECT_TYPE_NAME)\
        nlohmann::json PPCAT(OBJECT_TYPE_NAME, SerialisedData) = PPCAT(OBJECT_TYPE_NAME, SerialiserProperty).SerialiseToJsonObject(static_cast<OBJECT_TYPE const&>(object))
#define SERIALISE_CALL(OBJECT_TYPE, PROPERTY) OBJECT_TYPE PPCAT(PROPERTY, SerialisedData) = PPCAT(PROPERTY, SerialiserProperty)(PPCAT(object., PROPERTY))

#define SERIALISE_METHOD(OBJECT_TYPE, PROPERTY, CALL)\
    if (PPCAT(PROPERTY, _VersionRemoved) == 0)\
    {\
        CALL;\
        if (PPCAT(PROPERTY, SerialisedData).empty()) { IS_CORE_ERROR(STRINGIZE([SerialiserObject<PROPERTY>] Unable to serialise.)); }\
        serialisedData[STRINGIZE(PROPERTY)] = PPCAT(PROPERTY, SerialisedData);\
    }\
    else\
    {\
        serialisedData[STRINGIZE(PROPERTY)] = "REMOVED";\
    }

    /// Serialise include another OBJECT_SERIALISER.
#define SERIALISE_PARENT(OBJECT_TYPE, OBJECT_TYPE_NAME, VERSION_ADDED, VERSION_REMOVED)\
        static_assert(std::is_base_of_v<OBJECT_TYPE, ObjectType> && "[Object does not inherit]");\
        ::Insight::Serialisation::SerialiserObject<OBJECT_TYPE> PPCAT(OBJECT_TYPE_NAME, SerialiserProperty);\
        const u32 PPCAT(OBJECT_TYPE_NAME, _VersionRemoved) = VERSION_REMOVED;\
        SERIALISE_METHOD(OBJECT_TYPE, OBJECT_TYPE_NAME, SERIALISE_CALL_PARENT_OBJECT(OBJECT_TYPE, OBJECT_TYPE_NAME));

    // Marco magic. Just stepup a SerialiserProperty for the type. Then try and serialise the property.
#define SERIALISE_OBJECT(OBJECT_TYPE, PROPERTY, VERSION_ADDED, VERSION_REMOVED)\
        ::Insight::Serialisation::PropertySerialiser<OBJECT_TYPE> PPCAT(PROPERTY, SerialiserProperty);\
        const u32 PPCAT(PROPERTY, _VersionRemoved) = VERSION_REMOVED;\
        SERIALISE_METHOD(OBJECT_TYPE, PROPERTY, SERIALISE_CALL(std::string, PROPERTY));

#define SERIALISE_VECTOR(OBJECT_TYPE, PROPERTY, VERSION_ADDED, VERSION_REMOVED)\
        ::Insight::Serialisation::VectorSerialiser<OBJECT_TYPE> PPCAT(PROPERTY, SerialiserProperty);\
        const u32 PPCAT(PROPERTY, _VersionRemoved) = VERSION_REMOVED;\
        SERIALISE_METHOD(std::vector<OBJECT_TYPE>, PROPERTY, SERIALISE_CALL(std::vector<std::string>, PROPERTY));


#define SERIALISE_FUNC(OBJECT_TYPE, CURRENT_VERSION, ...)\
        std::string Serialise(OBJECT_TYPE const& object)\
        {\
            const u32 currentVersion = CURRENT_VERSION;\
            nlohmann::json serialisedData;\
            serialisedData["SERIALISED_VERSION"] = currentVersion;\
            __VA_ARGS__\
            return serialisedData.dump();\
        }\
        nlohmann::json SerialiseToJsonObject(OBJECT_TYPE const& object)\
        {\
            const u32 currentVersion = CURRENT_VERSION; \
            nlohmann::json serialisedData; \
            serialisedData["SERIALISED_VERSION"] = currentVersion; \
            __VA_ARGS__\
            return serialisedData; \
        }\

#define OBJECT_SERIALISER(OBJECT_TYPE, CURRENT_VERSION, ...)\
static_assert(CURRENT_VERSION >= 1);\
        template<>\
        struct ::Insight::Serialisation::SerialiserObject<OBJECT_TYPE>\
        {\
            using ObjectType = OBJECT_TYPE;\
            SERIALISE_FUNC(OBJECT_TYPE, CURRENT_VERSION, __VA_ARGS__);\
        };


    //===========================================================
    // Deserialise
    //===========================================================

#define DESERIALISE_CALL_PARENT(OBJECT_TYPE, PROPERTY) PPCAT(PROPERTY, DeserialiserProperty).DeserialiseToJsonObject(json[STRINGIZE(PROPERTY)], object)
#define DESERIALISE_CALL(OBJECT_TYPE, PROPERTY) OBJECT_TYPE PPCAT(PROPERTY, DeserialisedData) = PPCAT(PROPERTY, DeserialiserProperty)(json[STRINGIZE(PROPERTY)])
#define DESERIALISE_SET_OFFSET(OBJECT_TYPE, PROPERTY)\
        *((OBJECT_TYPE*)(((Byte*)(OBJECT_TYPE*)object) + offsetof(ObjectType, PROPERTY))) = PPCAT(PROPERTY, DeserialisedData)

#define DESERIALISE_PARENT(OBJECT_TYPE, OBJECT_TYPE_NAME, VERSION_ADDED, VERSION_REMOVED)\
        static_assert(std::is_base_of_v<OBJECT_TYPE, ObjectType>);\
        ::Insight::Serialisation::DeserialiserObject<OBJECT_TYPE> PPCAT(OBJECT_TYPE_NAME, DeserialiserProperty);\
        const u32 PPCAT(OBJECT_TYPE_NAME, _VersionRemoved) = VERSION_REMOVED;\
        DESERIALISE_CALL_PARENT(OBJECT_TYPE, OBJECT_TYPE_NAME);\

#define DESERIALISE_OBJECT(OBJECT_TYPE, PROPERTY, VERSION_ADDED, VERSION_REMOVED)\
        ::Insight::Serialisation::PropertyDeserialiser<OBJECT_TYPE> PPCAT(PROPERTY, DeserialiserProperty);\
        DESERIALISE_CALL(OBJECT_TYPE, PROPERTY);\
        DESERIALISE_SET_OFFSET(OBJECT_TYPE, PROPERTY);

#define DESERIALISE_VECTOR(OBJECT_TYPE, PROPERTY, VERSION_ADDED, VERSION_REMOVED)\
        ::Insight::Serialisation::VectorDeserialiser<OBJECT_TYPE> PPCAT(PROPERTY, DeserialiserProperty);\
        DESERIALISE_CALL(std::vector<OBJECT_TYPE>, PROPERTY);\
        DESERIALISE_SET_OFFSET(std::vector<OBJECT_TYPE>, PROPERTY);

#define DESERIALISE_FUNC(OBJECT_TYPE, CURRENT_VERSION, ...)\
        void Deserialise(std::string const& data, OBJECT_TYPE* object = nullptr)\
        {\
            const u32 currentVersion = CURRENT_VERSION;\
            nlohmann::json json;\
            json = nlohmann::json::parse(data);\
            const u32 serialisedVersion = json["SERIALISED_VERSION"];\
            __VA_ARGS__\
        }\
        void DeserialiseToJsonObject(nlohmann::json const& json, OBJECT_TYPE* object = nullptr)\
        {\
            const u32 currentVersion = CURRENT_VERSION; \
            const u32 serialisedVersion = json["SERIALISED_VERSION"];\
            __VA_ARGS__\
        }\

#define OBJECT_DESERIALISER(OBJECT_TYPE, CURRENT_VERSION, ...)                  \
static_assert(CURRENT_VERSION >= 1);                                            \
        template<>                                                              \
        struct ::Insight::Serialisation::DeserialiserObject<OBJECT_TYPE>        \
        {                                                                       \
            using ObjectType = OBJECT_TYPE;                                     \
            DESERIALISE_FUNC(OBJECT_TYPE, CURRENT_VERSION, __VA_ARGS__);        \
        };
}

/*
    NOTES:
    Just some general notes on serialisation.

    Idea1:
    Use a pure virtual base class ISerialiable which defines two virtual methods, Serialise/Deserialise which are overrisen 
    in derived classes. This is a classic technique.
    Pros: 
    - Easy to implemeant.

    Cons:
    - Versioning between two different stats requires each function to check the current version and the saved one.
    - Would be writing a lot of duplicate code. If saving a vector you would have to write the same code again and again.
    
    Idea2:
    Generate via macros a templated object which would define what to save. This would have two template types, one;
    ObjectSerialiser and the other ObjectDeserilaiser. These objects would handle the serialising and deserialising of thigs.
    You could use stringstreams for this. But I would add a PropertySerialiser<T> struct which would handle converting from T to std::string 
    to be saved.
    This is similar to what a AAA engine I know of does. This would require some template specialisations to be made for user defined classes.
    Pros:
    - Would reduced code to write.
    - Could remove the need to worry about versioning as the generated object would know what version variables were added and removed.

    Cons:
    - Complex macro structure needed to create the object.
    - Not easy to debug.
    - Would require template specialisations.

    I'm trying Idea2 as it would be nice to just write and not worry about anything like versions or having to write the same code to serialise a GUID
    each time for example.

   OBJECT_SERIALISER(Editor::ProjectInfo, 2,
        SERIALISE_PARENT(Editor::BaseProjectInfo, BaseProjectInfo, 2, 0)
        SERIALISE_OBJECT(std::string, ProjectPath, 1, 0)
        SERIALISE_OBJECT(std::string, ProjectName, 1, 0)
        SERIALISE_OBJECT(u32, ProjectVersion, 1, 0)
        SERIALISE_OBJECT(bool, IsOpen,         1, 0)
        SERIALISE_VECTOR(int, IntTestArray,   1, 0)
        );
    OBJECT_DESERIALISER(Editor::ProjectInfo, 2,
        DESERIALISE_PARENT(Editor::BaseProjectInfo, 2, 0)
        DESERIALISE_OBJECT(std::string, ProjectPath, 1, 0)
        DESERIALISE_OBJECT(std::string, ProjectName, 1, 0)
        DESERIALISE_OBJECT(u32, ProjectVersion, 1, 0)
        DESERIALISE_OBJECT(bool, IsOpen, 1, 0)
        DESERIALISE_VECTOR(int, IntTestArray, 1, 0)
    );

    Initial testing with Idea 2 works.
    Notes: A lot of code hidden by the macros used. Not nice to debug or try and work out what is happening. This would be something to look
    into for the future.
    ProertySerialiser and VectorSerialiser must return std::string and std::vector<std::string> respectively. On a technical note this does 
    increase file size as for example serialising a single int isn't 4 bytes but 6 bytes. 4 bytes for the value and an additional 2 for each 
    speech mark around the value as it is serialised as a string.

    Would be nice to look into replacing the code hidden by the macros to template classes instead. Main reason for this would be 
    maybe the serialised values could be saved as a json compatible format and checks could be made within the templated class/struct.
    Doing this would also give a clearer idea of what is going on as it would be more readable.
*/