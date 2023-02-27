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
            std::string jsonData = m_writer.TopNode().dump();
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
            ASSERT(IsObjectNode());
            if (IsReadMode())
            {
                m_reader.Pop();
            }
            else
            {
                m_writer.Pop();
            }
        }

        void JsonSerialiser::StartArray(std::string_view name, u64& size)
        {
            if (IsReadMode())
            {
                Read(std::string(name) + c_ArraySize, size);
                m_reader.Push(name, NodeStates::Array);
            }
            else
            {
                Write(std::string(name) + c_ArraySize, size);
                m_writer.Push(name, NodeStates::Array);
                m_writer.Top().ArraySize = size;
            }
        }

        void JsonSerialiser::StopArray()
        {
            ASSERT(IsArrayNode());
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

        void JsonSerialiser::Write(std::string_view tag, char data)
        {
            Write<char>(tag, data);
        }

        void JsonSerialiser::Write(std::string_view tag, float data)
        {
            Write<float>(tag, data);
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
            ReadValue<u8>(tag, data);
        }
        void JsonSerialiser::Read(std::string_view tag, u16& data)
        {
            ReadValue<u16>(tag, data);
        }
        void JsonSerialiser::Read(std::string_view tag, u32& data)
        {
            ReadValue<u32>(tag, data);
        }
        void JsonSerialiser::Read(std::string_view tag, u64& data)
        {
            ReadValue<u64>(tag, data);
        }

        void JsonSerialiser::Read(std::string_view tag, i8& data)
        {
            ReadValue<i8>(tag, data);
        }
        void JsonSerialiser::Read(std::string_view tag, i16& data)
        {
            ReadValue<i16>(tag, data);
        }
        void JsonSerialiser::Read(std::string_view tag, i32& data)
        {
            ReadValue<i32>(tag, data);
        }
        void JsonSerialiser::Read(std::string_view tag, i64& data)
        {
            ReadValue<i64>(tag, data);
        }

        void JsonSerialiser::Read(std::string_view tag, std::string& string)
        {
            ReadValue<std::string>(tag, string);
        }

        bool JsonSerialiser::IsObjectNode() const
        {
            if (m_isReadMode)
            {
                return m_reader.TopState() == NodeStates::Object;
            }
            else
            {
                return m_writer.TopState() == NodeStates::Object;
            }
        }

        bool JsonSerialiser::IsArrayNode() const
        {
            if (m_isReadMode)
            {
                return m_reader.TopState() == NodeStates::Array;
            }
            else
            {
                return m_writer.TopState() == NodeStates::Array;
            }
        }
    }
}