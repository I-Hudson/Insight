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

        enum class SerialiserType
        {
            Property,
            Object,
            Base,
            VectorProperty,
            VectorObject,
            UMapProperty,
            UMapObject,
        };

        /// @brief Empty template struct for the serialiser macros to be used to define SerialiserObjects for different types.
        /// @tparam T 
        template<typename T>
        struct SerialiserObject
        {
            void Serialise(T& object, ISerialiser* serialiser)
            {
                assert(false);
            }
            void Serialise(T* object, ISerialiser* serialiser)
            {
                assert(false);
            }
        };

        template<typename TVector, typename TypeSerialiser, SerialiserType SerialiserType>
        struct VectorSerialiser
        {
            void operator()(ISerialiser* serialiser, std::string_view name, std::vector<TVector>& object)
            {
                using Type = std::remove_pointer_t<std::remove_reference_t<std::remove_all_extents_t<TVector>>>;
                if (!serialiser)
                {
                    return;
                }

                serialiser->StartArray(name);
                for (auto& v : object)
                {
                    if constexpr (SerialiserType == SerialiserType::VectorProperty)
                    {
                        ::Insight::Serialisation::PropertySerialiser<TypeSerialiser> propertySerialiser;
                        auto SerialisedData = propertySerialiser(v);
                        serialiser->Write("", SerialisedData);
                    }
                    else if (SerialiserType == SerialiserType::VectorObject)
                    {
                        ::Insight::Serialisation::SerialiserObject<Type> serialiserObject;
                        serialiserObject.Serialise(v, serialiser);
                    }
                }
                serialiser->StopArray();
            }

            void operator()(ISerialiser* serialiser, std::string_view name, std::vector<TVector*>& object)
            {
                using Type = std::remove_pointer_t<std::remove_reference_t<std::remove_all_extents_t<TVector>>>;
                if (!serialiser)
                {
                    return;
                }

                serialiser->StartArray(name);
                for (auto& v : object)
                {
                    if constexpr (SerialiserType == SerialiserType::VectorProperty)
                    {
                        ::Insight::Serialisation::PropertySerialiser<TypeSerialiser> propertySerialiser;
                        auto SerialisedData = propertySerialiser(*v);
                        serialiser->Write("", SerialisedData);
                    }
                    else if (SerialiserType == SerialiserType::VectorObject)
                    {
                        ::Insight::Serialisation::SerialiserObject<Type> serialiserObject;
                        serialiserObject.Serialise(v, serialiser);
                    }
                }
                serialiser->StopArray();
            }

            //=============================================
            // Insight smart pointers
            //=============================================
            void operator()(ISerialiser* serialiser, std::string_view name, std::vector<UPtr<TVector>>& object)
            {
                std::vector<TVector*> rawPoniters;
                rawPoniters.reserve(object.size());
                for (auto& ptr : object)
                {
                    rawPoniters.push_back(ptr.Get());
                }
                operator()(serialiser, name, rawPoniters);
            }
            void operator()(ISerialiser* serialiser, std::string_view name, std::vector<RPtr<TVector>>& object)
            {
                std::vector<TVector*> rawPoniters;
                rawPoniters.reserve(object.size());
                for (auto& ptr : object)
                {
                    rawPoniters.push_back(ptr.Get());
                }
                operator()(serialiser, name, rawPoniters);
            }
            void operator()(ISerialiser* serialiser, std::string_view name, std::vector<WPtr<TVector>>& object)
            {
                std::vector<TVector*> rawPoniters;
                rawPoniters.reserve(object.size());
                for (auto& ptr : object)
                {
                    rawPoniters.push_back(ptr.Get());
                }
                operator()(serialiser, name, rawPoniters);
            }

            //=============================================
            // STL smart pointers
            //=============================================
            void operator()(ISerialiser* serialiser, std::string_view name, std::vector<std::unique_ptr<TVector>>& object)
            {
                std::vector<TVector*> rawPoniters;
                rawPoniters.reserve(object.size());
                for (auto& ptr : object)
                {
                    rawPoniters.push_back(ptr.get());
                }
                operator()(serialiser, name, rawPoniters);
            }
            void operator()(ISerialiser* serialiser, std::string_view name, std::vector<std::shared_ptr<TVector>>& object)
            {
                std::vector<TVector*> rawPoniters;
                rawPoniters.reserve(object.size());
                for (auto& ptr : object)
                {
                    rawPoniters.push_back(ptr.get());
                }
                operator()(serialiser, name, rawPoniters);
            }
            void operator()(ISerialiser* serialiser, std::string_view name, std::vector<std::weak_ptr<TVector>>& object)
            {
                std::vector<TVector*> rawPoniters;
                rawPoniters.reserve(object.size());
                for (auto& ptr : object)
                {
                    if (std::shared_ptr<T> sPtr = ptr.lock())
                    {
                        rawPoniters.push_back(sPtr.get());
                    }
                }
                operator()(serialiser, name, rawPoniters);
            }
        };

        //template<typename TKey, typename TValue>
        //struct UMapSerialiser
        //{
        //    nlohmann::json operator()(std::unordered_map<TKey, TValue> const& object)
        //    {
        //        using Type = std::remove_pointer_t<std::remove_reference_t<std::remove_all_extents_t<T>>>;
        //        constexpr bool c_KeyIsFundemental = std::is_fundamental_v<TKey>;
        //        constexpr bool c_ValueIsFundemental = std::is_fundamental_v<TValue>;

        //        if constexpr (ObjectSerialiser)
        //        {
        //            nlohmann::json json;
        //            //SerialiserObject<Type> objectSerialiser;
        //            //for (size_t i = 0; i < object.size(); ++i)
        //            //{
        //            //    if constexpr (std::is_pointer_v<T>)
        //            //    {
        //            //        json.push_back(objectSerialiser.SerialiseToJsonObject(*object.at(i)));
        //            //    }
        //            //    else
        //            //    {
        //            //        json.push_back(objectSerialiser.SerialiseToJsonObject(object.at(i)));
        //            //    }
        //            //}
        //            return json;
        //        }
        //    }
        //};

        //template<typename TKey, typename TValue>
        //struct UMapSerialiser
        //{
        //    nlohmann::json operator()(std::unordered_map<TKey, TValue> const& object)
        //    {
        //        using Type = std::remove_pointer_t<std::remove_reference_t<std::remove_all_extents_t<T>>>;
        //        constexpr bool c_KeyIsFundemental = std::is_fundamental_v<TKey>;
        //        constexpr bool c_ValueIsFundemental = std::is_fundamental_v<TValue>;

        //        if constexpr (ObjectSerialiser)
        //        {
        //            nlohmann::json json;
        //            //SerialiserObject<Type> objectSerialiser;
        //            //for (size_t i = 0; i < object.size(); ++i)
        //            //{
        //            //    if constexpr (std::is_pointer_v<T>)
        //            //    {
        //            //        json.push_back(objectSerialiser.SerialiseToJsonObject(*object.at(i)));
        //            //    }
        //            //    else
        //            //    {
        //            //        json.push_back(objectSerialiser.SerialiseToJsonObject(object.at(i)));
        //            //    }
        //            //}
        //            return json;
        //        }
        //    }
        //};

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