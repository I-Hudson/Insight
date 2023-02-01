#pragma once

#include "Serialisation/SerialiserBase.h"
#include "Serialisation/DeserialiserBasicTypes.h"
#include "Serialisation/PropertySerialiserTypes.h"

#include <string>

#define IS_SERIALISABLE_H(TYPE)\
        private:\
        template<typename>\
        friend struct ::Insight::Serialisation::SerialiserObject;\
        public:\
        virtual void Serialise(::Insight::Serialisation::ISerialiser* serialiser) override;

#define IS_SERIALISABLE_CPP(TYPE)\
        void TYPE::Serialise(::Insight::Serialisation::ISerialiser* serialiser)\
        {\
            ::Insight::Serialisation::SerialiserObject<TYPE> serialiserObject;\
            ::Insight::Serialisation::ISerialiser* objectSerialiser = serialiserObject.Serialise(this, serialiser->GetType());\
            serialiser->AddChildSerialiser(objectSerialiser);\
        }

#define IS_SERIALISE_FRIEND

    // Marco magic. Just stepup a SerialiserProperty for the type. Then try and serialise the property.
//#define SERIALISE_PROPERTY(OBJECT_TYPE, PROPERTY, VERSION_ADDED, VERSION_REMOVED)\
//        ::Insight::Serialisation::PropertySerialiser<OBJECT_TYPE> PPCAT(PROPERTY, SerialiserProperty);\
//        const u32 PPCAT(PROPERTY, _VersionRemoved) = VERSION_REMOVED;\
//        SERIALISE_METHOD(PROPERTY, SERIALISE_CALL(std::string, OBJECT_TYPE, PROPERTY));
//
//#define SERIALISE_PROPERTY_CAST(OBJECT_TYPE, PROPERTY, VERSION_ADDED, VERSION_REMOVED)\
//        ::Insight::Serialisation::PropertySerialiser<OBJECT_TYPE> PPCAT(PROPERTY, SerialiserProperty);\
//        const u32 PPCAT(PROPERTY, _VersionRemoved) = VERSION_REMOVED;\
//        SERIALISE_METHOD(PROPERTY, SERIALISE_CALL_CAST(std::string, OBJECT_TYPE, PROPERTY));
//
//#define SERIALISE_OBJECT(OBJECT_TYPE, PROPERTY, VERSION_ADDED, VERSION_REMOVED)\
//        ::Insight::Serialisation::SerialiserObject<OBJECT_TYPE> PPCAT(PROPERTY, SerialiserProperty);\
//        const u32 PPCAT(PROPERTY, _VersionRemoved) = VERSION_REMOVED;\
//        SERIALISE_METHOD(PROPERTY, SERIALISE_CALL_OBJECT(PROPERTY));
//
//    /// Serialise include another OBJECT_SERIALISER.
//#define SERIALISE_PARENT(OBJECT_TYPE, OBJECT_TYPE_NAME, VERSION_ADDED, VERSION_REMOVED)\
//        static_assert(std::is_base_of_v<OBJECT_TYPE, ObjectType> && "[Object does not inherit]");\
//        ::Insight::Serialisation::SerialiserObject<OBJECT_TYPE> PPCAT(OBJECT_TYPE_NAME, SerialiserProperty);\
//        const u32 PPCAT(OBJECT_TYPE_NAME, _VersionRemoved) = VERSION_REMOVED;\
//        SERIALISE_METHOD(OBJECT_TYPE_NAME, SERIALISE_CALL_PARENT_OBJECT(OBJECT_TYPE, OBJECT_TYPE_NAME));
//
//#define SERIALISE_PROPERTY_VECTOR(OBJECT_TYPE, PROPERTY, VERSION_ADDED, VERSION_REMOVED)\
//        ::Insight::Serialisation::VectorSerialiser<OBJECT_TYPE, false> PPCAT(PROPERTY, SerialiserProperty);\
//        const u32 PPCAT(PROPERTY, _VersionRemoved) = VERSION_REMOVED;\
//        SERIALISE_METHOD(PROPERTY, SERIALISE_CALL(std::vector<std::string>, std::vector<OBJECT_TYPE>, PROPERTY));
//
//#define SERIALISE_OBJECT_VECTOR(OBJECT_TYPE, PROPERTY, VERSION_ADDED, VERSION_REMOVED)\
//        ::Insight::Serialisation::VectorSerialiser<OBJECT_TYPE, true> PPCAT(PROPERTY, SerialiserProperty);\
//        const u32 PPCAT(PROPERTY, _VersionRemoved) = VERSION_REMOVED;\
//        SERIALISE_METHOD(PROPERTY, SERIALISE_CALL(nlohmann::json, OBJECT_TYPE, PROPERTY));
//
//#define SERIALISE_PROPERTY_UMAP(KEY_OBJECT_TYPE, VALUE_OBJECT_TYPE, PROPERTY, VERSION_ADDED, VERSION_REMOVED)\
//        ::Insight::Serialisation::UMapSerialiser<KEY_OBJECT_TYPE, VALUE_OBJECT_TYPE> PPCAT(PROPERTY, SerialiserProperty);\
//        const u32 PPCAT(PROPERTY, _VersionRemoved) = VERSION_REMOVED;\
//        SERIALISE_METHOD(PROPERTY, SERIALISE_CALL(nlohmann::json, std::unordered_map<KEY_OBJECT_TYPE, VALUE_OBJECT_TYPE>, PROPERTY));

