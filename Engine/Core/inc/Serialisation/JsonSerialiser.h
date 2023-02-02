#pragma once

#include "Serialisation/Serialisers/ISerialiser.h"

#include <nlohmann/json.hpp>

namespace Insight
{
    namespace Serialisation
    {
        class IS_CORE JsonSerialiser : public ISerialiser
        {
        public:
            JsonSerialiser();
            JsonSerialiser(bool isReadMode);

            virtual void Deserialise(std::vector<u8> data) override;
            virtual std::vector<Byte> GetSerialisedData() const override;

            virtual void Write(std::string_view tag, bool data) override;

            virtual void Write(std::string_view tag, u8 data) override;
            virtual void Write(std::string_view tag, u16 data) override;
            virtual void Write(std::string_view tag, u32 data) override;
            virtual void Write(std::string_view tag, u64 data) override;

            virtual void Write(std::string_view tag, i8 data) override;
            virtual void Write(std::string_view tag, i16 data) override;
            virtual void Write(std::string_view tag, i32 data) override;
            virtual void Write(std::string_view tag, i64 data) override;

            virtual void Write(std::string_view tag, std::string const& string) override;

            virtual void Read(std::string_view tag, bool& data) override;

            virtual void Read(std::string_view tag, u8& data) override;
            virtual void Read(std::string_view tag, u16& data) override;
            virtual void Read(std::string_view tag, u32& data) override;
            virtual void Read(std::string_view tag, u64& data) override;

            virtual void Read(std::string_view tag, i8& data) override;
            virtual void Read(std::string_view tag, i16& data) override;
            virtual void Read(std::string_view tag, i32& data) override;
            virtual void Read(std::string_view tag, i64& data) override;

            virtual void Read(std::string_view tag, std::string& string) override;

        private:
            nlohmann::json m_json;
        };
    }
}