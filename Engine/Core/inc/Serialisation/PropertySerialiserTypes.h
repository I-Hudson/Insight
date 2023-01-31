#pragma once

#include "Serialisation/SerialiserBase.h"

#include "Serialisation/PropertySerialiser.h"
#include "Serialisation/SerialiserBase.h"

#include "Platforms/Platform.h"

#include <glm/glm.hpp>

namespace Insight
{
    namespace Serialisation
    {
        template<>
        struct PropertySerialiser<bool>
        {
            std::string operator()(bool v)
            {
                return std::to_string(v);
            }
        };

        template<>
        struct PropertySerialiser<char>
        {
            std::string operator()(char v)
            {
                return std::to_string(v);
            }
        };

        template<>
        struct PropertySerialiser<u8>
        {
            std::string operator()(u8 v)
            {
                return std::to_string(v);
            }
        };
        template<>
        struct PropertySerialiser<u16>
        {
            std::string operator()(u32 v)
            {
                return std::to_string(v);
            }
        };
        template<>
        struct PropertySerialiser<u32>
        {
            std::string operator()(u32 v)
            {
                return std::to_string(v);
            }
        };
        template<>
        struct PropertySerialiser<u64>
        {
            std::string operator()(u64 v)
            {
                return std::to_string(v);
            }
        };

        template<>
        struct PropertySerialiser<i8>
        {
            std::string operator()(i8 v)
            {
                return std::to_string(v);
            }
        };
        template<>
        struct PropertySerialiser<i16>
        {
            std::string operator()(i16 v)
            {
                return std::to_string(v);
            }
        };
        template<>
        struct PropertySerialiser<i32>
        {
            std::string operator()(i32 v)
            {
                return std::to_string(v);
            }
        };
        template<>
        struct PropertySerialiser<i64>
        {
            std::string operator()(i64 v)
            {
                return std::to_string(v);
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


        //================================================================
        // GLM TYPES
        //================================================================
        template<>
        struct PropertySerialiser<glm::vec2>
        {
            std::string operator()(glm::vec2 v)
            {
                PropertySerialiser<float> intSerilaiser;
                std::string str;
                str += "(" + intSerilaiser(v.x);
                str += ",";
                str += intSerilaiser(v.y) + ")";
                return str;
            }
        };
        template<>
        struct PropertySerialiser<glm::vec3>
        {
            std::string operator()(glm::vec3 v)
            {
                PropertySerialiser<float> intSerilaiser;
                std::string str;
                str += "(" + intSerilaiser(v.x);
                str += ",";
                str += intSerilaiser(v.y);
                str += ",";
                str += intSerilaiser(v.z) + ")";
                return str;
            }
        };
        template<>
        struct PropertySerialiser<glm::vec4>
        {
            std::string operator()(glm::vec4 v)
            {
                PropertySerialiser<float> intSerilaiser;
                std::string str;
                str += "(" + intSerilaiser(v.x);
                str += ",";
                str += intSerilaiser(v.y);
                str += ",";
                str += intSerilaiser(v.z);
                str += ",";
                str += intSerilaiser(v.w) + ")";
                return str;
            }
        };
        template<>
        struct PropertySerialiser<glm::ivec2>
        {
            std::string operator()(glm::ivec2 v)
            {
                PropertySerialiser<int> intSerilaiser;
                std::string str;
                str += "(" + intSerilaiser(v.x);
                str +=",";
                str += intSerilaiser(v.y) + ")";
                return str;
            }
        };
        template<>
        struct PropertySerialiser<glm::ivec3>
        {
            std::string operator()(glm::ivec3 v)
            {
                PropertySerialiser<int> intSerilaiser;
                std::string str;
                str += "(" + intSerilaiser(v.x);
                str += ",";
                str += intSerilaiser(v.y);
                str += ",";
                str += intSerilaiser(v.z) + ")";
                return str;
            }
        };
        template<>
        struct PropertySerialiser<glm::ivec4>
        {
            std::string operator()(glm::ivec4 v)
            {
                PropertySerialiser<int> intSerilaiser;
                std::string str;
                str += "(" + intSerilaiser(v.x);
                str += ",";
                str += intSerilaiser(v.y);
                str += ",";
                str += intSerilaiser(v.z);
                str += ",";
                str += intSerilaiser(v.w) + ")";
                return str;
            }
        };
    }
}