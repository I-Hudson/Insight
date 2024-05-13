#pragma once

#include "Serialisation/SerialiserBase.h"
#include "Serialisation/DeserialiserBasicTypes.h"
#include "Serialisation/PropertySerialiserTypes.h"

#include "Core/StringUtils.h"

#include <string>
#include <optional>

constexpr bool VersionCheck(const u32 serialisedVersion, const u32 versionAdded, const u32 versionRemoved)
{
    return (serialisedVersion == -1)
        || (versionRemoved == 0 && serialisedVersion >= versionAdded) 
        || (serialisedVersion >= versionAdded && serialisedVersion < versionRemoved);
}

constexpr bool ObjectSerialiserCheck(std::string_view currentObjectSerialiser, std::string_view serialisedObjectSerialiser)
{
    return true;//currentObjectSerialiser == serialisedObjectSerialiser;
}

namespace Insight::Serialisation::Internal
{
    enum class SerialiseAction
    {
        Serialise,
        Deserialise,
        GetData
    };

    struct Options
    {
        SerialiseAction Action;
        const char* GetDataName;
    };



    template<typename TypeSerialiser, typename TData>
    TData SerialiseNamedProperty(ISerialiser* serialiser, u32 version, u32 versionAdded, u32 versionRemoved, const char* propertyName, TData& data, Options options)
    {
        if (options.Action == SerialiseAction::Serialise)
        {
            if (VersionCheck(version, versionAdded, versionRemoved))
            {
                ::Insight::Serialisation::SerialiseProperty<TypeSerialiser>(serialiser, propertyName, data);
            }
        }
        else if (options.Action == SerialiseAction::Deserialise)
        {
            if (VersionCheck(version, versionAdded, versionRemoved))
            {
                using PropertyType = typename std::decay<decltype(data)>::type;
                data = ::Insight::Serialisation::DeserialiseProperty<TypeSerialiser, PropertyType>(serialiser, propertyName);
            }
        }
        else if (options.Action == SerialiseAction::GetData)
        {
            if (VersionCheck(version, versionAdded, versionRemoved) && 
                options.GetDataName == propertyName)
            {
                using PropertyType = typename std::decay<decltype(data)>::type;
                return ::Insight::Serialisation::DeserialiseProperty<TypeSerialiser, PropertyType>(serialiser, propertyName);
            }
        }
        return TData();
    }
    
    template<typename TypeSerialiser>
    void SerialiseNamedPropertyRemoved(ISerialiser* serialiser, u32 version, u32 versionAdded, u32 versionRemoved, const char* propertyName, Options options)
    {
        if (options.Action == SerialiseAction::Deserialise)
        {
            if (VersionCheck(version, versionAdded, versionRemoved))
            {
                ::Insight::Serialisation::PropertyDeserialiser<TypeSerialiser> propertyDeserialiser;
                typename ::Insight::Serialisation::PropertyDeserialiser<TypeSerialiser>::InType blankData;
                using PropertyType = typename std::decay<decltype(propertyDeserialiser(blankData))>::type;
                ::Insight::Serialisation::DeserialiseProperty<TypeSerialiser, PropertyType>(serialiser, propertyName);
            }
        }
    }

    template<typename TypeSerialiser, typename TData>
    std::optional<TData> SerialiseNamedObject(ISerialiser* serialiser, u32 version, u32 versionAdded, u32 versionRemoved, const char* propertyName, TData& data, Options options)
    {
        if (options.Action == SerialiseAction::Serialise)
        {
            if (VersionCheck(version, versionAdded, versionRemoved))
            {
                ::Insight::Serialisation::SerialiseObject<TypeSerialiser>(serialiser, data);
            }
        }
        else if (options.Action == SerialiseAction::Deserialise)
        {
            if (VersionCheck(version, versionAdded, versionRemoved))
            {
                ::Insight::Serialisation::DeserialiseObject<TypeSerialiser>(serialiser, data);
            }
        }
        else if (options.Action == SerialiseAction::GetData)
        {
            if constexpr (std::is_copy_constructible_v<TData>)
            {
                if (VersionCheck(version, versionAdded, versionRemoved))
                {
                    using ObjectType = typename std::decay<decltype(data)>::type;
                    ObjectType objectData;
                    ::Insight::Serialisation::DeserialiseObject<TypeSerialiser>(serialiser, objectData);
                    return objectData;
                }
            }
        }
        return std::optional<TData>();
    }

