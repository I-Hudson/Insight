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
        }

        JsonSerialiser::JsonSerialiser(bool isReadMode)
            : ISerialiser(SerialisationTypes::Json, isReadMode)
        {  }

        std::vector<Byte> JsonSerialiser::GetSerialisedData()
        {
            nlohmann::json root = m_json;

            std::function<void(JsonSerialiser*, nlohmann::json*)> addChildJsonObjects;
            
            addChildJsonObjects = [&addChildJsonObjects](JsonSerialiser* serialiser, nlohmann::json* json)
            {
                for (size_t i = 0; i < serialiser->m_childSerialisers.size(); ++i)
                {
                    JsonSerialiser* child = static_cast<JsonSerialiser*>(serialiser->m_childSerialisers.at(i));
                    ASSERT(child);
                    nlohmann::json childJson = child->m_json;
                    addChildJsonObjects(child, &childJson);

                    (*json)[std::string(child->GetName())] = childJson;
                }
            };
            addChildJsonObjects(this, &root);

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

        void JsonSerialiser::Read(std::string_view tag, const char* cStr, u64 size)
        {
        }
    }
}