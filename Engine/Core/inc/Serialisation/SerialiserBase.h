#pragma once

#include "Core/Defines.h"
#include "Core/TypeAlias.h"
#include "Core/Logger.h"

#include <nlohmann/json.hpp>

#include <string>
#include <sstream>
#include <vector>

#include <cstddef>

namespace Insight
{
    namespace Serialisation
    {
        /// @brief Empty template struct for the serialiser macros to be used to define SerialiserObjects for different types.
        /// @tparam T 
        template<typename T>
        struct SerialiserObject
        { };

        template<typename T>
        struct VectorSerialiser
        {
            std::vector<std::string> operator()(std::vector<T> const& object)
            {
                PropertySerialiser<T> serialiserProperty;
                std::vector<std::string> strings;
                for (size_t i = 0; i < object.size(); ++i)
                {
                    strings.push_back(serialiserProperty(object.at(i)));
                }
                return strings;
            }
        };

        /// @brief Empty template struct for the serialiser macros to be used to define SerialiserObjects for different types.
        /// @tparam T 
        template<typename T>
        struct DeserialiserObject
        { };

        template<typename T>
        struct VectorDeserialiser
        {
            std::vector<T> operator()(std::vector<std::string> const& data)
            {
                PropertyDeserialiser<T> propertyDeserialiser;
                std::vector<T> vector;
                for (auto const& v : data)
                {
                    vector.push_back(propertyDeserialiser(v));
                }
                return vector;
            }
        };
    }
}