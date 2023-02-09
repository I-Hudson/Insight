#include "Serialisation/JsonSerialiser.h"

#include "Platforms/Platform.h"

#include <functional>

namespace Insight
{
    namespace Serialisation
    {
        JsonSerialiser::JsonSerialiser()
            : ISerialiser(SerialisationTypes::Json, false)
        { 
            m_reader = {};
            m_writer = {};
        }

        JsonSerialiser::JsonSerialiser(bool isReadMode)
            : ISerialiser(SerialisationTypes::Json, isReadMode)
        {
            m_reader = {};
            m_writer = {};
        }

        void JsonSerialiser::Deserialise(std::vector<u8> data)
        {
            m_reader = {};
            m_reader.DeserialisedJson = nlohmann::json::parse(data);
        }

        std::vector<Byte> JsonSerialiser::GetSerialisedData() const
        {
            std::string jsonData = m_writer.TopNode().dump(4);
            return { jsonData.begin(), jsonData.end() };
        }

        void JsonSerialiser::StartObject(std::string_view name)
        {
            if (IsReadMode())
            {
                m_reader.Push(name, NodeStates::Object);
            }
            else
            {
                m_writer.Push(name, NodeStates::Object);
            }
        }

        void JsonSerialiser::StopObject()
        {
            if (IsReadMode())
            {
                m_reader.Pop();
            }
            else
            {
                m_writer.Pop();
            }
        }

        void JsonSerialiser::StartArray(std::string_view name)
        {
            if (IsReadMode())
            {
                m_reader.Push(name, NodeStates::Array);
            }
            else
            {
                m_writer.Push(name, NodeStates::Array);
            }
        }

        void JsonSerialiser::StopArray()
        {
            if (IsReadMode())
            {
                m_reader.Pop();
            }
            else
            {
                m_writer.Pop();
            }
        }

        void JsonSerialiser::Write(std::string_view tag, bool data)
        {
            Write<bool>(tag, data);
        }

        void JsonSerialiser::Write(std::string_view tag, u8 data)
        {
            Write<u8>(tag, data);
        }
        void JsonSerialiser::Write(std::string_view tag, u16 data)
        {
            Write<u16>(tag, data);
        }
        void JsonSerialiser::Write(std::string_view tag, u32 data)
        {
            Write<u32>(tag, data);
        }
        void JsonSerialiser::Write(std::string_view tag, u64 data)
        {
            Write<u64>(tag, data);
        }

        void JsonSerialiser::Write(std::string_view tag, i8 data)
        {
            Write<i8>(tag, data);
        }
        void JsonSerialiser::Write(std::string_view tag, i16 data)
        {
            Write<i16>(tag, data);
        }
        void JsonSerialiser::Write(std::string_view tag, i32 data)
        {
            Write<i32>(tag, data);
        }
        void JsonSerialiser::Write(std::string_view tag, i64 data)
        {
            Write<i64>(tag, data);
        }

        void JsonSerialiser::Write(std::string_view tag, std::string const& string)
        {
            Write<std::string>(tag, string);
        }

        void JsonSerialiser::Read(std::string_view tag, bool& data)
        {
            ReadValue<bool>(tag, data);
        }

        void JsonSerialiser::Read(std::string_view tag, u8& data)
        {
            ReadValue(tag, data);
        }
        void JsonSerialiser::Read(std::string_view tag, u16& data)
        {
            ReadValue(tag, data);
        }
        void JsonSerialiser::Read(std::string_view tag, u32& data)
        {
            ReadValue(tag, data);
        }
        void JsonSerialiser::Read(std::string_view tag, u64& data)
        {
            ReadValue(tag, data);
        }

        void JsonSerialiser::Read(std::string_view tag, i8& data)
        {
            ReadValue(tag, data);
        }
        void JsonSerialiser::Read(std::string_view tag, i16& data)
        {
            ReadValue(tag, data);
        }
        void JsonSerialiser::Read(std::string_view tag, i32& data)
        {
            ReadValue(tag, data);
        }
        void JsonSerialiser::Read(std::string_view tag, i64& data)
        {
            ReadValue(tag, data);
        }

        void JsonSerialiser::Read(std::string_view tag, std::string& string)
        {
            ReadValue(tag, string);
        }
    }
}