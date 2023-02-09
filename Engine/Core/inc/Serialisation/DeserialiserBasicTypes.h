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

namespace Insight
{
    namespace Serialisation
    {
        template<>
        struct PropertyDeserialiser<bool, bool>
        {
            using InType = bool;
            bool operator()(InType const data) const
            {
                return data;
            }
        };

        template<>
        struct PropertyDeserialiser<char, char>
        {
            using InType = char;
            char operator()(InType const data) const
            {
                return data;
            }
        };

        template<>
        struct PropertyDeserialiser<u8, u8>
        {
            using InType = u8;
            u8 operator()(InType const data) const
            {
                return data;
            }
        };
        template<>
        struct PropertyDeserialiser<u16, u16>
        {
            using InType = u16;
            u16 operator()(InType const data) const
            {
                return data;
            }
        };
        template<>
        struct PropertyDeserialiser<u32, u32>
        {
            using InType = u32;
            u32 operator()(InType const data) const
            {
                return data;
            }
        };
        template<>
        struct PropertyDeserialiser<u64, u64>
        {
            using InType = u64;
            u64 operator()(InType const data) const
            {
                return data;
            }
        };

        template<>
        struct PropertyDeserialiser<i8, i8>
        {
            using InType = i8;
            i8 operator()(InType const data) const
            {
                return data;
            }
        };
        template<>
        struct PropertyDeserialiser<i16, i16>
        {
            using InType = i16;
            i16 operator()(InType const data) const
            {
                return data;
            }
        };
        template<>
        struct PropertyDeserialiser<i32, i32>
        {
            using InType = i32;
            i32 operator()(InType const data) const
            {
                return data;
            }
        };
        template<>
        struct PropertyDeserialiser<i64, i64>
        {
            using InType = i64;
            i64 operator()(InType const data) const
            {
                return data;
            }
        };

        template<>
        struct PropertyDeserialiser<std::string, std::string>
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
        struct PropertyDeserialiser<glm::vec2, glm::vec2>
        {
            using InType = std::string;
            glm::vec2 operator()(std::string const& data) const
            {
                glm::vec2 vec;
                int idx = 0;
                std::string numString;
                for (size_t i = 0; i < data.size(); ++i)
                {
                    if (data.at(i) == ',')
                    {
                        vec[idx] = std::stof(numString);
                        numString.clear();
                        ++idx;
                    }
                    else if (data.at(i) >= '0' && data.at(i) <= '9')
                    {
                        numString += data.at(i);
                    }
                }
                vec[idx] = std::stof(numString);

                return vec;
            }
        };
        template<>
        struct PropertyDeserialiser<glm::vec3, glm::vec3>
        {
            using InType = std::string;
            glm::vec3 operator()(std::string const& data) const
            {
                glm::vec3 vec;
                int idx = 0;
                std::string numString;
                for (size_t i = 0; i < data.size(); ++i)
                {
                    if (data.at(i) == ',')
                    {
                        vec[idx] = std::stof(numString);
                        numString.clear();
                        ++idx;
                    }
                    else if (data.at(i) >= '0' && data.at(i) <= '9')
                    {
                        numString += data.at(i);
                    }
                }
                vec[idx] = std::stof(numString);

                return vec;
            }
        };
        template<>
        struct PropertyDeserialiser<glm::vec4, glm::vec4>
        {
            using InType = std::string;
            glm::vec4 operator()(std::string const& data) const
            {
                glm::vec4 vec;
                int idx = 0;
                std::string numString;
                for (size_t i = 0; i < data.size(); ++i)
                {
                    if (data.at(i) == ',')
                    {
                        vec[idx] = std::stof(numString);
                        numString.clear();
                        ++idx;
                    }
                    else if (data.at(i) >= '0' && data.at(i) <= '9')
                    {
                        numString += data.at(i);
                    }
                }
                vec[idx] = std::stof(numString);

                return vec;
            }
        };


        template<>
        struct PropertyDeserialiser<glm::ivec2, glm::ivec2>
        {
            using InType = std::string;
            glm::ivec2 operator()(std::string const& data) const
            {
                glm::ivec2 vec;
                int idx = 0;
                std::string numString;
                for (size_t i = 0; i < data.size(); ++i)
                {
                    if (data.at(i) == ',') 
                    {
                        vec[idx] = std::stoi(numString);
                        numString.clear();
                        ++idx;
                    }
                    else if (data.at(i) >= '0' && data.at(i) <= '9')
                    {
                        numString += data.at(i);
                    }
                }
                vec[idx] = std::stoi(numString);

                return vec;
            }
        };
        template<>
        struct PropertyDeserialiser<glm::ivec3, glm::ivec3>
        {
            using InType = std::string;
            glm::ivec3 operator()(std::string const& data) const
            {
                glm::ivec3 vec;
                int idx = 0;
                std::string numString;
                for (size_t i = 0; i < data.size(); ++i)
                {
                    if (data.at(i) == ',')
                    {
                        vec[idx] = std::stoi(numString);
                        numString.clear();
                        ++idx;
                    }
                    else if (data.at(i) >= '0' && data.at(i) <= '9')
                    {
                        numString += data.at(i);
                    }
                }
                vec[idx] = std::stoi(numString);

                return vec;
            }
        };
        template<>
        struct PropertyDeserialiser<glm::ivec4, glm::ivec4>
        {
            using InType = std::string;
            glm::ivec4 operator()(std::string const& data) const
            {
                glm::ivec4 vec;
                int idx = 0;
                std::string numString;
                for (size_t i = 0; i < data.size(); ++i)
                {
                    if (data.at(i) == ',')
                    {
                        vec[idx] = std::stoi(numString);
                        numString.clear();
                        ++idx;
                    }
                    else if (data.at(i) >= '0' && data.at(i) <= '9')
                    {
                        numString += data.at(i);
                    }
                }
                vec[idx] = std::stoi(numString);

                return vec;
            }
        };
    }
}