#pragma once

#include "Core/Defines.h"
#include "Core/TypeAlias.h"
#include "Core/Logger.h"

#include "Core/Memory.h"

#include "Serialisation/Serialisers/ISerialiser.h"

#include <nlohmann/json.hpp>

#include <string>
#include <sstream>
#include <vector>

#include <cstddef>
#include <type_traits>
#include <assert.h>

namespace Insight
{
    namespace Serialisation
    {
        class ISerialiser;

        /// @brief Empty template struct for the serialiser macros to be used to define SerialiserObjects for different types.
        /// @tparam T 
        template<typename T>
        struct SerialiserObject
        {
            ISerialiser* Serialise(T const& object, SerialisationTypes type)
            {
                assert(false);
                return nullptr;
            }
            ISerialiser* Serialise(T const* object, SerialisationTypes type)
            {
                return Serialise(*object, type); 
            }
        };

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
                        if constexpr (is_uptr<T>::value)
                        {
                            json.push_back(objectSerialiser.SerialiseToJsonObject(*object.at(i).Get()));
                        }
                        else if constexpr (std::is_pointer_v<T>)
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

        template<typename TKey, typename TValue>
        struct UMapSerialiser
        {
            nlohmann::json operator()(std::unordered_map<TKey, TValue> const& object)
            {
                using Type = std::remove_pointer_t<std::remove_reference_t<std::remove_all_extents_t<T>>>;
                constexpr bool c_KeyIsFundemental = std::is_fundamental_v<TKey>;
                constexpr bool c_ValueIsFundemental = std::is_fundamental_v<TValue>;

                if constexpr (ObjectSerialiser)
                {
                    nlohmann::json json;
                    //SerialiserObject<Type> objectSerialiser;
                    //for (size_t i = 0; i < object.size(); ++i)
                    //{
                    //    if constexpr (std::is_pointer_v<T>)
                    //    {
                    //        json.push_back(objectSerialiser.SerialiseToJsonObject(*object.at(i)));
                    //    }
                    //    else
                    //    {
                    //        json.push_back(objectSerialiser.SerialiseToJsonObject(object.at(i)));
                    //    }
                    //}
                    return json;
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