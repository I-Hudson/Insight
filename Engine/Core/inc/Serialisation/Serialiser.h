#pragma once

#include "Serialisation/SerialiserBase.h"
#include "Serialisation/DeserialiserBasicTypes.h"
#include "Serialisation/PropertySerialiserTypes.h"

#include <string>

#define IS_SERIALISABLE_FRIEND\
        private:\
        template<typename>\
        friend struct ::Insight::Serialisation::SerialiserObject;\
        template<typename, typename, typename>\
        friend struct ::Insight::Serialisation::ComplexSerialiser;\
        template<typename>\
        friend struct ::Insight::Serialisation::PropertySerialiser;\

#define IS_SERIALISABLE_H(TYPE)\
        IS_SERIALISABLE_FRIEND\
        public:\
        virtual void Serialise(::Insight::Serialisation::ISerialiser* serialiser) override;\
        virtual void Deserialise(::Insight::Serialisation::ISerialiser* serialiser) override;

#define IS_SERIALISABLE_CPP(TYPE)\
        void TYPE::Serialise(::Insight::Serialisation::ISerialiser* serialiser)\
        {\
            ::Insight::Serialisation::SerialiserObject<TYPE> serialiserObject;\
            serialiserObject.Serialise(serialiser, *this);\
        }\
        void TYPE::Deserialise(::Insight::Serialisation::ISerialiser* serialiser)\
        {\
            ::Insight::Serialisation::SerialiserObject<TYPE> serialiserObject;\
            serialiserObject.Deserialise(serialiser, *this);\
        }

