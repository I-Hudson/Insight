#pragma once

#include "Serialisation/PropertySerialiser.h"
#include "Serialisation/SerialiserBase.h"

#include "Platforms/Platform.h"

#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/ext/vector_int2.hpp>
#include <glm/ext/vector_int3.hpp>
#include <glm/ext/vector_int4.hpp>
#include <glm/ext/matrix_float2x2.hpp>
#include <glm/ext/matrix_float3x3.hpp>
#include <glm/ext/matrix_float4x4.hpp>

namespace Insight
{
    namespace Serialisation
    {
        template<>
        struct PropertyDeserialiser<bool>
        {
            using InType = bool;
            bool operator()(InType const data) const
            {
                return data;
            }
        };

        template<>
        struct PropertyDeserialiser<char>
        {
            using InType = char;
            char operator()(InType const data) const
            {
                return data;
            }
        };

        template<>
        struct PropertyDeserialiser<float>
        {
            using InType = float;
            float operator()(InType const data) const
            {
                return data;
            }
        };

        template<>
        struct PropertyDeserialiser<u8>
        {
            using InType = u8;
            u8 operator()(InType const data) const
            {
                return data;
            }
        };
        template<>
        struct PropertyDeserialiser<u16>
        {
            using InType = u16;
            u16 operator()(InType const data) const
            {
                return data;
            }
        };
        template<>
        struct PropertyDeserialiser<u32>
        {
            using InType = u32;
            u32 operator()(InType const data) const
            {
                return data;
            }
        };
        template<>
        struct PropertyDeserialiser<u64>
        {
            using InType = u64;
            u64 operator()(InType const data) const
            {
                return data;
            }
        };

        template<>
        struct PropertyDeserialiser<i8>
        {
            using InType = i8;
            i8 operator()(InType const data) const
            {
                return data;
            }
        };
        template<>
        struct PropertyDeserialiser<i16>
        {
            using InType = i16;
            i16 operator()(InType const data) const
            {
                return data;
            }
        };
        template<>
        struct PropertyDeserialiser<i32>
        {
            using InType = i32;
            i32 operator()(InType const data) const
            {
                return data;
            }
        };
        template<>
        struct PropertyDeserialiser<i64>
        {
            using InType = i64;
            i64 operator()(InType const data) const
            {
                return data;
            }
        };

        template<>
        struct PropertyDeserialiser<std::string>
        {
            using InType = std::string;
            std::string operator()(InType const& data) const
            {
                return data;
            }
        };

        //================================================================
        // GLM TYPES
        //================================================================
        template<>
        struct PropertyDeserialiser<glm::vec2>
        {
            using InType = std::string;
            glm::vec2 operator()(std::string const& data) const
            {
                glm::vec2 vec;
                int idx = 0;
                std::string numString;
                for (size_t i = 0; i < data.size(); ++i)
                {
                    const char c = data.at(i);

                    if (c == ',')
                    {
                        vec[idx] = std::stof(numString);
                        numString.clear();
                        ++idx;
                    }
                    else if (c >= '0' && c <= '9'
                        || c == '.' || c == '-')
                    {
                        numString += c;
                    }
                    else
                    {
                        assert(false && "Ill-formatted string.");
                    }
                }
                vec[idx] = std::stof(numString);

                return vec;
            }
        };
        template<>
        struct PropertyDeserialiser<glm::vec3>
        {
            using InType = std::string;
            glm::vec3 operator()(std::string const& data) const
            {
                glm::vec3 vec;
                int idx = 0;
                std::string numString;
                for (size_t i = 0; i < data.size(); ++i)
                {
                    const char c = data.at(i);

                    if (c == ',')
                    {
                        vec[idx] = std::stof(numString);
                        numString.clear();
                        ++idx;
                    }
                    else if (c >= '0' && c <= '9'
                        || c == '.' || c == '-')
                    {
                        numString += c;
                    }
                    else
                    {
                        assert(false && "Ill-formatted string.");
                    }
                }
                vec[idx] = std::stof(numString);

                return vec;
            }
        };
        template<>
        struct PropertyDeserialiser<glm::vec4>
        {
            using InType = std::string;
            glm::vec4 operator()(std::string const& data) const
            {
                glm::vec4 vec;
                int idx = 0;
                std::string numString;
                for (size_t i = 0; i < data.size(); ++i)
                {
                    const char c = data.at(i);

                    if (c == ',')
                    {
                        vec[idx] = std::stof(numString);
                        numString.clear();
                        ++idx;
                    }
                    else if (c >= '0' && c <= '9'
                        || c=='.' || c == '-')
                    {
                        numString += c;
                    }
                    else
                    {
                        assert(false && "Ill-formatted string.");
                    }
                }
                vec[idx] = std::stof(numString);

                return vec;
            }
        };


