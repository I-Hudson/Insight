#pragma once

#include "Serialisation/Serialiser.h"

#include <stack>

namespace Insight
{
    namespace Serialisation
    {
        class IS_CORE JsonSerialiser : public ISerialiser
        {
        public:
            JsonSerialiser();
            JsonSerialiser(bool isReadMode);

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

            virtual void Write(std::string_view tag, const char* cStr, u64 size) override;

            virtual void Read(std::string_view tag, bool& data) override;

            virtual void Read(std::string_view tag, u8& data) override;
            virtual void Read(std::string_view tag, u16& data) override;
            virtual void Read(std::string_view tag, u32& data) override;
            virtual void Read(std::string_view tag, u64& data) override;

            virtual void Read(std::string_view tag, i8& data) override;
            virtual void Read(std::string_view tag, i16& datan) override;
            virtual void Read(std::string_view tag, i32& data) override;
            virtual void Read(std::string_view tag, i64& data) override;

            virtual void Read(std::string_view tag, const char* cStr, u64 size) override;

            virtual ISerialiser* AddChildSerialiser() override;

        private:
            nlohmann::json m_json;
        };
    }
}