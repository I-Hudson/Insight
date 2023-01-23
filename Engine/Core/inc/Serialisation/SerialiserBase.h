#pragma once

#include "Core/Defines.h"
#include "Core/TypeAlias.h"
#include "Core/Logger.h"

#include "Core/Memory.h"

#include <nlohmann/json.hpp>

#include <string>
#include <sstream>
#include <vector>

#include <cstddef>
#include <type_traits>

namespace Insight
{
    namespace Serialisation
    {
        /// @brief Empty template struct for the serialiser macros to be used to define SerialiserObjects for different types.
        /// @tparam T 
        template<typename T>
        struct SerialiserObject
        { };

        template<typename T, bool ObjectSerialiser>
        struct VectorSerialiser
        {
            auto operator()(std::vector<T> const& object)
            {
                using Type = std::remove_pointer_t<std::remove_reference_t<std::remove_all_extents_t<T>>>;
                if constexpr (ObjectSerialiser)
                {
                    nlohmann::json json;
                    SerialiserObject<Type> objectSerialiser;
                    for (size_t i = 0; i < object.size(); ++i)
                    {
                        if constexpr (std::is_pointer_v<T>)
                        {
                            json.push_back(objectSerialiser.SerialiseToJsonObject(*object.at(i)));
                        }
                        else
                        {
                            json.push_back(objectSerialiser.SerialiseToJsonObject(object.at(i)));
                        }
                    }
                    return json;
                }
                else
                {
                    std::vector<std::string> strings;
                    PropertySerialiser<Type> serialiserProperty;
                    for (size_t i = 0; i < object.size(); ++i)
                    {
                        if constexpr (std::is_pointer_v<T>)
                        {
                            strings.push_back(serialiserProperty(*object.at(i)));
                        }
                        else
                        {
                            strings.push_back(serialiserProperty(object.at(i)));
                        }
                    }
                    return strings;
                }
            }
        };

        /// @brief Empty template struct for the serialiser macros to be used to define SerialiserObjects for different types.
        /// @tparam T 
        template<typename T>
        struct DeserialiserObject
        { };

        /// @brief Deserialise a vector.
        /// @tparam T 
        /// @tparam ObjectDesialiser 
        template<typename T, bool ObjectDesialiser>
        struct VectorDeserialiser
        {
            std::vector<T> operator()(std::vector<nlohmann::json> const& data)
            {
                using Type = std::remove_pointer_t<std::remove_reference_t<std::remove_all_extents_t<T>>>;
                if constexpr (ObjectDesialiser)
                {
                    DeserialiserObject<Type> objectDeserialiser;
                    std::vector<T> vector;
                    for (auto const& v : data)
                    {
                        if constexpr (std::is_pointer_v<T>)
                        {
                            Type* typePtr = New<Type>();
                            objectDeserialiser.DeserialiseToJsonObject(v, typePtr);
                            vector.push_back(typePtr);
                        }
                        else
                        {
                            Type obj;
                            objectDeserialiser.DeserialiseToJsonObject(v, &obj)
                            vector.push_back(obj);
                        }
                    }
                    return vector;
                }
                else
                {
                    PropertyDeserialiser<Type> propertyDeserialiser;
                    std::vector<T> vector;
                    for (auto const& v : data)
                    {
                        if constexpr (std::is_pointer_v<T>)
                        {
                            Type* typePtr = New<Type>();
                            *typePtr = propertyDeserialiser(v);
                            vector.push_back(typePtr);
                        }
                        else
                        {
                            vector.push_back(propertyDeserialiser(v));
                        }
                    }
                    return vector;
                }
            }
        };
    }
}