//static_assert(HasSerialiserObject || HasPropertySerialiser, STRINGIZE(Unable to find either a SerialiserObject or PropertySerialiser for TYPE_SERIALISER)); \
//serialiser->AddChildSerialiser(PPCAT(PROPERTY, ObjectSerialiser).Serialise(PPCAT(object., PROPERTY)));

enum class SerialiserType
{
    Property,
    Object,
    Base,
    VectorProperty,
    VectorObject,
    UMapProperty,
    UMapObject,
};

#define SERIALISE_CALL_PROPERTY_SERIALISER(OBJECT_TYPE, PROPERTY)\
        auto SerialisedData = PropertySerialiser(PPCAT(object., PROPERTY))

// Serialise a single property. This would be thiings which only contain data for them self. 
#define SERIALISE_NAMED_PROPERTY(TYPE_SERIALISER, PROPERTY_NAME, PROPERTY, VERSION_ADDED, VERSION_REMOVED)\
        if (!serialiser->IsReadMode())\
        {\
            ::Insight::Serialisation::PropertySerialiser<TYPE_SERIALISER> PropertySerialiser; \
            const u32 VersionRemoved = VERSION_REMOVED; \
            SERIALISE_CALL_PROPERTY_SERIALISER(TYPE_SERIALISER, PROPERTY); \
            serialiser->Write(#PROPERTY_NAME, SerialisedData);\
        }\
        else\
        {\
        }

#define SERIALISE_NAMED_OBJECT(TYPE_SERIALISER, PROPERTY_NAME, PROPERTY, VERSION_ADDED, VERSION_REMOVED)\
        if (!serialiser->IsReadMode())\
        {\
            ::Insight::Serialisation::SerialiserObject<TYPE_SERIALISER> ObjectSerialiser; \
            ::Insight::Serialisation::ISerialiser* objISerialiser = ObjectSerialiser.Serialise(PPCAT(object., PROPERTY), serialiserType);\
            objISerialiser->SetName(#PROPERTY_NAME);\
            serialiser->AddChildSerialiser(objISerialiser);\
        }\
        else\
        {\
        }

#define SERIALISE_NAMED_BASE(BASE_TYPE, VERSION_ADDED, VERSION_REMOVED)\
        if (!serialiser->IsReadMode())\
        {\
            BASE_TYPE const* baseType = static_cast<BASE_TYPE const*>(&object);\
            ::Insight::Serialisation::SerialiserObject<BASE_TYPE> ObjectSerialiser; \
            ::Insight::Serialisation::ISerialiser* objISerialiser = ObjectSerialiser.Serialise(baseType, serialiserType);\
            objISerialiser->SetName(#BASE_TYPE);\
            serialiser->AddChildSerialiser(objISerialiser);\
        }\
        else\
        {\
        }

// Serialise a single property with a ProertySerialiser.
#define SERIALISE_PROPERTY(TYPE_SERIALISER, PROPERTY, VERSION_ADDED, VERSION_REMOVED) SERIALISE_NAMED_PROPERTY(TYPE_SERIALISER, PROPERTY, PROPERTY, VERSION_ADDED, VERSION_REMOVED)
// Serialise a single property with a SerialiserObject.
#define SERIALISE_OBJECT(TYPE_SERIALISER, PROPERTY, VERSION_ADDED, VERSION_REMOVED) SERIALISE_NAMED_OBJECT(TYPE_SERIALISER, PROPERTY, PROPERTY, VERSION_ADDED, VERSION_REMOVED)
// Cast the serialiser object to 'BASE_TYPE_SERIALISER' and use a SerialiserObject.
#define SERIALISE_BASE(BASE_TYPE, VERSION_ADDED, VERSION_REMOVED) SERIALISE_NAMED_BASE(BASE_TYPE, VERSION_ADDED, VERSION_REMOVED)

#define SERIALISE_FUNC(OBJECT_TYPE, CURRENT_VERSION, ...)\
    public:\
        ::Insight::Serialisation::ISerialiser* Serialise(OBJECT_TYPE const& object, ::Insight::Serialisation::SerialisationTypes serialiserType)\
        {\
            const u32 currentVersion = CURRENT_VERSION;\
            ::Insight::Serialisation::ISerialiser* serialiser = ::Insight::Serialisation::ISerialiser::Create(serialiserType);\
            if (!serialiser->IsReadMode())\
            {\
                serialiser->SetName(#OBJECT_TYPE); \
                serialiser->Write("SERIALISED_VERSION", currentVersion); \
            }\
            __VA_ARGS__\
            return serialiser;\
        }\
        ::Insight::Serialisation::ISerialiser* Serialise(OBJECT_TYPE const* object, ::Insight::Serialisation::SerialisationTypes serialiserType)\
        {\
            return Serialise(*object, serialiserType);\
        }\

#define DESERIALISE_FUNC(OBJECT_TYPE, CURRENT_VERSION, ...)\
        OBJECT_TYPE Deserialise(::Insight::Serialisation::ISerialiser* serialiser)\
        {\
            OBJECT_TYPE object;\
            ::Insight::Serialisation::SerialisationTypes serialiserType = serialiser->GetType();\
            u32 currentVersion = CURRENT_VERSION;\
            u32 serialisedVersion = 0;\
            if (serialiser->IsReadMode())\
            {\
                serialiser->Read("SERIALISED_VERSION", serialisedVersion); \
            }\
            __VA_ARGS__\
            return object;\
        }

#define OBJECT_SERIALISER(OBJECT_TYPE, CURRENT_VERSION, ...)\
        static_assert(CURRENT_VERSION >= 1);\
        template<>\
        struct ::Insight::Serialisation::SerialiserObject<OBJECT_TYPE>\
        {\
            using ObjectType = OBJECT_TYPE;\
            SERIALISE_FUNC(OBJECT_TYPE, CURRENT_VERSION, __VA_ARGS__);\
            DESERIALISE_FUNC(OBJECT_TYPE, CURRENT_VERSION, __VA_ARGS__);\
        };

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
        SERIALISE_PROPERTY(std::string, ProjectPath, 1, 0)
        SERIALISE_PROPERTY(std::string, ProjectName, 1, 0)
        SERIALISE_PROPERTY(u32, ProjectVersion, 1, 0)
        SERIALISE_PROPERTY(bool, IsOpen,         1, 0)
        SERIALISE_VECTOR(int, IntTestArray,   1, 0)
        );
    OBJECT_DESERIALISER(Editor::ProjectInfo, 2,
        DESERIALISE_PARENT(Editor::BaseProjectInfo, 2, 0)
        DESERIALISE_PROPERTY(std::string, ProjectPath, 1, 0)
        DESERIALISE_PROPERTY(std::string, ProjectName, 1, 0)
        DESERIALISE_PROPERTY(u32, ProjectVersion, 1, 0)
        DESERIALISE_PROPERTY(bool, IsOpen, 1, 0)
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