        template<>
        struct PropertyDeserialiser<glm::ivec2>
        {
            using InType = std::string;
            glm::ivec2 operator()(std::string const& data) const
            {
                glm::ivec2 vec;
                int idx = 0;
                std::string numString;
                for (size_t i = 0; i < data.size(); ++i)
                {
                    const char c = data.at(i);

                    if (c == ',') 
                    {
                        vec[idx] = std::stoi(numString);
                        numString.clear();
                        ++idx;
                    }
                    else if (c >= '0' && c <= '9'
                        || c == '.' || c == '-')
                    {
                        numString += c;
                    }
                    else
                    {
                        assert(false && "Ill-formatted string.");
                    }
                }
                vec[idx] = std::stoi(numString);

                return vec;
            }
        };
        template<>
        struct PropertyDeserialiser<glm::ivec3>
        {
            using InType = std::string;
            glm::ivec3 operator()(std::string const& data) const
            {
                glm::ivec3 vec;
                int idx = 0;
                std::string numString;
                for (size_t i = 0; i < data.size(); ++i)
                {
                    const char c = data.at(i);

                    if (c == ',')
                    {
                        vec[idx] = std::stoi(numString);
                        numString.clear();
                        ++idx;
                    }
                    else if (c >= '0' && c <= '9'
                        || c == '.' || c == '-')
                    {
                        numString += c;
                    }
                    else
                    {
                        assert(false && "Ill-formatted string.");
                    }
                }
                vec[idx] = std::stoi(numString);

                return vec;
            }
        };
        template<>
        struct PropertyDeserialiser<glm::ivec4>
        {
            using InType = std::string;
            glm::ivec4 operator()(std::string const& data) const
            {
                glm::ivec4 vec;
                int idx = 0;
                std::string numString;
                for (size_t i = 0; i < data.size(); ++i)
                {
                    const char c = data.at(i);

                    if (c == ',')
                    {
                        vec[idx] = std::stoi(numString);
                        numString.clear();
                        ++idx;
                    }
                    else if (c >= '0' && c <= '9'
                        || c == '.' || c == '-')
                    {
                        numString += c;
                    }
                    else
                    {
                        assert(false && "Ill-formatted string.");
                    }
                }
                vec[idx] = std::stoi(numString);

                return vec;
            }
        };

        template<>
        struct PropertyDeserialiser<glm::mat2>
        {
            using InType = std::string;
            glm::mat2 operator()(std::string const& data) const
            {
                PropertyDeserialiser<glm::vec2> vecDeserialiser;
                glm::mat2 vec;
                int idx = 0;
                std::string numString;
                for (size_t i = 0; i < data.size(); ++i)
                {
                    if (data.at(i) == ')')
                    {
                        vec[idx] = vecDeserialiser(numString);
                        numString.clear();
                        ++idx;
                    }
                    else if (data.at(i) != '(' && data.at(i) != ')')
                    {
                        numString += data.at(i);
                    }
                }
                return vec;
            }
        };
        template<>
        struct PropertyDeserialiser<glm::mat3>
        {
            using InType = std::string;
            glm::mat2 operator()(std::string const& data) const
            {
                PropertyDeserialiser<glm::vec3> vecDeserialiser;
                glm::mat3 vec;
                int idx = 0;
                std::string numString;
                for (size_t i = 0; i < data.size(); ++i)
                {
                    if (data.at(i) == ')')
                    {
                        vec[idx] = vecDeserialiser(numString);
                        numString.clear();
                        ++idx;
                    }
                    else if (data.at(i) != '(' && data.at(i) != ')')
                    {
                        numString += data.at(i);
                    }
                }
                return vec;
            }
        };
        template<>
        struct PropertyDeserialiser<glm::mat4>
        {
            using InType = std::string;
            glm::mat4 operator()(std::string const& data) const
            {
                PropertyDeserialiser<glm::vec4> vecDeserialiser;
                glm::mat4 vec;
                int idx = 0;
                std::string numString;
                for (size_t i = 0; i < data.size(); ++i)
                {
                    if (data.at(i) == ')')
                    {
                        vec[idx] = vecDeserialiser(numString);
                        numString.clear();
                        ++idx;
                    }
                    else if (data.at(i) != '(' && data.at(i) != ')')
                    {
                        numString += data.at(i);
                    }
                }
                return vec;
            }
        };
    }
}