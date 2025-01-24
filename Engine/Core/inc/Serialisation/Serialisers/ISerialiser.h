#pragma once

#include "Core/Defines.h"
#include "Core/TypeAlias.h"

#include <vector>
#include <string_view>
#include <string>

namespace Insight
{
    namespace Serialisation
    {
        constexpr const char* c_ObjectName = "ObjectName";
        constexpr const char* c_ArrayName = "ArrayName";
        constexpr const char* c_SerialiserName = "SerialiserName";
        constexpr const char* c_ChildSerialiser = "ChildSerialiser";
        constexpr const char* c_ArraySize = "ArraySize";

        enum class SerialisationTypes
        {
            Json,
            Binary,

            Size
        };
        constexpr const char* SerialisationTypeToString[]
        {
            "Json",
            "Binary",
        };
        static_assert(ARRAY_COUNT(SerialisationTypeToString) == static_cast<u64>(SerialisationTypes::Size));

        enum class SerialiserWriteReadTypes
        {
            Property,
            Object
        };

        template<typename T>
        constexpr bool IsSerialiserAllowedType = 
               std::is_same_v<bool, T> 
            || std::is_same_v<char, T>
            || std::is_same_v<float, T>
            || std::is_same_v<u8, T>
            || std::is_same_v<u16, T>
            || std::is_same_v<u32, T>
            || std::is_same_v<u64, T>
            || std::is_same_v<i8, T>
            || std::is_same_v<i16, T>
            || std::is_same_v<i32, T>
            || std::is_same_v<i64, T>
            || std::is_same_v<std::string, T>
            || std::is_same_v<std::vector<u8>, T>;

        enum class SerialiserNodeStates 
        { 
            None, 
            Object, 
            Array 
        };

        constexpr const char* c_SerialiserType = "SerialiserType";

        /// @brief Base class for all serialises used. This defines what data can be saved.
        class IS_CORE ISerialiser
        {
        public:
            ISerialiser(SerialisationTypes type, const u32 version, bool isReadMode);
            virtual ~ISerialiser();

            static ISerialiser* Create(SerialisationTypes type, bool isReadMode);

            void SetName(std::string_view tag);
            std::string_view GetName() const;

            u32 GetVersion() const;
            SerialisationTypes GetType() const;
            
            bool IsReadMode() const;
            virtual bool IsEmpty() const = 0;

            void Write(std::string_view tag, const char* string);

            virtual bool Deserialise(std::vector<u8> data) = 0;
            virtual std::vector<Byte> GetSerialisedData() = 0;

            /// @brief Clear any data stored within the serialsier.
            virtual void Clear() = 0;

            virtual void StartObject(std::string_view name) = 0;
            virtual void StopObject() = 0;
            /// @brief Skip the next object in the stream.
            virtual void SkipObject() { }

            virtual void StartArray(std::string_view name, u64& size, bool encodeSize = true) = 0;
            virtual void StopArray() = 0;

            virtual void Write(std::string_view tag, bool data) = 0;
            virtual void Write(std::string_view tag, char data) = 0;
            virtual void Write(std::string_view tag, float data) = 0;

            virtual void Write(std::string_view tag, u8 data) = 0;
            virtual void Write(std::string_view tag, u16 data) = 0;
            virtual void Write(std::string_view tag, u32 data) = 0;
            virtual void Write(std::string_view tag, u64 data) = 0;

            virtual void Write(std::string_view tag, i8 data) = 0;
            virtual void Write(std::string_view tag, i16 data) = 0;
            virtual void Write(std::string_view tag, i32 data) = 0;
            virtual void Write(std::string_view tag, i64 data) = 0;

            virtual void Write(std::string_view tag, std::string const& string) = 0;

            virtual void Write(std::string_view tag, const std::vector<Byte>& vector, bool encodeSize = true) = 0;

            //--

            virtual void Read(std::string_view tag, bool& data) = 0;
            virtual void Read(std::string_view tag, char& data) = 0;
            virtual void Read(std::string_view tag, float& data) = 0;

            virtual void Read(std::string_view tag, u8& data) = 0;
            virtual void Read(std::string_view tag, u16& data) = 0;
            virtual void Read(std::string_view tag, u32& data) = 0;
            virtual void Read(std::string_view tag, u64& data) = 0;

            virtual void Read(std::string_view tag, i8& data) = 0;
            virtual void Read(std::string_view tag, i16& datan) = 0;
            virtual void Read(std::string_view tag, i32& data) = 0;
            virtual void Read(std::string_view tag, i64& data) = 0;

            virtual void Read(std::string_view tag, std::string& string) = 0;

            virtual void Read(std::string_view tag, std::vector<Byte>& vector, bool dcodeSize = true) = 0;

        protected:
            virtual void WriteHeader(std::vector<Byte>& data) const;

            /// @brief Validate that the serialised data deserialised is valid
            /// for the serialiser being used.
            /// @return bool
            bool ValidateHeader(std::vector<Byte>& data) const;

        protected:
            bool m_isReadMode = false;
            u32 m_version = -1;
            SerialisationTypes m_type;
            std::string m_name;
        };


        //SFINAE - https://en.wikipedia.org/wiki/Substitution_failure_is_not_an_error
        template<typename T> class ObjectSerialiserExists
        {
            template<typename> static std::false_type test(...);
            template<typename U> static auto test(int)
                -> decltype(std::declval<U>().Serialise(), std::true_type());
        public:
            static constexpr bool value
                = std::is_same<decltype(test<T>(0)), std::true_type>::value;
        };

        template<typename T> class PropertySerialiserExists
        {
            template<typename> static std::false_type test(...);
            template<typename U> static auto test(int)
                -> decltype(std::declval<U>()({}), std::true_type());
        public:
            static constexpr bool value
                = std::is_same<decltype(test<T>(0)), std::true_type>::value;
        };
    }
}
