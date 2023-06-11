#pragma once

#include "Serialisation/Serialisers/ISerialiser.h"

#include <stack>

namespace Insight
{
    namespace Serialisation
    {
        class BinaryHead
        {
        public:
            BinaryHead();
            ~BinaryHead();

            void Write(std::string_view tag, const void* data, u64 sizeBytes);
            void Read(std::string_view tag, const void* data, u64 sizeBytes);
            void Resize(u64 newSize);

            void PushState(SerialiserNodeStates state);
            void PopState();
            SerialiserNodeStates GetCurrentState() const;

            void Clear();
            void Deserialise(const std::vector<Byte>& data);

        public:
            Byte* Data = nullptr;
            u64 Capacity = 0;
            u64 Size = 0;
            std::stack<SerialiserNodeStates> NodeStates;
        };

        class IS_CORE BinarySerialiser : public ISerialiser
        {
        public:
            BinarySerialiser();
            BinarySerialiser(bool isReadMode);
            virtual ~BinarySerialiser() override;

            virtual bool Deserialise(std::vector<u8> data) override;
            virtual std::vector<Byte> GetSerialisedData() override;

            virtual void Clear() override;

            virtual void StartObject(std::string_view name) override;
            virtual void StopObject() override;

            virtual void StartArray(std::string_view name, u64& size) override;
            virtual void StopArray() override;

            virtual void Write(std::string_view tag, bool data) override;
            virtual void Write(std::string_view tag, char data) override;
            virtual void Write(std::string_view tag, float data) override;

            virtual void Write(std::string_view tag, u8 data) override;
            virtual void Write(std::string_view tag, u16 data) override;
            virtual void Write(std::string_view tag, u32 data) override;
            virtual void Write(std::string_view tag, u64 data) override;

            virtual void Write(std::string_view tag, i8 data) override;
            virtual void Write(std::string_view tag, i16 data) override;
            virtual void Write(std::string_view tag, i32 data) override;
            virtual void Write(std::string_view tag, i64 data) override;

            virtual void Write(std::string_view tag, std::string const& string) override;
            virtual void Write(std::string_view tag, const std::vector<Byte>& vector) override;
            virtual void Write(std::string_view tag, const void* data, const u64 size) override;

            virtual void Read(std::string_view tag, bool& data) override;
            virtual void Read(std::string_view tag, char& data) override;
            virtual void Read(std::string_view tag, float& data) override;

            virtual void Read(std::string_view tag, u8& data) override;
            virtual void Read(std::string_view tag, u16& data) override;
            virtual void Read(std::string_view tag, u32& data) override;
            virtual void Read(std::string_view tag, u64& data) override;

            virtual void Read(std::string_view tag, i8& data) override;
            virtual void Read(std::string_view tag, i16& data) override;
            virtual void Read(std::string_view tag, i32& data) override;
            virtual void Read(std::string_view tag, i64& data) override;

            virtual void Read(std::string_view tag, std::string& string) override;
            virtual void Read(std::string_view tag, std::vector<Byte>& vector) override;

        protected:
            virtual bool ReadType(std::vector<Byte>& data) override;

        private:
            template<typename T>
            void Write(std::string_view tag, T const& data)
            {
                WriteBlock(tag, &data, sizeof(T));
            }

            /// @brief Write a single block of memory to the serialiser.
            /// @param data 
            /// @param sizeBytes 
            void WriteBlock(std::string_view tag, const void* data, u64 sizeBytes);

            template<typename T>
            void ReadValue(std::string_view tag, T& data)
            {
                ReadBlock(tag, &data, sizeof(T));
            }

            /// @brief Read a block of memory from the serialiser.
            /// @param tag 
            /// @param data 
            /// @param sizeBytes 
            void ReadBlock(std::string_view tag, const void* data, u64 sizeBytes);

            bool IsObjectNode() const;
            bool IsArrayNode() const;

        private:
            BinaryHead m_head;
        };
    }
}