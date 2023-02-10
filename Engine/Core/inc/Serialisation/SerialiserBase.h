#pragma once

#include "Core/Defines.h"
#include "Core/TypeAlias.h"

#include "Core/Memory.h"

#include "Serialisation/Serialisers/ISerialiser.h"

#include <vector>

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

        template<typename TypeSerialiser, typename T>
        void SerialiseProperty(ISerialiser* serialiser, std::string_view propertyName, T& data)
        {
            ::Insight::Serialisation::PropertySerialiser<TypeSerialiser> propertySerialiser;
            auto SerialisedData = propertySerialiser(data);
            serialiser->Write(propertyName, SerialisedData);
        }

        template<typename Type>
        void SerialiseObject(ISerialiser* serialiser, Type& data)
        {
            // If the object derives from 'ISerialisable' then it might be over-ridden so
            // call the 'Serialise' function in case of this.
            if constexpr (std::is_base_of_v<ISerialisable, Type>)
            {
                data.Serialise(serialiser);
            }
            else
            {
                ::Insight::Serialisation::SerialiserObject<Type> objectSerialiser;
                objectSerialiser.Serialise(data, serialiser);
            }
        }
        template<typename Type>
        void SerialiseObject(ISerialiser* serialiser, Type* data)
        {
            SerialiseObject(serialiser, *data);
        }

        template<typename TypeSerialiser, typename T>
        T DeserialiseProperty(ISerialiser* serialiser, std::string_view propertyName)
        {
            ::Insight::Serialisation::PropertyDeserialiser<TypeSerialiser> propertyDeserialiser;
            ::Insight::Serialisation::PropertyDeserialiser<TypeSerialiser>::InType serialisedData;
            serialiser->Read(propertyName, serialisedData);
            return propertyDeserialiser(serialisedData);
        }

        template<typename Type>
        void DeserialiseObject(ISerialiser* serialiser, Type& v)
        {
            if constexpr (std::is_base_of_v<ISerialisable, Type>)
            {
                v.Deserialise(serialiser);
            }
            else
            {
                ::Insight::Serialisation::SerialiserObject<Type> objectSerialiser;
                objectSerialiser.Deserialise(serialiser, v);
            }
        }

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

        template<typename T, typename TypeSerialiser, SerialiserType SerialiserType>
        struct VectorSerialiser
        {
            void operator()(ISerialiser* serialiser, std::string_view name, std::vector<T>& object)
            {
                if (!serialiser)
                {
                    return;
                }

                serialiser->Write(std::string(name) + c_ArraySize, object.size());
                serialiser->StartArray(name);
                for (auto& v : object)
                {
                    if constexpr (is_insight_smart_pointer_v<T>)
                    {
                        if constexpr (SerialiserType == SerialiserType::VectorProperty)
                        {
                            using TVectorElementType = std::remove_pointer_t<decltype(v.Get())>;
                            static_assert(std::is_same_v<TVectorElementType, ::Insight::Serialisation::PropertySerialiser<TypeSerialiser>::InType>,
                                "[VectorSerialiser] TVectorElementType is different from TypeSerialiser::InType.");
                            ::Insight::Serialisation::SerialiseProperty<TypeSerialiser, TVectorElementType>(serialiser, name, *v.Get());
                        }
                        else
                        {
                            using TVectorElementType = std::remove_pointer_t<decltype(v.Get())>;
                            static_assert(std::is_same_v<TVectorElementType, TypeSerialiser>,
                                "[VectorSerialiser] TVectorElementType is different from TypeSerialiser. Did you mean to use PropertySerialiser?");
                            ::Insight::Serialisation::SerialiseObject<TypeSerialiser>(serialiser, *v.Get());
                        }
                    }
                    else if constexpr (is_stl_smart_pointer_v<T>)
                    {
                        if constexpr (SerialiserType == SerialiserType::VectorProperty)
                        {
                            using TVectorElementType = std::remove_pointer_t<decltype(v.get())>;
                            static_assert(std::is_same_v<TVectorElementType, ::Insight::Serialisation::PropertySerialiser<TypeSerialiser>::InType>,
                                "[VectorSerialiser] TVectorElementType is different from TypeSerialiser::InType.");
                            ::Insight::Serialisation::SerialiseProperty<TypeSerialiser, TVectorElementType>(serialiser, name, *v.get());
                        }
                        else
                        {
                            using TVectorElementType = std::remove_pointer_t<decltype(v.get())>;
                            static_assert(std::is_same_v<TVectorElementType, TypeSerialiser>,
                                "[VectorSerialiser] TVectorElementType is different from TypeSerialiser. Did you mean to use PropertySerialiser?");
                            ::Insight::Serialisation::SerialiseObject<TypeSerialiser>(serialiser, *v.Get());
                        }
                    }
                    else if constexpr (std::is_pointer_v<T>)
                    {
                        if constexpr (SerialiserType == SerialiserType::VectorProperty)
                        {
                            using TVectorElementType = std::remove_pointer_t<decltype(v)>;
                            static_assert(std::is_same_v<TVectorElementType, ::Insight::Serialisation::PropertySerialiser<TypeSerialiser>::InType>,
                                "[VectorSerialiser] TVectorElementType is different from TypeSerialiser::InType.");
                            ::Insight::Serialisation::SerialiseProperty<TypeSerialiser, TVectorElementType>(serialiser, name, *v);
                        }
                        else
                        {
                            using TVectorElementType = std::remove_pointer_t<decltype(v)>;
                            static_assert(std::is_same_v<TVectorElementType, TypeSerialiser>,
                                "[VectorSerialiser] TVectorElementType is different from TypeSerialiser. Did you mean to use PropertySerialiser?");
                            ::Insight::Serialisation::SerialiseObject<TypeSerialiser>(serialiser, *v);
                        }
                    }
                    else
                    {
                        if constexpr (SerialiserType == SerialiserType::VectorProperty)
                        {
                            ::Insight::Serialisation::SerialiseProperty<TypeSerialiser, T>(serialiser, name, v);
                        }
                        else if (SerialiserType == SerialiserType::VectorObject)
                        {
                            ::Insight::Serialisation::SerialiseObject<TypeSerialiser>(serialiser, v);
                        }
                    }
                }
                serialiser->StopArray();
            }
        };

        template<typename T, typename TypeSerialiser, SerialiserType SerialiserType>
        struct VectorDeserialiser
        {
            template<typename ElementType>
            T NewInsightPointer() const
            {
                if constexpr (is_insight_unique_pointer_v<T>)
                {
                    return MakeUPtr<ElementType>();
                }
                else if constexpr (is_insight_shared_pointer_v<T>)
                {
                    return MakeRPtr<ElementType>();
                }
                else if constexpr (is_insight_wrapper_pointer_v<T>)
                {
                    return Ptr<ElementType>(New<ElementType>());
                }
                else
                {
                    assert(false);
                    return T();
                }
            }

            template<typename ElementType>
            T NewSTLPointer() const
            {
                if constexpr (is_stl_unique_pointer_v<T>)
                {
                    return std::make_unique<ElementType>();
                }
                else if constexpr (is_stl_shared_pointer_v<T>)
                {
                    return std::make_shared<ElementType>();
                }
                else
                {
                    assert(false);
                    return T();
                }
            }

            void operator()(ISerialiser* serialiser, std::string_view name, std::vector<T>& object)
            {
                if (!serialiser)
                {
                    return;
                }

                u64 arraySize = 0;
                serialiser->Read(std::string(name) + c_ArraySize, arraySize);
                object.resize(arraySize);

                serialiser->StartArray(name);
                for (auto& v : object)
                {
                    if constexpr (is_insight_smart_pointer_v<T>)
                    {
                        if constexpr (SerialiserType == SerialiserType::VectorProperty)
                        {
                            //using TVectorElementType = std::remove_pointer_t<decltype(v.Get())>;
                            //T newPointer = NewInsightPointer<TVectorElementType>();
                            //v = newPointer;
                            v = std::move(::Insight::Serialisation::DeserialiseProperty<TypeSerialiser, T>(serialiser, name));
                        }
                        else
                        {
                            //using TVectorElementType = std::remove_pointer_t<decltype(v.Get())>;
                            //T newPointer = NewInsightPointer<TVectorElementType>();
                            //v = newPointer;
                            ::Insight::Serialisation::DeserialiseObject<TypeSerialiser>(serialiser, v);
                        }
                    }
                    else if constexpr (is_stl_smart_pointer_v<T>)
                    {
                        if constexpr (SerialiserType == SerialiserType::VectorProperty)
                        {
                            //using TVectorElementType = std::remove_pointer_t<decltype(v.get())>;
                            //T newPointer = NewSTLPointer<TVectorElementType>();
                            //v = newPointer;
                            v = std::move(::Insight::Serialisation::DeserialiseProperty<TypeSerialiser, T>(serialiser, name));
                        }
                        else
                        {
                            //using TVectorElementType = std::remove_pointer_t<decltype(v.get())>;
                            //T newPointer = NewSTLPointer<TVectorElementType>();
                            //v = newPointer;
                            ::Insight::Serialisation::DeserialiseObject<TypeSerialiser>(serialiser, v);

                        }
                    }
                    else if constexpr (std::is_pointer_v<T>)
                    {
                        if constexpr (SerialiserType == SerialiserType::VectorProperty)
                        {
                            v = std::move(::Insight::Serialisation::DeserialiseProperty<TypeSerialiser, T>(serialiser, name));
                        }
                        else
                        {
                            ::Insight::Serialisation::DeserialiseObject<TypeSerialiser>(serialiser, v);
                        }
                    }
                    else
                    {
                        if constexpr (SerialiserType == SerialiserType::VectorProperty)
                        {
                            v = std::move(::Insight::Serialisation::DeserialiseProperty<TypeSerialiser, T>(serialiser, name));
                        }
                        else if (SerialiserType == SerialiserType::VectorObject)
                        {
                            ::Insight::Serialisation::DeserialiseObject<TypeSerialiser>(serialiser, v);
                        }
                    }
                }
                serialiser->StopArray();
            }
        };

        template<typename TypeSerialiser, typename T, typename TObjectType>
        struct ComplexSerialiser
        {
            void operator()(T const& v, TObjectType* object, ISerialiser* serialiser) const
            {
                assert(false);
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
        /*template<typename T, bool ObjectDesialiser>
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
        };*/
    }
}