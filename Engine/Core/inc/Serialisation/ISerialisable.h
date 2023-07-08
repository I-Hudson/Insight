#pragma once

#include "Core/Defines.h"
#include "Serialisation/Serialiser.h"

namespace Insight
{
    namespace Serialisation
    {
        template<typename>
        struct SerialiserObject;
        template<typename, typename, typename>
        struct ComplexSerialiser;
        template<typename>
        struct PropertySerialiser;

        class IS_CORE ISerialisable
        {
        public:
            virtual void BeforeSerialise(ISerialiser* serialiser) { }
            virtual void Serialise(ISerialiser* serialiser) = 0;
            virtual void AfterSerialise(ISerialiser* serialiser) { }

            virtual void BeforeDeserialise(ISerialiser* serialiser) { }
            virtual void Deserialise(ISerialiser* serialiser) = 0;
            virtual void AfterDeserialise(ISerialiser* serialiser) { }
        };
    }
}

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
        virtual void Serialise(::Insight::Serialisation::ISerialiser* serialiser);\
        virtual void Deserialise(::Insight::Serialisation::ISerialiser* serialiser);

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