#include "Serialisation/JsonSerialiser.h"

#include "Platforms/Platform.h"

#include <functional>

namespace Insight
{
    namespace Serialisation
    {
        constexpr char* c_SerialiserName = "SerialiserName";
        constexpr char* c_ChildSerialiser = "ChildSerialiser";

        JsonSerialiser::JsonSerialiser()
            : ISerialiser(SerialisationTypes::Json, false)
        { 
        }

        JsonSerialiser::JsonSerialiser(bool isReadMode)
            : ISerialiser(SerialisationTypes::Json, isReadMode)
        {  }

        void JsonSerialiser::Deserialise(std::vector<u8> data)
        {
            nlohmann::json rootJson = nlohmann::json::parse(data);

            std::function<void(JsonSerialiser*, nlohmann::json const&)> addChildSerilaiser;
            addChildSerilaiser = [&addChildSerilaiser](JsonSerialiser* serialiser, nlohmann::json const& json)
            {
                serialiser->m_json = json;
                if (auto nameIter = serialiser->m_json.find(c_SerialiserName);
                    nameIter != serialiser->m_json.end())
                {
                    serialiser->SetName(nameIter.value());
                }

                nlohmann::json const& childSerilaisers = serialiser->m_json[c_ChildSerialiser];
                for (size_t i = 0; i < childSerilaisers.size(); ++i)
                {
                    nlohmann::json const& childSerilaiser = childSerilaisers.at(i);
                    addChildSerilaiser(static_cast<JsonSerialiser*>(serialiser->AddChildSerialiser(true)), childSerilaiser);
                }
            };
            addChildSerilaiser(this, rootJson);
        }

        std::vector<Byte> JsonSerialiser::GetSerialisedData() const
        {
            nlohmann::json root = m_json;
            root[c_SerialiserName] = GetName();
                
            std::function<void(JsonSerialiser const*, nlohmann::json&)> addChildJsonObjects;
            
            addChildJsonObjects = [&addChildJsonObjects](JsonSerialiser const* serialiser, nlohmann::json& json)
            {
                for (size_t i = 0; i < serialiser->m_childSerialisers.size(); ++i)
                {
                    JsonSerialiser* child = static_cast<JsonSerialiser*>(serialiser->m_childSerialisers.at(i));
                    ASSERT(child);
                    nlohmann::json childJson = child->m_json;
                    addChildJsonObjects(child, childJson);

                    childJson[c_SerialiserName] = child->GetName();
                    json[c_ChildSerialiser].push_back(childJson);
                }
            };
            addChildJsonObjects(this, root);

            std::string jsonData = root.dump(4);
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

        void JsonSerialiser::Write(std::string_view tag, std::string const& string)
        {
            m_json[tag] = string;
        }

        void JsonSerialiser::Read(std::string_view tag, bool& data)
        {
            if (auto iter = m_json.find(tag); iter != m_json.end())
            {
                data = iter.value();
            }
        }

        void JsonSerialiser::Read(std::string_view tag, u8& data)
        {
            if (auto iter = m_json.find(tag); iter != m_json.end())
            {
                data = iter.value();
            }
        }
        void JsonSerialiser::Read(std::string_view tag, u16& data)
        {
            if (auto iter = m_json.find(tag); iter != m_json.end())
            {
                data = iter.value();
            }
        }
        void JsonSerialiser::Read(std::string_view tag, u32& data)
        {
            if (auto iter = m_json.find(tag); iter != m_json.end())
            {
                data = iter.value();
            }
        }
        void JsonSerialiser::Read(std::string_view tag, u64& data)
        {
            if (auto iter = m_json.find(tag); iter != m_json.end())
            {
                data = iter.value();
            }
        }

        void JsonSerialiser::Read(std::string_view tag, i8& data)
        {
            if (auto iter = m_json.find(tag); iter != m_json.end())
            {
                data = iter.value();
            }
        }
        void JsonSerialiser::Read(std::string_view tag, i16& data)
        {
            if (auto iter = m_json.find(tag); iter != m_json.end())
            {
                data = iter.value();
            }
        }
        void JsonSerialiser::Read(std::string_view tag, i32& data)
        {
            if (auto iter = m_json.find(tag); iter != m_json.end())
            {
                data = iter.value();
            }
        }
        void JsonSerialiser::Read(std::string_view tag, i64& data)
        {
            if (auto iter = m_json.find(tag); iter != m_json.end())
            {
                data = iter.value();
            }
        }

        void JsonSerialiser::Read(std::string_view tag, std::string& string)
        {
            if (auto iter = m_json.find(tag); iter != m_json.end())
            {
                if (!iter.value().is_string())
                {
                    return;
                }
                string = iter.value();
            }
        }
    }
}