#pragma once

#include "Serialisation/Serialisers/ISerialiser.h"

#include <nlohmann/json.hpp>

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

            virtual void Deserialise(std::vector<u8> data) override;
            virtual std::vector<Byte> GetSerialisedData() const override;

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


        private:
            template<typename T>
            void Write(std::string_view tag, T const& data)
            {
                if (m_writer.TopState() == SerialiserNodeStates::Array)
                {
                    m_writer.TopNode().push_back(data);
                    ++m_writer.Top().ArrayIndex;
                }
                else
                {
                    m_writer.TopNode()[tag] = data;
                }
            }

            template<typename T>
            void ReadValue(std::string_view tag, T& data)
            {
                JsonReader::JsonNode& node = m_reader.Top();
                if (node.NoneStats == SerialiserNodeStates::Array)
                {
                    data = node.Node.at(node.ArrayIndex);
                    ++node.ArrayIndex;
                }
                else
                {
                    if (auto iter = node.Node.find(tag); iter != node.Node.end())
                    {
                        data = iter.value();
                    }
                }
            }

            bool IsObjectNode() const;
            bool IsArrayNode() const;

        private:
            struct JsonReader
            {
                struct JsonNode
                {
                    SerialiserNodeStates NoneStats;
                    u32 ArrayIndex = 0;
                    nlohmann::json Node;
                };

                JsonNode& Top() { return Nodes.top(); }
                JsonNode const& Top() const { return Nodes.top(); }

                nlohmann::json& TopNode() { return Nodes.top().Node; }
                nlohmann::json const& TopNode() const { return Nodes.top().Node; }

                SerialiserNodeStates& TopState() { return Nodes.top().NoneStats; }
                SerialiserNodeStates const& TopState() const { return Nodes.top().NoneStats; }

                void Push(std::string_view name, SerialiserNodeStates state)
                {
                    if (Size() == 0)
                    {
                        Nodes.push(
                            JsonNode
                            {
                                state,
                                0,
                                DeserialisedJson
                            }
                        );
                    }
                    else
                    {
                        if (TopState() == SerialiserNodeStates::Array)
                        {
                            Nodes.push(
                                JsonNode
                                {
                                    state,
                                    0,
                                    TopNode().at(Top().ArrayIndex)
                                }
                            );
                        }
                        else if (TopState() == SerialiserNodeStates::Object)
                        {
                            Nodes.push(
                                JsonNode
                                {
                                    state,
                                    0,
                                    TopNode()[name]
                                }
                            );
                        }
                        else
                        {
                            assert(false);
                        }
                    }
                }
                void Pop()
                {
                    if (Nodes.size() == 1)
                    {
                        return;
                    }
                    Nodes.pop();
                    
                    if (TopState() == SerialiserNodeStates::Array)
                    {
                        ++Top().ArrayIndex;
                    }
                }

                u32 Size() const { return static_cast<u32>(Nodes.size()); }

                std::stack<JsonNode> Nodes;
                nlohmann::json DeserialisedJson;
            };
            struct JsonWriter
            {
                struct JsonNode
                {
                    SerialiserNodeStates NoneStats;
                    std::string NodeName;
                    nlohmann::json Node;

                    union
                    {
                        struct
                        {
                            u64 ArraySize;
                            u64 ArrayIndex;
                        };
                    };
                };
                JsonNode& Top() { return Nodes.top(); }
                nlohmann::json& TopNode() { return Nodes.top().Node; }
                SerialiserNodeStates& TopState() { return Nodes.top().NoneStats; }

                JsonNode const& Top() const { return Nodes.top(); }
                nlohmann::json const& TopNode() const { return Nodes.top().Node; }
                SerialiserNodeStates const& TopState() const { return Nodes.top().NoneStats; }

                void Push(std::string_view name, SerialiserNodeStates state)
                {
                    nlohmann::json jsonObject;

                    Nodes.push(
                        JsonNode
                        {
                            state,
                            std::string(name),
                            jsonObject
                        }
                    );
                }
                void Pop()
                {
                    if (Nodes.size() == 1)
                    {
                        return;
                    }

                    JsonNode node = Nodes.top();
                    Nodes.pop();

                    if (TopState() == SerialiserNodeStates::Array)
                    {
                        Top().Node.push_back(node.Node);
                        ++Top().ArrayIndex;
                    }
                    else if (TopState() == SerialiserNodeStates::Object)
                    {
                        Top().Node[node.NodeName] = node.Node;;
                    }
                    else
                    {
                        assert(false);
                    }
                }

                u32 Size() const { return static_cast<u32>(Nodes.size()); }

                std::stack<JsonNode> Nodes;
            };

            JsonReader m_reader;
            JsonWriter m_writer;
        };
    }
}