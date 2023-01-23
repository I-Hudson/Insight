#pragma once

#include "Serialisation/PropertySerialiser.h"
#include "Serialisation/SerialiserBase.h"

#include "Platforms/Platform.h"

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
                return static_cast<u8>(std::stoi(data));
            }
        };
        template<>
        struct PropertyDeserialiser<u16>
        {
            u16 operator()(std::string const& data)
            {
                return static_cast<u16>(std::stoi(data));
            }
        };
        template<>
        struct PropertyDeserialiser<u32>
        {
            u32 operator()(std::string const& data)
            {
                return static_cast<u32>(std::stoi(data));
            }
        };
        template<>
        struct PropertyDeserialiser<u64>
        {
            u64 operator()(std::string const& data)
            {
                return static_cast<u64>(std::stoi(data));
            }
        };

        template<>
        struct PropertyDeserialiser<int>
        {
            int operator()(std::string const& data)
            {
                return static_cast<int>(std::stoi(data));
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
    }
}