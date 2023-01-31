#include "Serialisation/JsonSerialiser.h"

namespace Insight
{
    namespace Serialisation
    {
        JsonSerialiser::JsonSerialiser()
            : ISerialiser(false)
        { 
        }

        JsonSerialiser::JsonSerialiser(bool isReadMode)
            : ISerialiser(isReadMode)
        {  }

        std::vector<Byte> JsonSerialiser::GetSerialisedData() const
        {
            for (size_t i = 0; i < m_childSerialisers.size(); ++i)
            {
                m_json.array().push_back(static_cast<JsonSerialiser const*>(m_childSerialisers.at(i))->m_json);
            }
            std::string jsonData = m_json.dump(4);
            return { jsonData.begin(), jsonData.end() };
        }

        void JsonSerialiser::Write(std::string_view tag, bool data)
        {
            m_json[tag] = data;
        }

        void JsonSerialiser::Write(std::string_view tag, u8 data)
        {
            m_json[tag] = data;
        }
        void JsonSerialiser::Write(std::string_view tag, u16 data)
        {
            m_json[tag] = data;
        }
        void JsonSerialiser::Write(std::string_view tag, u32 data)
        {
            m_json[tag] = data;
        }
        void JsonSerialiser::Write(std::string_view tag, u64 data)
        {
            m_json[tag] = data;
        }

        void JsonSerialiser::Write(std::string_view tag, i8 data)
        {
            m_json[tag] = data;
        }
        void JsonSerialiser::Write(std::string_view tag, i16 data)
        {
            m_json[tag] = data;
        }
        void JsonSerialiser::Write(std::string_view tag, i32 data)
        {
            m_json[tag] = data;
        }
        void JsonSerialiser::Write(std::string_view tag, i64 data)
        {
            m_json[tag] = data;
        }

        void JsonSerialiser::Write(std::string_view tag, const char* cStr, u64 size)
        {
            m_json[tag] = cStr;
        }

        void JsonSerialiser::Read(std::string_view tag, bool& data)
        {

        }

        void JsonSerialiser::Read(std::string_view tag, u8& data)
        {

        }
        void JsonSerialiser::Read(std::string_view tag, u16& data)
        {
        }
        void JsonSerialiser::Read(std::string_view tag, u32& data)
        {
        }
        void JsonSerialiser::Read(std::string_view tag, u64& data)
        {
        }

        void JsonSerialiser::Read(std::string_view tag, i8& data)
        {
        }
        void JsonSerialiser::Read(std::string_view tag, i16& data)
        {
        }
        void JsonSerialiser::Read(std::string_view tag, i32& data)
        {
        }
        void JsonSerialiser::Read(std::string_view tag, i64& data)
        {
        }

        void JsonSerialiser::Read(std::string_view tag, const char* cStr, u64 size){}

        ISerialiser* JsonSerialiser::AddChildSerialiser()
        {
            JsonSerialiser* ser = New<JsonSerialiser>();
            m_childSerialisers.push_back(ser);
            return ser;
        }

    }
}