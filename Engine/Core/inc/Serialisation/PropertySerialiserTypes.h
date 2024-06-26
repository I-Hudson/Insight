#pragma once

#include "Core/GUID.h"

namespace Insight
{
    namespace Serialisation
    {
        template<>
        struct PropertySerialiser<Core::GUID>
        {
            using InType = Core::GUID;
            std::string operator()(Core::GUID const& object)
            {
                std::string guidString = object.ToString();
                Core::GUID stringToGuid;
                stringToGuid.StringToGuid(guidString);
                assert(stringToGuid == object);
                return object.ToString();
            }
        };

        template<>
        struct PropertySerialiser<bool>
        {
            bool operator()(bool v)
            {
                return v;
            }
        };

        template<>
        struct PropertySerialiser<char>
        {
            char operator()(char v)
            {
                return v;
            }
        };

        template<>
        struct PropertySerialiser<float>
        {
            float operator()(float v)
            {
                return v;
            }
        };

        template<>
        struct PropertySerialiser<u8>
        {
            u8 operator()(u8 v)
            {
                return v;
            }
        };
        template<>
        struct PropertySerialiser<u16>
        {
            u16 operator()(u16 v)
            {
                return v;
            }
        };
        template<>
        struct PropertySerialiser<u32>
        {
            u32 operator()(u32 v)
            {
                return v;
            }
        };
        template<>
        struct PropertySerialiser<u64>
        {
            u64 operator()(u64 v)
            {
                return v;
            }
        };

        template<>
        struct PropertySerialiser<i8>
        {
            i8 operator()(i8 v)
            {
                return v;
            }
        };
        template<>
        struct PropertySerialiser<i16>
        {
            i16 operator()(i16 v)
            {
                return v;
            }
        };
        template<>
        struct PropertySerialiser<i32>
        {
            i32 operator()(i32 v)
            {
                return v;
            }
        };
        template<>
        struct PropertySerialiser<i64>
        {
            i64 operator()(i64 v)
            {
                return v;
            }
        };

        template<>
        struct PropertySerialiser<std::string>
        {
            std::string operator()(std::string const& data)
            {
                return data;
            }
        };

#if 0
        //================================================================
        // GLM TYPES
        //================================================================
        template<>
        struct PropertySerialiser<glm::vec2>
        {
            std::string operator()(glm::vec2 v)
            {
                std::string str;
                str += "(" + std::to_string(v.x);
                str += ",";
                str += std::to_string(v.y) + ")";
                return str;
            }
        };
        template<>
        struct PropertySerialiser<glm::vec3>
        {
            std::string operator()(glm::vec3 v)
            {
                std::string str;
                str += "(" + std::to_string(v.x);
                str += ",";
                str += std::to_string(v.y);
                str += ",";
                str += std::to_string(v.z) + ")";
                return str;
            }
        };
        template<>
        struct PropertySerialiser<glm::vec4>
        {
            std::string operator()(glm::vec4 v)
            {
                std::string str;
                str += "(" + std::to_string(static_cast<float>(v.x));
                str += ",";
                str += std::to_string(static_cast<float>(v.y));
                str += ",";
                str += std::to_string(static_cast<float>(v.z));
                str += ",";
                str += std::to_string(static_cast<float>(v.w)) + ")";
                return str;
            }
        };
        template<>
        struct PropertySerialiser<glm::ivec2>
        {
            std::string operator()(glm::ivec2 v)
            {
                std::string str;
                str += "(" + std::to_string(v.x);
                str +=",";
                str += std::to_string(v.y) + ")";
                return str;
            }
        };
        template<>
        struct PropertySerialiser<glm::ivec3>
        {
            std::string operator()(glm::ivec3 v)
            {
                std::string str;
                str += "(" + std::to_string(v.x);
                str += ",";
                str += std::to_string(v.y);
                str += ",";
                str += std::to_string(v.z) + ")";
                return str;
            }
        };
        template<>
        struct PropertySerialiser<glm::ivec4>
        {
            std::string operator()(glm::ivec4 v)
            {
                std::string str;
                str += "(" + std::to_string(v.x);
                str += ",";
                str += std::to_string(v.y);
                str += ",";
                str += std::to_string(v.z);
                str += ",";
                str += std::to_string(v.w) + ")";
                return str;
            }
        };

        template<>
        struct PropertySerialiser<glm::mat2>
        {
            std::string operator()(glm::mat2 v)
            {
                PropertySerialiser<glm::vec2> vecSerialiser;
                std::string str;
                str += vecSerialiser(v[0]);
                str += vecSerialiser(v[1]);
                return str;
            }
        };
        template<>
        struct PropertySerialiser<glm::mat3>
        {
            std::string operator()(glm::mat3 v)
            {
                PropertySerialiser<glm::vec3> vecSerialiser;
                std::string str;
                str += vecSerialiser(v[0]);
                str += vecSerialiser(v[1]);
                str += vecSerialiser(v[2]);
                return str;
            }
        };
        template<>
        struct PropertySerialiser<glm::mat4>
        {
            std::string operator()(glm::mat4 v)
            {
                PropertySerialiser<glm::vec4> vecSerialiser;
                std::string str;
                str += vecSerialiser(v[0]);
                str += vecSerialiser(v[1]);
                str += vecSerialiser(v[2]);
                str += vecSerialiser(v[3]);
                return str;
            }
        };
#endif
    }
}