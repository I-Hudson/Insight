#pragma once

#include "Serialisation/Serialisers/ISerialiser.h"

#include <nlohmann/json.hpp>

#include <stack>

namespace Insight
{
    namespace Serialisation
    {
        constexpr char* c_ObjectName = "ObjectName";
        constexpr char* c_ArrayName = "ArrayName";
        constexpr char* c_SerialiserName = "SerialiserName";
        constexpr char* c_ChildSerialiser = "ChildSerialiser";
        constexpr char* c_ArraySize = "ArraySize";

        class IS_CORE JsonSerialiser : public ISerialiser
        {
        public:
            JsonSerialiser();
            JsonSerialiser(bool isReadMode);

            virtual void Deserialise(std::vector<u8> data) override;
            virtual std::vector<Byte> GetSerialisedData() const override;

            virtual void StartObject(std::string_view name) override;
            virtual void StopObject() override;

            virtual void StartArray(std::string_view name, u64 const size) override;
            virtual void StopArray() override;

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
            template<typename T>
            void Write(std::string_view tag, T const& data)
            {
                if (m_writer.TopState() == NodeStates::Array)
                {
                    m_writer.TopNode().push_back(data);
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
                if (node.NoneStats == NodeStates::Array)
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

        private:
            enum class NodeStates { None, Object, Array };

            struct JsonReader
            {
                struct JsonNode
                {
                    NodeStates NoneStats;
                    u32 ArrayIndex = 0;
                    nlohmann::json Node;
                };

                JsonNode& Top() { return Nodes.top(); }
                nlohmann::json& TopNode() { return Nodes.top().Node; }
                NodeStates& TopState() { return Nodes.top().NoneStats; }

                void Push(std::string_view name, NodeStates state)
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
                        if (TopState() == NodeStates::Array)
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
                        else if (TopState() == NodeStates::Object)
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
                    
                    if (TopState() == NodeStates::Array)
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
                    NodeStates NoneStats;
                    std::string NodeName;
                    nlohmann::json Node;
                };
                JsonNode& Top() { return Nodes.top(); }
                nlohmann::json& TopNode() { return Nodes.top().Node; }
                NodeStates& TopState() { return Nodes.top().NoneStats; }

                JsonNode const& Top() const { return Nodes.top(); }
                nlohmann::json const& TopNode() const { return Nodes.top().Node; }
                NodeStates const& TopState() const { return Nodes.top().NoneStats; }

                void Push(std::string_view name, NodeStates state)
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

                    if (TopState() == NodeStates::Array)
                    {
                        Top().Node.push_back(node.Node);
                    }
                    else if (TopState() == NodeStates::Object)
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