#include "Serialisation/Serialisers/BinarySerialiser.h"

#include "Platforms/Platform.h"
#include "Core/Memory.h"
#include "Core/Logger.h"

namespace Insight
{
    namespace Serialisation
    {
        BinaryHead::BinaryHead()
        {
            Resize(1024);
        }

        BinaryHead::~BinaryHead()
        {
            if (Data)
            {
                Clear();
                Capacity = 0;
                DeleteBytes(Data);
            }
        }

        void BinaryHead::Write(std::string_view tag, const void* data, u64 sizeBytes)
        {
            const u64 requiredSize = Size + sizeBytes;
            if (requiredSize > Capacity)
            {
                Resize(AlignUp(requiredSize, 4));
            }
            Platform::MemCopy(Data + Size, data, sizeBytes);
            Size += sizeBytes;
        }

        void BinaryHead::Read(std::string_view tag, const void* data, u64 sizeBytes)
        {
            Platform::MemCopy((void*)data, Data + Size, sizeBytes);
            ASSERT(Size + sizeBytes <= Capacity);
            Size += sizeBytes;
        }

        void BinaryHead::Resize(u64 newSize)
        {
            if (newSize <= Capacity)
            {
                return;
            }

            Byte* newBlock = static_cast<Byte*>(NewBytes(newSize));
            Platform::MemSet(newBlock, 0, newSize);
            if (Data)
            {
                Platform::MemCopy(newBlock, Data, Capacity);
                Delete(Data);
            }
            Data = newBlock;
            Capacity = newSize;
        }

        void BinaryHead::PushState(SerialiserNodeStates state)
        {
            BinaryNode node;
            node.State = state;
            Nodes.push(node);
        }

        void BinaryHead::PopState()
        {
            Nodes.pop();
        }

        SerialiserNodeStates BinaryHead::GetCurrentState() const
        {
            return Nodes.top().State;
        }

        BinaryNode& BinaryHead::Top()
        {
            return Nodes.top();
        }

        void BinaryHead::Clear()
        {
            Size = 0;
        }

        void BinaryHead::Deserialise(const std::vector<Byte>& data)
        {
            Resize(data.size());
            Platform::MemCopy(Data, data.data(), data.size());
            Clear();
        }


        BinarySerialiser::BinarySerialiser()
            : ISerialiser(SerialisationTypes::Binary, false)
        {
            m_head.Clear();
        }

        BinarySerialiser::BinarySerialiser(bool isReadMode)
            : ISerialiser(SerialisationTypes::Binary, isReadMode)
        {
            m_head.Clear();
        }

        BinarySerialiser::~BinarySerialiser()
        {
            m_head.Clear();
        }

        u64 BinarySerialiser::GetHeadPosition() const
        {
            return m_head.Size;
        }

        void BinarySerialiser::Skip(u64 size)
        {
            ASSERT((m_head.Size + size) < m_head.Capacity);
            m_head.Size += size;
        }

        bool BinarySerialiser::AtEnd() const
        {
            return m_head.Size == m_head.Capacity;
        }

        // -- Begin ISerialiser --
        bool BinarySerialiser::IsEmpty() const
        {
            return m_head.Size <= 8;
        }

        bool BinarySerialiser::Deserialise(std::vector<u8> data)
        {
            m_head.Deserialise(data);
            return true;
        }

        std::vector<Byte> BinarySerialiser::GetSerialisedData()
        {
            WriteType();

            std::vector<Byte> serialisedData;
            serialisedData.resize(m_head.Size);
            Platform::MemCopy(serialisedData.data(), m_head.Data, m_head.Size);

            return serialisedData;
        }

        void BinarySerialiser::Clear()
        {
            m_head.Clear();
        }

        void BinarySerialiser::StartObject(std::string_view name)
        {
            m_head.PushState(SerialiserNodeStates::Object);
            if (IsReadMode())
            {
                Read(std::string(name) + "ObjectSize", m_head.Top().Size);
            }
            else
            {
                Write(std::string(name) + "ObjectSize", 0ull);
            }

            m_head.Top().StartPosition = GetHeadPosition();
        }

        void BinarySerialiser::StopObject()
        {
            ASSERT(IsObjectNode());

            if (!IsReadMode())
            {
                m_head.Top().Size = GetHeadPosition() - m_head.Top().StartPosition;
                void* dst = (m_head.Data + m_head.Top().StartPosition) - sizeof(u64);
                void* src = &m_head.Top().Size;
                Platform::MemCopy(dst, src, sizeof(u64));
            }
            m_head.PopState();
        }

        void BinarySerialiser::SkipObject()
        {
            ASSERT(IsReadMode());
            StartObject("SKIP");
            Skip(m_head.Top().Size);
            StopObject();
        }

        void BinarySerialiser::StartArray(std::string_view name, u64& size, bool encodeSize)
        {
            m_head.PushState(SerialiserNodeStates::Array);
            if (IsReadMode())
            {
                if (encodeSize)
                {
                    Read(name, size);
                }
            }
            else
            {
                if (encodeSize)
                {
                    Write(name, size);
                }
            }
        }

        void BinarySerialiser::StopArray()
        {
            ASSERT(IsArrayNode());
            m_head.PopState();
        }

