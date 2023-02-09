#pragma once

#include "Core/Defines.h"
#include "Core/TypeAlias.h"

#include <vector>
#include <string_view>

namespace Insight
{
    namespace Serialisation
    {
        enum class SerialisationTypes
        {
            Json,
            Binary
        };

        enum class SerialiserWriteReadTypes
        {
            Property,
            Object
        };

        /// @brief Base class for all serialises used. This defines what data can be saved.
        class IS_CORE ISerialiser
        {
        public:
            ISerialiser(SerialisationTypes type);
            ISerialiser(SerialisationTypes type, bool isReadMode);
            virtual ~ISerialiser();

            static ISerialiser* Create(SerialisationTypes type, bool isReadMode);

            void SetName(std::string_view tag);
            std::string_view GetName() const;

            void SetVersion(u32 currentVersion);
            u32 GetVersion() const;
            SerialisationTypes GetType() const;
            bool IsReadMode() const;

            void Write(std::string_view tag, const char* string);

            virtual void Deserialise(std::vector<u8> data) = 0;
            virtual std::vector<Byte> GetSerialisedData() const = 0;

            virtual void StartObject(std::string_view name) = 0;
            virtual void StopObject() = 0;

            virtual void StartArray(std::string_view name, u64 const size) = 0;
            virtual void StopArray() = 0;

            virtual void Write(std::string_view tag, bool data) = 0;

            virtual void Write(std::string_view tag, u8 data) = 0;
            virtual void Write(std::string_view tag, u16 data) = 0;
            virtual void Write(std::string_view tag, u32 data) = 0;
            virtual void Write(std::string_view tag, u64 data) = 0;

            virtual void Write(std::string_view tag, i8 data) = 0;
            virtual void Write(std::string_view tag, i16 data) = 0;
            virtual void Write(std::string_view tag, i32 data) = 0;
            virtual void Write(std::string_view tag, i64 data) = 0;

            virtual void Write(std::string_view tag, std::string const& string) = 0;

            virtual void Read(std::string_view tag, bool& data) = 0;

            virtual void Read(std::string_view tag, u8& data) = 0;
            virtual void Read(std::string_view tag, u16& data) = 0;
            virtual void Read(std::string_view tag, u32& data) = 0;
            virtual void Read(std::string_view tag, u64& data) = 0;

            virtual void Read(std::string_view tag, i8& data) = 0;
            virtual void Read(std::string_view tag, i16& datan) = 0;
            virtual void Read(std::string_view tag, i32& data) = 0;
            virtual void Read(std::string_view tag, i64& data) = 0;

            virtual void Read(std::string_view tag, std::string& string) = 0;

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
