#pragma once

#include "Serialisation/PropertySerialiser.h"
#include "Serialisation/SerialiserBase.h"

#include "Platforms/Platform.h"

#include <glm/glm.hpp>

namespace Insight
{
    namespace Serialisation
    {
        template<>
        struct PropertyDeserialiser<bool>
        {
            using InType = bool;
            using OutType = bool;
            bool operator()(bool const data)
            {
                return data;
            }
        };

        template<>
        struct PropertyDeserialiser<char>
        {
            using InType = std::string;
            using OutType = char;
            char operator()(char const data)
            {
                return data;
            }
        };

        template<>
        struct PropertyDeserialiser<u8>
        {
            using InType = u8;
            using OutType = u8;
            u8 operator()(u8 const data)
            {
                return data;
            }
        };
        template<>
        struct PropertyDeserialiser<u16>
        {
            using InType = u16;
            using OutType = u16;
            u16 operator()(u16 const data)
            {
                return data;
            }
        };
        template<>
        struct PropertyDeserialiser<u32>
        {
            using InType = u32;
            using OutType = u32;
            u32 operator()(u32 const data)
            {
                return data;
            }
        };
        template<>
        struct PropertyDeserialiser<u64>
        {
            using InType = u64;
            using OutType = u64;
            u64 operator()(u64 const data)
            {
                return data;
            }
        };

        template<>
        struct PropertyDeserialiser<i8>
        {
            using InType = i8;
            using OutType = i8;
            i8 operator()(i8 const data)
            {
                return data;
            }
        };
        template<>
        struct PropertyDeserialiser<i16>
        {
            using InType = i16;
            using OutType = i16;
            i16 operator()(i16 const data)
            {
                return data;
            }
        };
        template<>
        struct PropertyDeserialiser<i32>
        {
            using InType = i32;
            using OutType = i32;
            i32 operator()(i32 const data)
            {
                return data;
            }
        };
        template<>
        struct PropertyDeserialiser<i64>
        {
            using InType = i64;
            using OutType = i64;
            i64 operator()(i64 const data)
            {
                return data;
            }
        };

        template<>
        struct PropertyDeserialiser<std::string>
        {
            using InType = std::string;
            using OutType = std::string;
            std::string operator()(std::string const& data)
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
            using OutType = glm::vec2;
            glm::vec2 operator()(std::string const& data)
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
        struct PropertyDeserialiser<glm::vec3>
        {
            using InType = std::string;
            using OutType = glm::vec3;
            glm::vec3 operator()(std::string const& data)
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
        struct PropertyDeserialiser<glm::vec4>
        {
            using InType = std::string;
            using OutType = glm::vec4;
            glm::vec4 operator()(std::string const& data)
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
        struct PropertyDeserialiser<glm::ivec2>
        {
            using InType = std::string;
            using OutType = glm::ivec2;
            glm::ivec2 operator()(std::string const& data)
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
        struct PropertyDeserialiser<glm::ivec3>
        {
            using InType = std::string;
            using OutType = glm::ivec3;
            glm::ivec3 operator()(std::string const& data)
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
        struct PropertyDeserialiser<glm::ivec4>
        {
            using InType = std::string;
            using OutType = glm::ivec4;
            glm::ivec4 operator()(std::string const& data)
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