    template<typename TypeSerialiser, typename TData>
    void SerialiseNamedObjectRemoved(ISerialiser* serialiser, u32 version, u32 versionAdded, u32 versionRemoved, const char* propertyName, Options options)
    {
        if (options.Action == SerialiseAction::Deserialise)
        {
            if (VersionCheck(version, versionAdded, versionRemoved))
            {
                using ObjectType = typename std::decay<decltype(data)>::type;
                ObjectType objectData;
                ::Insight::Serialisation::DeserialiseObject<TypeSerialiser>(serialiser, objectData);
            }
        }
    }

    template<typename TypeSerialiser, typename TData, typename TObject>
    void SerialiseNamedComplex(ISerialiser* serialiser, u32 version, u32 versionAdded, u32 versionRemoved, TData& data, TObject& object)
    {
        if (VersionCheck(version, versionAdded, versionRemoved))
        {
            ::Insight::Serialisation::ComplexSerialiser<TypeSerialiser, TData, TObject> complexSerialiser;
            complexSerialiser(serialiser, data, &object);
        }
    }

    template<typename TypeSerialiser, typename TData, typename TObject>
    void SerialiseNamedComplexRemoved(ISerialiser* serialiser, u32 version, u32 versionAdded, u32 versionRemoved, TObject& object)
    {
        if (VersionCheck(version, versionAdded, versionRemoved))
        {
            TData blankData;
            ::Insight::Serialisation::ComplexSerialiser<TypeSerialiser, TData, TObject> complexSerialiser;
            complexSerialiser(serialiser, blankData, &object);
        }
    }   
}

