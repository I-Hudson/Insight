#include "Serialisation/Serialisers/JsonSerialiser.h"

#include "Platforms/Platform.h"
#include "Core/Logger.h"

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

        bool JsonSerialiser::Deserialise(std::vector<u8> data)
        {
            m_reader = {};

            u8 serialiserType = data.front() > 0 ? data.front() - '0' : data.front();
            data.erase(data.begin());
            if (serialiserType != static_cast<u8>(m_type))
            {
                IS_CORE_ERROR("[JsonSerialiser::Deserialise] 'data' has been serialised with type '{}'. Serialiser type mismatch.", SerialisationTypeToString[(u8)m_type]);
                return false;
            }

            m_reader.DeserialisedJson = nlohmann::json::parse(data);
            return true;
        }

        std::vector<Byte> JsonSerialiser::GetSerialisedData() const
        {
            std::string jsonData = m_writer.TopNode().dump();
            std::vector<Byte> data = { jsonData.begin(), jsonData.end() };

            data.insert(data.begin(), static_cast<u8>(static_cast<int>(m_type)));
            return data;
        }

        void JsonSerialiser::Clear()
        {
            m_reader = {};
            m_writer = {};
        }

        void JsonSerialiser::StartObject(std::string_view name)
        {
            if (IsReadMode())
            {
                m_reader.Push(name, SerialiserNodeStates::Object);
            }
            else
            {
                m_writer.Push(name, SerialiserNodeStates::Object);
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
                m_reader.Push(name, SerialiserNodeStates::Array);
            }
            else
            {
                Write(std::string(name) + c_ArraySize, size);
                m_writer.Push(name, SerialiserNodeStates::Array);
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
        void JsonSerialiser::Write(std::string_view tag, const std::vector<Byte>& vector)
        {
            u64 arraySize = vector.size();
            StartArray(tag, arraySize);
            for (size_t i = 0; i < arraySize; ++i)
            {
                Write(tag, vector.at(i));
            }
            StopArray();
        }

        void JsonSerialiser::Read(std::string_view tag, bool& data)
        {
            ReadValue<bool>(tag, data);
        }

        void JsonSerialiser::Read(std::string_view tag, char& data)
        {
            std::string str;
            ReadValue<std::string>(tag, str);
            ASSERT(str.size() <= 1);
            data = str.front();
        }

        void JsonSerialiser::Read(std::string_view tag, float& data)
        {
            ReadValue<float>(tag, data);
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
        void JsonSerialiser::Read(std::string_view tag, std::vector<Byte>& vector)
        {
            u64 arraySize = 0;
            StartArray(tag, arraySize);
            vector.resize(arraySize);
            for (size_t i = 0; i < arraySize; ++i)
            {
                Read(tag, vector.at(i));
            }
            StopArray();
        }

        bool JsonSerialiser::IsObjectNode() const
        {
            if (m_isReadMode)
            {
                return m_reader.TopState() == SerialiserNodeStates::Object;
            }
            else
            {
                return m_writer.TopState() == SerialiserNodeStates::Object;
            }
        }

        bool JsonSerialiser::IsArrayNode() const
        {
            if (m_isReadMode)
            {
                return m_reader.TopState() == SerialiserNodeStates::Array;
            }
            else
            {
                return m_writer.TopState() == SerialiserNodeStates::Array;
            }
        }
    }
}