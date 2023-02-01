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
            bool operator()(std::string const& data)
            {
                return std::stoi(data) == 1 ? true : false;
            }
        };

        template<>
        struct PropertyDeserialiser<char>
        {
            char operator()(std::string const& data)
            {
                return data.front();
            }
        };

        template<>
        struct PropertyDeserialiser<u8>
        {
            u8 operator()(std::string const& data)
            {
                return static_cast<u8>(std::stoul(data));
            }
        };
        template<>
        struct PropertyDeserialiser<u16>
        {
            u16 operator()(std::string const& data)
            {
                return static_cast<u16>(std::stoul(data));
            }
        };
        template<>
        struct PropertyDeserialiser<u32>
        {
            u32 operator()(std::string const& data)
            {
                return static_cast<u32>(std::stoul(data));
            }
        };
        template<>
        struct PropertyDeserialiser<u64>
        {
            u64 operator()(std::string const& data)
            {
                return static_cast<u64>(std::stoull(data));
            }
        };

        template<>
        struct PropertyDeserialiser<i8>
        {
            i8 operator()(std::string const& data)
            {
                return static_cast<i8>(std::stoi(data));
            }
        };
        template<>
        struct PropertyDeserialiser<i16>
        {
            i16 operator()(std::string const& data)
            {
                return static_cast<i16>(std::stoi(data));
            }
        };
        template<>
        struct PropertyDeserialiser<i32>
        {
            i32 operator()(std::string const& data)
            {
                return static_cast<i32>(std::stoi(data));
            }
        };
        template<>
        struct PropertyDeserialiser<i64>
        {
            i64 operator()(std::string const& data)
            {
                return static_cast<i64>(std::stoll(data));
            }
        };

        template<>
        struct PropertyDeserialiser<std::string>
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
        struct PropertyDeserialiser<glm::vec2>
        {
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
            glm::ivec2 operator()(std::string const& data)
            {
                PropertyDeserialiser<int> propertyDeserialiser;
                glm::ivec2 vec;
                int idx = 0;
                std::string numString;
                for (size_t i = 0; i < data.size(); ++i)
                {
                    if (data.at(i) == ',') 
                    {
                        vec[idx] = propertyDeserialiser(numString);
                        numString.clear();
                        ++idx;
                    }
                    else if (data.at(i) >= '0' && data.at(i) <= '9')
                    {
                        numString += data.at(i);
                    }
                }
                vec[idx] = propertyDeserialiser(numString);

                return vec;
            }
        };
        template<>
        struct PropertyDeserialiser<glm::ivec3>
        {
            glm::ivec3 operator()(std::string const& data)
            {
                PropertyDeserialiser<int> propertyDeserialiser;
                glm::ivec3 vec;
                int idx = 0;
                std::string numString;
                for (size_t i = 0; i < data.size(); ++i)
                {
                    if (data.at(i) == ',')
                    {
                        vec[idx] = propertyDeserialiser(numString);
                        numString.clear();
                        ++idx;
                    }
                    else if (data.at(i) >= '0' && data.at(i) <= '9')
                    {
                        numString += data.at(i);
                    }
                }
                vec[idx] = propertyDeserialiser(numString);

                return vec;
            }
        };
        template<>
        struct PropertyDeserialiser<glm::ivec4>
        {
            glm::ivec4 operator()(std::string const& data)
            {
                PropertyDeserialiser<int> propertyDeserialiser;
                glm::ivec4 vec;
                int idx = 0;
                std::string numString;
                for (size_t i = 0; i < data.size(); ++i)
                {
                    if (data.at(i) == ',')
                    {
                        vec[idx] = propertyDeserialiser(numString);
                        numString.clear();
                        ++idx;
                    }
                    else if (data.at(i) >= '0' && data.at(i) <= '9')
                    {
                        numString += data.at(i);
                    }
                }
                vec[idx] = propertyDeserialiser(numString);

                return vec;
            }
        };
    }
}