// Serialise a single property. This would be things which only contain data for them self. 
#define SERIALISE_NAMED_PROPERTY(TYPE_SERIALISER, PROPERTY_NAME, PROPERTY, VERSION_ADDED, VERSION_REMOVED)\
::Insight::Serialisation::Internal::SerialiseNamedProperty<TYPE_SERIALISER>(serialiser, version, VERSION_ADDED, VERSION_REMOVED, #PROPERTY_NAME, PPCAT(object., PROPERTY), m_options);


// Replace SERIALISE_NAMED_PROPERTY with SERIALISE_NAMED_PROPERTY_REMOVED when a property has been removed.
#define SERIALISE_NAMED_PROPERTY_REMOVED(TYPE_SERIALISER, PROPERTY_NAME, VERSION_ADDED, VERSION_REMOVED)\
::Insight::Serialisation::Internal::SerialiseNamedPropertyRemoved<TYPE_SERIALISER>(serialiser, version, VERSION_ADDED, VERSION_REMOVED, #PROPERTY_NAME, m_options);

#define SERIALISE_NAMED_OBJECT(TYPE_SERIALISER, PROPERTY_NAME, PROPERTY, VERSION_ADDED, VERSION_REMOVED)\
::Insight::Serialisation::Internal::SerialiseNamedObject<TYPE_SERIALISER>(serialiser, version, VERSION_ADDED, VERSION_REMOVED, #PROPERTY_NAME, PPCAT(object., PROPERTY), m_options);

// Replace SERIALISE_NAMED_OBJECT with SERIALISE_NAMED_OBJECT_REMOVED when an object is removed.
#define SERIALISE_NAMED_OBJECT_REMOVED(TYPE_SERIALISER, PROPERTY_NAME, VERSION_ADDED, VERSION_REMOVED)\
::Insight::Serialisation::Internal::SerialiseNamedObjectRemoved<TYPE_SERIALISER>(serialiser, version, VERSION_ADDED, VERSION_REMOVED, #PROPERTY_NAME, m_options);

#define SERIALISE_NAMED_BASE(BASE_TYPE, VERSION_ADDED, VERSION_REMOVED)\
        if (!isReadMode)\
        {\
            if(VersionCheck(version, VERSION_ADDED, VERSION_REMOVED))\
            {\
                BASE_TYPE* baseType = static_cast<BASE_TYPE*>(&object);\
                ::Insight::Serialisation::SerialiseBase<BASE_TYPE>(serialiser, *baseType);\
            }\
        }\
        else\
        {\
            if(VersionCheck(version, VERSION_ADDED, VERSION_REMOVED))\
            {\
                ::Insight::Serialisation::SerialiserObject<BASE_TYPE> objectSerialiser; \
                BASE_TYPE* baseTypePtr = static_cast<BASE_TYPE*>(&object);\
                BASE_TYPE& baseTypeRef = *baseTypePtr;\
                objectSerialiser.Deserialise(serialiser, baseTypeRef, serialisedObjectSerilaiser);\
            }\
        }
// Replace SERIALISE_NAMED_BASE with SERIALISE_NAMED_OBJECT_REMOVED when an object is removed.
#define SERIALISE_NAMED_BASE_REMOVED(BASE_TYPE, VERSION_ADDED, VERSION_REMOVED)\
        if (isReadMode)\
        {\
            if(VersionCheck(version, VERSION_ADDED, VERSION_REMOVED))\
            {\
                ::Insight::Serialisation::SerialiserObject<BASE_TYPE> objectSerialiser; \
                BASE_TYPE* baseTypePtr = static_cast<BASE_TYPE*>(&object);\
                BASE_TYPE& baseTypeRef = *baseTypePtr;\
                objectSerialiser.Deserialise(serialiser, baseTypeRef, serialisedObjectSerilaiser);\
            }\
        }


// Serialise a single property. This would be thiings which only contain data for them self. 
#define SERIALISE_VECTOR_NAMED_PROPERTY(TYPE_SERIALISER, PROPERTY_NAME, PROPERTY, VERSION_ADDED, VERSION_REMOVED)\
        if (!isReadMode)\
        {\
            if(VersionCheck(version, VERSION_ADDED, VERSION_REMOVED))\
            {\
                using TVectorType = typename std::decay<decltype(*PPCAT(object., PROPERTY).begin())>::type;\
                ::Insight::Serialisation::VectorSerialiser<TVectorType, TYPE_SERIALISER, ::Insight::Serialisation::SerialiserType::Property> vectorSerialiser;\
                vectorSerialiser(serialiser, #PROPERTY_NAME, PPCAT(object., PROPERTY));\
            }\
        }\
        else\
        {\
            if(VersionCheck(version, VERSION_ADDED, VERSION_REMOVED) && ObjectSerialiserCheck(objectSerialiserType, serialisedObjectSerilaiser))\
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
        if (!isReadMode)\
        {\
            if(VersionCheck(version, VERSION_ADDED, VERSION_REMOVED))\
            {\
                using TVectorType = typename std::decay<decltype(*PPCAT(object., PROPERTY).begin())>::type;\
                ::Insight::Serialisation::VectorSerialiser<TVectorType, TYPE_SERIALISER, ::Insight::Serialisation::SerialiserType::Object> vectorSerialiser;\
                vectorSerialiser(serialiser, #PROPERTY_NAME, PPCAT(object., PROPERTY));\
            }\
        }\
        else\
        {\
            if(VersionCheck(version, VERSION_ADDED, VERSION_REMOVED) && ObjectSerialiserCheck(objectSerialiserType, serialisedObjectSerilaiser))\
            {\
                using TVectorType = typename std::decay<decltype(*PPCAT(object., PROPERTY).begin())>::type;\
                ::Insight::Serialisation::VectorDeserialiser<TVectorType, TYPE_SERIALISER, ::Insight::Serialisation::SerialiserType::Object> vectorDeserialiser;\
                vectorDeserialiser(serialiser, #PROPERTY_NAME, PPCAT(object., PROPERTY));\
            }\
        }

// Serialise a single property. This would be thiings which only contain data for them self. 
#define SERIALISE_ARRAY_NAMED_PROPERTY(TYPE_SERIALISER, PROPERTY_NAME, PROPERTY, VERSION_ADDED, VERSION_REMOVED)\
        if (!isReadMode)\
        {\
            if(VersionCheck(version, VERSION_ADDED, VERSION_REMOVED))\
            {\
                using TArrayType = typename std::decay<decltype(*PPCAT(object., PROPERTY).begin())>::type;\
                ::Insight::Serialisation::ArraySerialiser<TArrayType, TYPE_SERIALISER, ::Insight::Serialisation::SerialiserType::Property> arraySerialiser;\
                arraySerialiser(serialiser, #PROPERTY_NAME, const_cast<TArrayType*>(PPCAT(object., PROPERTY).data()), PPCAT(object., PROPERTY).size());\
            }\
        }\
        else\
        {\
            if(VersionCheck(version, VERSION_ADDED, VERSION_REMOVED) && ObjectSerialiserCheck(objectSerialiserType, serialisedObjectSerilaiser))\
            {\
                using TArrayType = typename std::decay<decltype(*PPCAT(object., PROPERTY).begin())>::type;\
                ::Insight::Serialisation::ArrayDeserialiser<TArrayType, TYPE_SERIALISER, ::Insight::Serialisation::SerialiserType::Property> arrayDeserialiser;\
                arrayDeserialiser(serialiser, #PROPERTY_NAME, const_cast<TArrayType*>(PPCAT(object., PROPERTY).data()), PPCAT(object., PROPERTY).size());\
            }\
        }
#define SERIALISE_ARRAY_NAMED_OBJECT(TYPE_SERIALISER, PROPERTY_NAME, PROPERTY, VERSION_ADDED, VERSION_REMOVED)\
        if (!isReadMode)\
        {\
            if(VersionCheck(version, VERSION_ADDED, VERSION_REMOVED))\
            {\
                using TArrayType = typename std::decay<decltype(*PPCAT(object., PROPERTY).begin())>::type;\
                ::Insight::Serialisation::ArraySerialiser<TArrayType,TYPE_SERIALISER, ::Insight::Serialisation::SerialiserType::Object> arraySerialiser;\
                arraySerialiser(serialiser, #PROPERTY_NAME, PPCAT(object., PROPERTY).data(), PPCAT(object., PROPERTY).size());\
            }\
        }\
        else\
        {\
            if(VersionCheck(version, VERSION_ADDED, VERSION_REMOVED) && ObjectSerialiserCheck(objectSerialiserType, serialisedObjectSerilaiser))\
            {\
            }\
        }

// Serialise a single property. This would be thiings which only contain data for them self. 
#define SERIALISE_SET_NAMED(TYPE_SERIALISER, PROPERTY_NAME, PROPERTY, VERSION_ADDED, VERSION_REMOVED, SERIALISER_TYPE)\
        if (!isReadMode)\
        {\
            if(VersionCheck(version, VERSION_ADDED, VERSION_REMOVED))\
            {\
                using TVectorType = typename std::decay<decltype(*PPCAT(object., PROPERTY).begin())>::type;\
                ::Insight::Serialisation::SetSerialiser<TVectorType, TYPE_SERIALISER, SERIALISER_TYPE> setSerialiser;\
                setSerialiser(serialiser, #PROPERTY_NAME, PPCAT(object., PROPERTY));\
            }\
        }\
        else\
        {\
            if(VersionCheck(version, VERSION_ADDED, VERSION_REMOVED) && ObjectSerialiserCheck(objectSerialiserType, serialisedObjectSerilaiser))\
            {\
                using TVectorType = typename std::decay<decltype(*PPCAT(object., PROPERTY).begin())>::type;\
                ::Insight::Serialisation::SetSerialiser<TVectorType, TYPE_SERIALISER, SERIALISER_TYPE> setDeserialiser;\
                setDeserialiser(serialiser, #PROPERTY_NAME, PPCAT(object., PROPERTY));\
            }\
        }

#define SERIALISE_MAP_NAMED_OBJECT(KEY_SERIALISER, VALUE_SERIALISER, PROPERTY_NAME, PROPERTY, VERSION_ADDED, VERSION_REMOVED)\
        if (!isReadMode)\
        {\
            if(VersionCheck(version, VERSION_ADDED, VERSION_REMOVED))\
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
::Insight::Serialisation::Internal::SerialiseNamedComplex<TYPE_SERIALISER>(serialiser, version, VERSION_ADDED, VERSION_REMOVED, PPCAT(object., PROPERTY), object);

#define SERIALISE_NAMED_COMPLEX_REMOVED(TYPE_SERIALISER, PROPERTY_TYPE, PROPERTY, VERSION_ADDED, VERSION_REMOVED)\
::Insight::Serialisation::Internal::SerialiseNamedComplexRemoved<TYPE_SERIALISER, PROPERTY_TYPE>(serialiser, version, VERSION_ADDED, VERSION_REMOVED, object);

#define SERIALISE_NAMED_COMPLEX_THIS(TYPE_SERIALISER, VERSION_ADDED, VERSION_REMOVED)\
        {\
            if(VersionCheck(version, VERSION_ADDED, VERSION_REMOVED))\
            {\
                using ObjectType = typename std::decay<decltype(object)>::type;\
                ::Insight::Serialisation::ComplexSerialiser<TYPE_SERIALISER, void, ObjectType> complexSerialiser;\
                complexSerialiser(serialiser, &object);\
            }\
        }   

// Serialise a single property with a ProertySerialiser.
#define SERIALISE_PROPERTY(TYPE_SERIALISER, PROPERTY, VERSION_ADDED, VERSION_REMOVED)                   SERIALISE_NAMED_PROPERTY(TYPE_SERIALISER, PROPERTY, PROPERTY, VERSION_ADDED, VERSION_REMOVED)
// Serialise a single property with a SerialiserObject.
#define SERIALISE_OBJECT(TYPE_SERIALISER, PROPERTY, VERSION_ADDED, VERSION_REMOVED)                     SERIALISE_NAMED_OBJECT(TYPE_SERIALISER, PROPERTY, PROPERTY, VERSION_ADDED, VERSION_REMOVED)
// Cast the serialiser object to 'BASE_TYPE_SERIALISER' and use a SerialiserObject.
#define SERIALISE_BASE(BASE_TYPE, VERSION_ADDED, VERSION_REMOVED)                                       SERIALISE_NAMED_BASE(BASE_TYPE, VERSION_ADDED, VERSION_REMOVED)

// Replace SERIALISE_PROPERTY with this when a property is removed.
#define SERIALISE_PROPERTY_REMOVED(TYPE_SERIALISER, PROPERTY_NAME, VERSION_ADDED, VERSION_REMOVED)      SERIALISE_NAMED_PROPERTY_REMOVED(TYPE_SERIALISER, PROPERTY_NAME, VERSION_ADDED, VERSION_REMOVED)
// Replace SERIALISE_OBJECT with this when an object is removed.
#define SERIALISE_OBJECT_REMOVED(TYPE_SERIALISER, PROPERTY_NAME, VERSION_ADDED, VERSION_REMOVED)        SERIALISE_NAMED_OBJECT_REMOVED(TYPE_SERIALISER, PROPERTY_NAME, VERSION_ADDED, VERSION_REMOVED)
// Replace SERIALISE_BASE with this when an base object is removed.
#define SERIALISE_BASE_REMOVED(BASE_TYPE, VERSION_ADDED, VERSION_REMOVED)                               SERIALISE_NAMED_BASE_REMOVED(BASE_TYPE, VERSION_ADDED, VERSION_REMOVED)

// Serialise a vector property with a ProertySerialiser.
#define SERIALISE_VECTOR_PROPERTY(TYPE_SERIALISER, PROPERTY, VERSION_ADDED, VERSION_REMOVED)            SERIALISE_VECTOR_NAMED_PROPERTY(TYPE_SERIALISER, PROPERTY, PROPERTY, VERSION_ADDED, VERSION_REMOVED)
// Serialise a vector property with a SerialiserObject.
#define SERIALISE_VECTOR_OBJECT(TYPE_SERIALISER, PROPERTY, VERSION_ADDED, VERSION_REMOVED)              SERIALISE_VECTOR_NAMED_OBJECT(TYPE_SERIALISER, PROPERTY, PROPERTY, VERSION_ADDED, VERSION_REMOVED)

// Serialise an array property with a ProertySerialiser.
#define SERIALISE_ARRAY_PROPERTY(TYPE_SERIALISER, PROPERTY, VERSION_ADDED, VERSION_REMOVED)             SERIALISE_ARRAY_NAMED_PROPERTY(TYPE_SERIALISER, PROPERTY, PROPERTY, VERSION_ADDED, VERSION_REMOVED)
// Serialise an array property with a SerialiserObject.
#define SERIALISE_ARRAY_OBJECT(TYPE_SERIALISER, PROPERTY, VERSION_ADDED, VERSION_REMOVED)               SERIALISE_ARRAY_NAMED_OBJECT(TYPE_SERIALISER, PROPERTY, PROPERTY, VERSION_ADDED, VERSION_REMOVED)

// Serialise a vector property with a ProertySerialiser.
#define SERIALISE_SET_PROPERTY(TYPE_SERIALISER, PROPERTY, VERSION_ADDED, VERSION_REMOVED)               SERIALISE_SET_NAMED(TYPE_SERIALISER, PROPERTY, PROPERTY, VERSION_ADDED, VERSION_REMOVED, ::Insight::Serialisation::SerialiserType::Property)
// Serialise a vector property with a SerialiserObject.
#define SERIALISE_SET_OBJECT(TYPE_SERIALISER, PROPERTY, VERSION_ADDED, VERSION_REMOVED)                 SERIALISE_SET_NAMED(TYPE_SERIALISER, PROPERTY, PROPERTY, VERSION_ADDED, VERSION_REMOVED, ::Insight::Serialisation::SerialiserType::Object)


//
#define SERIALISE_MAP_OBJECT(KEY_SERIALISER, VALUE_SERIALISER, PROPERTY, VERSION_ADDED, VERSION_REMOVED) SERIALISE_MAP_NAMED_OBJECT(KEY_SERIALISER, VALUE_SERIALISER, PROPERTY, PROPERTY, VERSION_ADDED, VERSION_REMOVED)

// Serialise anything. This should be used when there is a certain requirement needed. 
// An example could be loading entities.
#define SERIALISE_COMPLEX(TYPE_SERIALISER, PROPERTY, VERSION_ADDED, VERSION_REMOVED)                     SERIALISE_NAMED_COMPLEX(TYPE_SERIALISER, PROPERTY, PROPERTY, VERSION_ADDED, VERSION_REMOVED)
#define SERIALISE_COMPLEX_REMOVED(TYPE_SERIALISER, PROPERTY, VERSION_ADDED, VERSION_REMOVED)             SERIALISE_NAMED_COMPLEX_REMOVED(TYPE_SERIALISER, PROPERTY, PROPERTY, VERSION_ADDED, VERSION_REMOVED)

#define SERIALISE_COMPLEX_THIS(TYPE_SERIALISER, VERSION_ADDED, VERSION_REMOVED)                          SERIALISE_NAMED_COMPLEX_THIS(TYPE_SERIALISER, VERSION_ADDED, VERSION_REMOVED)
#define SERIALISE_COMPLEX_THIS_REMOVED(TYPE_SERIALISER, VERSION_ADDED, VERSION_REMOVED)                  SERIALISE_NAMED_COMPLEX_THIS_REMOVED(TYPE_SERIALISER, VERSION_ADDED, VERSION_REMOVED)

namespace Insight::Serialisation::Keys
{
    constexpr const char* c_ObjectSerialiser = "OBJECT_SERIALISER";
    constexpr const char* c_SerialisedVersion = "SERIALISED_VERSION";
}

#define SERIALISE_FUNC(OBJECT_TYPE, CURRENT_VERSION, ...)\
    public:\
        void Serialise(::Insight::Serialisation::ISerialiser* serialiser, OBJECT_TYPE& object, std::string serialisedObjectSerilaiser = "")\
        {\
            const u32 version = CURRENT_VERSION;\
            std::string objectSerialiserType = typeid(OBJECT_TYPE).name();\
            objectSerialiserType = RemoveString(objectSerialiserType, "class");\
            objectSerialiserType = RemoveString(objectSerialiserType, "struct");\
\
            m_options = {};\
            m_options.Action = ::Insight::Serialisation::Internal::SerialiseAction::Serialise;\
\
            bool isReadMode = false;\
\
            serialisedObjectSerilaiser = objectSerialiserType;\
\
/*           serialiser->StartObject(Insight::Serialisation::Keys::c_ObjectSerialiser);\
            serialiser->Write(Insight::Serialisation::Keys::c_ObjectSerialiser, objectSerialiserType); \
            serialiser->StopObject();\
*/\
\
            if (serialiser) { serialiser->StartObject(#OBJECT_TYPE); }\
            if (serialiser&& !serialiser->IsReadMode())\
            {\
                serialiser->SetName(#OBJECT_TYPE); \
                if (MetaDataEnabled)\
                {\
                    serialiser->Write(Insight::Serialisation::Keys::c_SerialisedVersion, version); \
                }\
            }\
            __VA_ARGS__\
            if (serialiser) { serialiser->StopObject();} \
        }\

static bool DeserialiseCheckForObjectSerialiser(::Insight::Serialisation::ISerialiser* serialiser, std::string& serialisedObject, const std::string& currentObjectSerialiser)
{
    if (serialisedObject == currentObjectSerialiser)
    {
        return true;
    }

    std::string diskObjectSerialiser;
    serialiser->StartObject(Insight::Serialisation::Keys::c_ObjectSerialiser);
    serialiser->Read(Insight::Serialisation::Keys::c_ObjectSerialiser, diskObjectSerialiser);
    serialiser->StopObject();
    
    if (!diskObjectSerialiser.empty())
    {
        serialisedObject = diskObjectSerialiser;
    }
    bool correctObject = serialisedObject == currentObjectSerialiser;
    assert(correctObject);
    return correctObject;
}

#define DESERIALISE_FUNC(OBJECT_TYPE, CURRENT_VERSION, ...)\
        void Deserialise(::Insight::Serialisation::ISerialiser* serialiser, OBJECT_TYPE& object, std::string serialisedObjectSerilaiser = "")\
        {\
            u32 version = -1;\
            std::string objectSerialiserType = typeid(OBJECT_TYPE).name();\
            objectSerialiserType = RemoveString(objectSerialiserType, "class");\
            objectSerialiserType = RemoveString(objectSerialiserType, "struct");\
\
            m_options = {};\
            m_options.Action = ::Insight::Serialisation::Internal::SerialiseAction::Deserialise;\
\
            bool isReadMode = true;\
            bool correctObject = true;\
\
            if (correctObject)\
            {\
                if (serialiser) { serialiser->StartObject(#OBJECT_TYPE); }\
                    if (isReadMode)\
                    {\
                        if (MetaDataEnabled)\
                        {\
                            serialiser->Read(Insight::Serialisation::Keys::c_SerialisedVersion, version); \
                        }\
                    }\
            }\
            __VA_ARGS__\
            if (correctObject)\
            {\
                if (serialiser) { serialiser->StopObject(); }\
            }\
        }

#define GET_DATA_FUNC(OBJECT_TYPE, CURRENT_VERSION, ...)\
    public:\
        template<typename T>\
        T GetData(::Insight::Serialisation::ISerialiser* serialiser, OBJECT_TYPE& object, const char* name)\
        {\
            const u32 version = CURRENT_VERSION;\
            std::string objectSerialiserType = typeid(OBJECT_TYPE).name();\
            objectSerialiserType = RemoveString(objectSerialiserType, "class");\
            objectSerialiserType = RemoveString(objectSerialiserType, "struct");\
\
            m_options = {};\
            m_options.Action = ::Insight::Serialisation::Internal::SerialiseAction::GetData;\
            m_options.GetDataName = name;\
\
            bool isReadMode = true;\
\
            std::string serialisedObjectSerilaiser = objectSerialiserType;\
\
            if (serialiser) { serialiser->StartObject(#OBJECT_TYPE); }\
            if (serialiser&& !serialiser->IsReadMode())\
            {\
                serialiser->SetName(#OBJECT_TYPE); \
                if (MetaDataEnabled)\
                {\
                    serialiser->Write(Insight::Serialisation::Keys::c_SerialisedVersion, version); \
                }\
            }\
            __VA_ARGS__\
            if (serialiser) { serialiser->StopObject();} \
            return T();\
        }\

#define OBJECT_SERIALISER(OBJECT_TYPE, CURRENT_VERSION, ...)\
        static_assert(CURRENT_VERSION >= 1);\
        template<>\
        struct ::Insight::Serialisation::SerialiserObject<OBJECT_TYPE> : public SerialiserObjectBase\
        {\
            using ObjectType = OBJECT_TYPE;\
            SERIALISE_FUNC(OBJECT_TYPE, CURRENT_VERSION, __VA_ARGS__);\
            DESERIALISE_FUNC(OBJECT_TYPE, CURRENT_VERSION, __VA_ARGS__);\
            GET_DATA_FUNC(OBJECT_TYPE, CURRENT_VERSION, __VA_ARGS__);\
        \
        private:\
            std::string objectSerialiserType;\
            ::Insight::Serialisation::Internal::Options m_options;\
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