// Serialise a single property. This would be thiings which only contain data for them self. 
#define SERIALISE_NAMED_PROPERTY(TYPE_SERIALISER, PROPERTY_NAME, PROPERTY, VERSION_ADDED, VERSION_REMOVED)\
        if (!serialiser->IsReadMode())\
        {\
            if(VERSION_REMOVED == 0 || (currentVersion >= VERSION_ADDED && currentVersion < VERSION_REMOVED))\
            {\
                ::Insight::Serialisation::SerialiseProperty<TYPE_SERIALISER>(serialiser, #PROPERTY_NAME, PPCAT(object., PROPERTY));\
            }\
        }\
        else\
        {\
            if(VERSION_REMOVED == 0 || (currentVersion >= VERSION_ADDED && currentVersion < VERSION_REMOVED))\
            {\
                using PropertyType = typename std::decay<decltype(PPCAT(object., PROPERTY))>::type;\
                PPCAT(object., PROPERTY) = std::move(::Insight::Serialisation::DeserialiseProperty<TYPE_SERIALISER, PropertyType>(serialiser, #PROPERTY_NAME));\
            }\
        }

#define SERIALISE_NAMED_OBJECT(TYPE_SERIALISER, PROPERTY_NAME, PROPERTY, VERSION_ADDED, VERSION_REMOVED)\
        if (!serialiser->IsReadMode())\
        {\
            if(VERSION_REMOVED == 0 || (currentVersion >= VERSION_ADDED && currentVersion < VERSION_REMOVED))\
            {\
                ::Insight::Serialisation::SerialiseObject<TYPE_SERIALISER>(serialiser, PPCAT(object., PROPERTY));\
            }\
        }\
        else\
        {\
            if(VERSION_REMOVED == 0 || (currentVersion >= VERSION_ADDED && currentVersion < VERSION_REMOVED))\
            {\
                ::Insight::Serialisation::DeserialiseObject<TYPE_SERIALISER>(serialiser, PPCAT(object., PROPERTY));\
            }\
        }

#define SERIALISE_NAMED_BASE(BASE_TYPE, VERSION_ADDED, VERSION_REMOVED)\
        if (!serialiser->IsReadMode())\
        {\
            if(VERSION_REMOVED == 0 || (currentVersion >= VERSION_ADDED && currentVersion < VERSION_REMOVED))\
            {\
                BASE_TYPE* baseType = static_cast<BASE_TYPE*>(&object);\
                ::Insight::Serialisation::SerialiseBase<BASE_TYPE>(serialiser, *baseType);\
            }\
        }\
        else\
        {\
            if(VERSION_REMOVED == 0 || (currentVersion >= VERSION_ADDED && currentVersion < VERSION_REMOVED))\
            {\
                ::Insight::Serialisation::SerialiserObject<BASE_TYPE> objectSerialiser; \
                BASE_TYPE* baseTypePtr = static_cast<BASE_TYPE*>(&object);\
                BASE_TYPE& baseTypeRef = *baseTypePtr;\
                objectSerialiser.Deserialise(serialiser, baseTypeRef);\
            }\
        }


// Serialise a single property. This would be thiings which only contain data for them self. 
#define SERIALISE_VECTOR_NAMED_PROPERTY(TYPE_SERIALISER, PROPERTY_NAME, PROPERTY, VERSION_ADDED, VERSION_REMOVED)\
        if (!serialiser->IsReadMode())\
        {\
            if(VERSION_REMOVED == 0 || (currentVersion >= VERSION_ADDED && currentVersion < VERSION_REMOVED))\
            {\
                using TVectorType = typename std::decay<decltype(*PPCAT(object., PROPERTY).begin())>::type;\
                ::Insight::Serialisation::VectorSerialiser<TVectorType, TYPE_SERIALISER, ::Insight::Serialisation::SerialiserType::Property> vectorSerialiser;\
                vectorSerialiser(serialiser, #PROPERTY_NAME, PPCAT(object., PROPERTY));\
            }\
        }\
        else\
        {\
            if(VERSION_REMOVED == 0 || (currentVersion >= VERSION_ADDED && currentVersion < VERSION_REMOVED))\
            {\
                using TVectorType = typename std::decay<decltype(*PPCAT(object., PROPERTY).begin())>::type;\
                ::Insight::Serialisation::VectorDeserialiser<TVectorType, TYPE_SERIALISER, ::Insight::Serialisation::SerialiserType::Property> vectorDeserialiser;\
                vectorDeserialiser(serialiser, #PROPERTY_NAME, PPCAT(object., PROPERTY));\
            }\
        }

/*
// https://stackoverflow.com/a/56136573
// This is fine to use in an unevaluated context
template<class T>
T& reference_to();
using TVectorType = typename std::decay<decltype(*PPCAT(object., PROPERTY).begin())>::type;\
using TVectorElementType = typename std::remove_pointer_t<std::remove_reference_t<decltype(*reference_to<TVectorType>())>>;\
*/

// https://stackoverflow.com/a/56136573
#define SERIALISE_VECTOR_NAMED_OBJECT(TYPE_SERIALISER, PROPERTY_NAME, PROPERTY, VERSION_ADDED, VERSION_REMOVED)\
        if (!serialiser->IsReadMode())\
        {\
            if(VERSION_REMOVED == 0 || (currentVersion >= VERSION_ADDED && currentVersion < VERSION_REMOVED))\
            {\
                using TVectorType = typename std::decay<decltype(*PPCAT(object., PROPERTY).begin())>::type;\
                ::Insight::Serialisation::VectorSerialiser<TVectorType, TYPE_SERIALISER, ::Insight::Serialisation::SerialiserType::Object> vectorSerialiser;\
                vectorSerialiser(serialiser, #PROPERTY_NAME, PPCAT(object., PROPERTY));\
            }\
        }\
        else\
        {\
            if(VERSION_REMOVED == 0 || (currentVersion >= VERSION_ADDED && currentVersion < VERSION_REMOVED))\
            {\
                using TVectorType = typename std::decay<decltype(*PPCAT(object., PROPERTY).begin())>::type;\
                ::Insight::Serialisation::VectorDeserialiser<TVectorType, TYPE_SERIALISER, ::Insight::Serialisation::SerialiserType::Object> vectorDeserialiser;\
                vectorDeserialiser(serialiser, #PROPERTY_NAME, PPCAT(object., PROPERTY));\
            }\
        }

// Serialise a single property. This would be thiings which only contain data for them self. 
#define SERIALISE_ARRAY_NAMED_PROPERTY(TYPE_SERIALISER, PROPERTY_NAME, PROPERTY, VERSION_ADDED, VERSION_REMOVED)\
        if (!serialiser->IsReadMode())\
        {\
            if(VERSION_REMOVED == 0 || (currentVersion >= VERSION_ADDED && currentVersion < VERSION_REMOVED))\
            {\
                using TArrayType = typename std::decay<decltype(*PPCAT(object., PROPERTY).begin())>::type;\
                ::Insight::Serialisation::ArraySerialiser<TArrayType, TYPE_SERIALISER, ::Insight::Serialisation::SerialiserType::Property> arraySerialiser;\
                arraySerialiser(serialiser, #PROPERTY_NAME, const_cast<TArrayType*>(PPCAT(object., PROPERTY).data()), PPCAT(object., PROPERTY).size());\
            }\
        }\
        else\
        {\
            if(VERSION_REMOVED == 0 || (currentVersion >= VERSION_ADDED && currentVersion < VERSION_REMOVED))\
            {\
                using TArrayType = typename std::decay<decltype(*PPCAT(object., PROPERTY).begin())>::type;\
                ::Insight::Serialisation::ArrayDeserialiser<TArrayType, TYPE_SERIALISER, ::Insight::Serialisation::SerialiserType::Property> arrayDeserialiser;\
                arrayDeserialiser(serialiser, #PROPERTY_NAME, const_cast<TArrayType*>(PPCAT(object., PROPERTY).data()), PPCAT(object., PROPERTY).size());\
            }\
        }
#define SERIALISE_ARRAY_NAMED_OBJECT(TYPE_SERIALISER, PROPERTY_NAME, PROPERTY, VERSION_ADDED, VERSION_REMOVED)\
        if (!serialiser->IsReadMode())\
        {\
            if(VERSION_REMOVED == 0 || (currentVersion >= VERSION_ADDED && currentVersion < VERSION_REMOVED))\
            {\
                using TArrayType = typename std::decay<decltype(*PPCAT(object., PROPERTY).begin())>::type;\
                ::Insight::Serialisation::ArraySerialiser<TArrayType,TYPE_SERIALISER, ::Insight::Serialisation::SerialiserType::Object> arraySerialiser;\
                arraySerialiser(serialiser, #PROPERTY_NAME, PPCAT(object., PROPERTY).data(), PPCAT(object., PROPERTY).size());\
            }\
        }\
        else\
        {\
            if(VERSION_REMOVED == 0 || (currentVersion >= VERSION_ADDED && currentVersion < VERSION_REMOVED))\
            {\
            }\
        }

#define SERIALISE_MAP_NAMED_OBJECT(KEY_SERIALISER, VALUE_SERIALISER, PROPERTY_NAME, PROPERTY, VERSION_ADDED, VERSION_REMOVED)\
        if (!serialiser->IsReadMode())\
        {\
            if(VERSION_REMOVED == 0 || (currentVersion >= VERSION_ADDED && currentVersion < VERSION_REMOVED))\
            {\
                using TMapType = std::remove_const_t<typename std::decay<decltype(PPCAT(object., PROPERTY))>::type>;\
                ::Insight::Serialisation::MapSerialiser<KEY_SERIALISER, VALUE_SERIALISER, TMapType, ::Insight::Serialisation::SerialiserType::Object> mapSerialiser;\
                mapSerialiser(serialiser, #PROPERTY_NAME, PPCAT(object., PROPERTY));\
            }\
        }\
        else\
        {\
        }

#define SERIALISE_NAMED_COMPLEX(TYPE_SERIALISER, PROPERTY_NAME, PROPERTY, VERSION_ADDED, VERSION_REMOVED)\
        {\
            if(VERSION_REMOVED == 0 || (currentVersion >= VERSION_ADDED && currentVersion < VERSION_REMOVED))\
            {\
                using PropertyType = typename std::decay<decltype(PPCAT(object., PROPERTY))>::type;\
                using ObjectType = typename std::decay<decltype(object)>::type;\
                ::Insight::Serialisation::ComplexSerialiser<TYPE_SERIALISER, PropertyType, ObjectType> complexSerialiser;\
                complexSerialiser(serialiser, PPCAT(object., PROPERTY), &object);\
            }\
        }

// Serialise a single property with a ProertySerialiser.
#define SERIALISE_PROPERTY(TYPE_SERIALISER, PROPERTY, VERSION_ADDED, VERSION_REMOVED)           SERIALISE_NAMED_PROPERTY(TYPE_SERIALISER, PROPERTY, PROPERTY, VERSION_ADDED, VERSION_REMOVED)
// Serialise a single property with a SerialiserObject.
#define SERIALISE_OBJECT(TYPE_SERIALISER, PROPERTY, VERSION_ADDED, VERSION_REMOVED)             SERIALISE_NAMED_OBJECT(TYPE_SERIALISER, PROPERTY, PROPERTY, VERSION_ADDED, VERSION_REMOVED)
// Cast the serialiser object to 'BASE_TYPE_SERIALISER' and use a SerialiserObject.
#define SERIALISE_BASE(BASE_TYPE, VERSION_ADDED, VERSION_REMOVED)                               SERIALISE_NAMED_BASE(BASE_TYPE, VERSION_ADDED, VERSION_REMOVED)

// Serialise a vector property with a ProertySerialiser.
#define SERIALISE_VECTOR_PROPERTY(TYPE_SERIALISER, PROPERTY, VERSION_ADDED, VERSION_REMOVED)    SERIALISE_VECTOR_NAMED_PROPERTY(TYPE_SERIALISER, PROPERTY, PROPERTY, VERSION_ADDED, VERSION_REMOVED)
// Serialise a vector property with a SerialiserObject.
#define SERIALISE_VECTOR_OBJECT(TYPE_SERIALISER, PROPERTY, VERSION_ADDED, VERSION_REMOVED)      SERIALISE_VECTOR_NAMED_OBJECT(TYPE_SERIALISER, PROPERTY, PROPERTY, VERSION_ADDED, VERSION_REMOVED)

// Serialise an array property with a ProertySerialiser.
#define SERIALISE_ARRAY_PROPERTY(TYPE_SERIALISER, PROPERTY, VERSION_ADDED, VERSION_REMOVED)    SERIALISE_ARRAY_NAMED_PROPERTY(TYPE_SERIALISER, PROPERTY, PROPERTY, VERSION_ADDED, VERSION_REMOVED)
// Serialise an array property with a SerialiserObject.
#define SERIALISE_ARRAY_OBJECT(TYPE_SERIALISER, PROPERTY, VERSION_ADDED, VERSION_REMOVED)      SERIALISE_ARRAY_NAMED_OBJECT(TYPE_SERIALISER, PROPERTY, PROPERTY, VERSION_ADDED, VERSION_REMOVED)


//
#define SERIALISE_MAP_OBJECT(KEY_SERIALISER, VALUE_SERIALISER, PROPERTY, VERSION_ADDED, VERSION_REMOVED) SERIALISE_MAP_NAMED_OBJECT(KEY_SERIALISER, VALUE_SERIALISER, PROPERTY, PROPERTY, VERSION_ADDED, VERSION_REMOVED)

// Serialise anything. This should be used when there is a certain requirement needed. 
// An example could be loading entities.
#define SERIALISE_COMPLEX(TYPE_SERIALISER, PROPERTY, VERSION_ADDED, VERSION_REMOVED)            SERIALISE_NAMED_COMPLEX(TYPE_SERIALISER, PROPERTY, PROPERTY, VERSION_ADDED, VERSION_REMOVED)

#define SERIALISE_FUNC(OBJECT_TYPE, CURRENT_VERSION, ...)\
    public:\
        void Serialise(::Insight::Serialisation::ISerialiser* serialiser, OBJECT_TYPE& object)\
        {\
            const u32 currentVersion = CURRENT_VERSION;\
            serialiser->StartObject(#OBJECT_TYPE);\
            u32 currentChildSerialiser = 0;\
            if (!serialiser->IsReadMode())\
            {\
                serialiser->SetName(#OBJECT_TYPE); \
                serialiser->Write("SERIALISED_VERSION", currentVersion); \
            }\
            __VA_ARGS__\
            serialiser->StopObject();\
        }\

#define DESERIALISE_FUNC(OBJECT_TYPE, CURRENT_VERSION, ...)\
        void Deserialise(::Insight::Serialisation::ISerialiser* serialiser, OBJECT_TYPE& object)\
        {\
            u32 currentVersion = CURRENT_VERSION;\
            serialiser->StartObject(#OBJECT_TYPE);\
            u32 serialisedVersion = 0;\
            u32 currentChildSerialiser = 0;\
            if (serialiser->IsReadMode())\
            {\
                serialiser->Read("SERIALISED_VERSION", serialisedVersion); \
            }\
            __VA_ARGS__\
            serialiser->StopObject();\
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