        void BinarySerialiser::Write(std::string_view tag, bool data)
        {
            Write<bool>(tag, data);
        }

        void BinarySerialiser::Write(std::string_view tag, char data)
        {
            Write<char>(tag, data);
        }

        void BinarySerialiser::Write(std::string_view tag, float data)
        {
            Write<float>(tag, data);
        }

        void BinarySerialiser::Write(std::string_view tag, u8 data)
        {
            Write<u8>(tag, data);
        }
        void BinarySerialiser::Write(std::string_view tag, u16 data)
        {
            Write<u16>(tag, data);
        }
        void BinarySerialiser::Write(std::string_view tag, u32 data)
        {
            Write<u32>(tag, data);
        }
        void BinarySerialiser::Write(std::string_view tag, u64 data)
        {
            Write<u64>(tag, data);
        }

        void BinarySerialiser::Write(std::string_view tag, i8 data)
        {
            Write<i8>(tag, data);
        }
        void BinarySerialiser::Write(std::string_view tag, i16 data)
        {
            Write<i16>(tag, data);
        }
        void BinarySerialiser::Write(std::string_view tag, i32 data)
        {
            Write<i32>(tag, data);
        }
        void BinarySerialiser::Write(std::string_view tag, i64 data)
        {
            Write<i64>(tag, data);
        }

        void BinarySerialiser::Write(std::string_view tag, std::string const& string)
        {
            u64 arraySize = string.size();
            StartArray(tag, arraySize);
            WriteBlock(tag, string.data(), arraySize);
            StopArray();
        }

        void BinarySerialiser::Write(std::string_view tag, const std::vector<Byte>& vector, bool encodeSize)
        {
            u64 arraySize = vector.size();
            StartArray(tag, arraySize, encodeSize);
            WriteBlock(tag, vector.data(), vector.size());
            StopArray();
        }

        void BinarySerialiser::Read(std::string_view tag, bool& data)
        {
            ReadValue<bool>(tag, data);
        }

        void BinarySerialiser::Read(std::string_view tag, char& data)
        {
            ReadValue<char>(tag, data);
        }

        void BinarySerialiser::Read(std::string_view tag, float& data)
        {
            ReadValue<float>(tag, data);
        }

        void BinarySerialiser::Read(std::string_view tag, u8& data)
        {
            ReadValue<u8>(tag, data);
        }
        void BinarySerialiser::Read(std::string_view tag, u16& data)
        {
            ReadValue<u16>(tag, data);
        }
        void BinarySerialiser::Read(std::string_view tag, u32& data)
        {
            ReadValue<u32>(tag, data);
        }
        void BinarySerialiser::Read(std::string_view tag, u64& data)
        {
            ReadValue<u64>(tag, data);
        }

        void BinarySerialiser::Read(std::string_view tag, i8& data)
        {
            ReadValue<i8>(tag, data);
        }
        void BinarySerialiser::Read(std::string_view tag, i16& data)
        {
            ReadValue<i16>(tag, data);
        }
        void BinarySerialiser::Read(std::string_view tag, i32& data)
        {
            ReadValue<i32>(tag, data);
        }
        void BinarySerialiser::Read(std::string_view tag, i64& data)
        {
            ReadValue<i64>(tag, data);
        }

        void BinarySerialiser::Read(std::string_view tag, std::string& string)
        {
            u64 arraySize = 0;
            StartArray(tag, arraySize);
            string.resize(arraySize);
            ReadBlock(tag, string.data(), arraySize);
            StopArray();
        }

        void BinarySerialiser::Read(std::string_view tag, std::vector<Byte>& vector, bool decodeSize)
        {
            u64 arraySize = 0;
            StartArray(tag, arraySize, decodeSize);
            if (decodeSize)
            {
                vector.resize(arraySize);
            }
            ReadBlock(tag, vector.data(), vector.size());
            StopArray();
        }

        bool BinarySerialiser::ReadType(std::vector<Byte>& data)
        {
            u32 type = 0;
            Platform::MemCopy(&type, &(*(data.end() - 4)), sizeof(u32));

            SerialisationTypes serialiserType = static_cast<SerialisationTypes>(type);
            if (serialiserType != m_type)
            {
                IS_CORE_ERROR("[BinarySerialiser::Deserialise] 'data' has been serialised with type '{}' trying to deserialise with '{}'. Serialiser type mismatch.",
                    SerialisationTypeToString[(u32)type], SerialisationTypeToString[(u32)m_type]);
                return false;
            }
            return true;
        }
        // -- End ISerialiser --

        void BinarySerialiser::WriteBlock(std::string_view tag, const void* data, u64 sizeBytes)
        {
            m_head.Write(tag, data, sizeBytes);
        }

        void BinarySerialiser::ReadBlock(std::string_view tag, const void* data, u64 sizeBytes)
        {
            m_head.Read(tag, data, sizeBytes);
        }

        bool BinarySerialiser::IsObjectNode() const
        {
            return m_head.GetCurrentState() == SerialiserNodeStates::Object;
        }

        bool BinarySerialiser::IsArrayNode() const
        {
            return m_head.GetCurrentState() == SerialiserNodeStates::Array;
        }
    }
}