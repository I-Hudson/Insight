#pragma once

#include "Core/Defines.h"
#include "Core/TypeAlias.h"

#include "Core/Memory.h"

#include "Serialisation/Serialisers/ISerialiser.h"
#include "Serialisation/ISerialisable.h"

#include <vector>

namespace Insight
{
    namespace Serialisation
    {
        template<typename>
        struct SerialiserObject;

        enum class SerialiserType
        {
            Property,
            Object,
            Base,
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
            static_assert(std::is_base_of_v<ISerialisable, Type>, "[SerialiseBase] 'Type' does not inherit 'ISerialisable'.");
            data.Serialise(serialiser);
        }
        template<typename Type>
        void SerialiseObject(ISerialiser* serialiser, Type* data)
        {
            SerialiseObject(serialiser, *data);
        }

        template<typename Type>
        void SerialiseBase(ISerialiser* serialiser, Type& data)
        {
            ::Insight::Serialisation::SerialiserObject<Type> objectSerialiser;
            objectSerialiser.Serialise(serialiser, data);
        }

        template<typename TypeSerialiser, typename TValue, SerialiserType SerialiserType>
        void Serialise(ISerialiser* serialiser, std::string_view name, TValue& v)
        {
            if constexpr (is_insight_smart_pointer_v<TValue>)
            {
                if constexpr (SerialiserType == SerialiserType::Property)
                {
                    using TVectorElementType = std::remove_pointer_t<std::decay_t<decltype(v.Get())>>;
                    static_assert(std::is_same_v<TVectorElementType, typename ::Insight::Serialisation::PropertySerialiser<TypeSerialiser>::InType>,
                        "[VectorSerialiser] TVectorElementType is different from TypeSerialiser::InType.");
                    ::Insight::Serialisation::SerialiseProperty<TypeSerialiser, TVectorElementType>(serialiser, name, *v.Get());
                }
                else
                {
                    using TVectorElementType = std::remove_pointer_t<std::decay_t<decltype(v.Get())>>;
                    static_assert(std::is_same_v<TVectorElementType, TypeSerialiser>,
                        "[VectorSerialiser] TVectorElementType is different from TypeSerialiser. Did you mean to use PropertySerialiser?");
                    ::Insight::Serialisation::SerialiseObject<TypeSerialiser>(serialiser, *v.Get());
                }
            }
            else if constexpr (is_stl_smart_pointer_v<TValue>)
            {
                if constexpr (SerialiserType == SerialiserType::Property)
                {
                    using TVectorElementType = std::remove_pointer_t<std::decay_t<decltype(v.get())>>;
                    static_assert(std::is_same_v<TVectorElementType, typename ::Insight::Serialisation::PropertySerialiser<TypeSerialiser>::InType>,
                        "[VectorSerialiser] TVectorElementType is different from TypeSerialiser::InType.");
                    ::Insight::Serialisation::SerialiseProperty<TypeSerialiser, TVectorElementType>(serialiser, name, *v.get());
                }
                else
                {
                    using TVectorElementType = std::remove_pointer_t<std::decay_t<decltype(v.get())>>;
                    static_assert(std::is_same_v<TVectorElementType, TypeSerialiser>,
                        "[VectorSerialiser] TVectorElementType is different from TypeSerialiser. Did you mean to use PropertySerialiser?");
                    ::Insight::Serialisation::SerialiseObject<TypeSerialiser>(serialiser, *v.Get());
                }
            }
            else if constexpr (std::is_pointer_v<TValue>)
            {
                if constexpr (SerialiserType == SerialiserType::Property)
                {
                    using TVectorElementType = std::remove_pointer_t<std::decay_t<decltype(v)>>;
                    static_assert(std::is_same_v<TVectorElementType, typename ::Insight::Serialisation::PropertySerialiser<TypeSerialiser>::InType>,
                        "[VectorSerialiser] TVectorElementType is different from TypeSerialiser::InType.");
                    ::Insight::Serialisation::SerialiseProperty<TypeSerialiser, TVectorElementType>(serialiser, name, *v);
                }
                else
                {
                    using TVectorElementType = std::remove_pointer_t<std::decay_t<decltype(v)>>;
                    static_assert(std::is_same_v<TVectorElementType, TypeSerialiser>,
                        "[VectorSerialiser] TVectorElementType is different from TypeSerialiser. Did you mean to use PropertySerialiser?");
                    ::Insight::Serialisation::SerialiseObject<TypeSerialiser>(serialiser, *v);
                }
            }
            else
            {
                if constexpr (SerialiserType == SerialiserType::Property)
                {
                    ::Insight::Serialisation::SerialiseProperty<TypeSerialiser>(serialiser, name, v);
                }
                else if (SerialiserType == SerialiserType::Object)
                {
                    ::Insight::Serialisation::SerialiseObject<TypeSerialiser>(serialiser, v);
                }
            }
        }

        template<typename TypeSerialiser, typename T>
        T DeserialiseProperty(ISerialiser* serialiser, std::string_view propertyName)
        {
            ::Insight::Serialisation::PropertyDeserialiser<TypeSerialiser> propertyDeserialiser;
            typename ::Insight::Serialisation::PropertyDeserialiser<TypeSerialiser>::InType serialisedData;
            serialiser->Read(propertyName, serialisedData);
            auto data = propertyDeserialiser(serialisedData);
            return static_cast<T>(data);
        }

        template<typename Type>
        void DeserialiseObject(ISerialiser* serialiser, Type& v)
        {
            static_assert(std::is_base_of_v<ISerialisable, Type>, "[SerialiseBase] 'Type' does not inherit 'ISerialisable'.");
            v.Deserialise(serialiser);
        }

        /// @brief Store all known object serialisers with their typenames for look ups with out a type.
        class ObjectSerialiserRegister
        {
            using SerialiserObjectFunc = void(*)(void* objectPtr);
        public:
            static void RegisterObjectSerialiser(const char* typeName, SerialiserObjectFunc serialiserObjectFunc)
            {
                m_serialsierObjectFunc[typeName] = serialiserObjectFunc;
            }
            static void UnregisterObjectSerialiser(const char* typeName)    
            {

            }

            static void SerialsierObjectFromTypeName(std::string_view typeName, void* object);

        private:
            static std::unordered_map<std::string, SerialiserObjectFunc> m_serialsierObjectFunc;
        };

        template<typename T>
        class ObjectSerialiserReg
        {
            ObjectSerialiserReg(const char* typeName)
            {
                ObjectSerialiserRegister::RegisterObjectSerialiser()
            }
            ~ObjectSerialiserReg();
        };

        /// @brief Empty template struct for the serialiser macros to be used to define SerialiserObjects for different types.
        /// @tparam T 
        template<typename T>
        struct SerialiserObject
        {
            void Serialise(ISerialiser* serialiser, T& object)
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

                u64 arraySize = object.size();
                serialiser->StartArray(name, arraySize);
                for (auto& v : object)
                {
                    ::Insight::Serialisation::Serialise<TypeSerialiser, T, SerialiserType>(serialiser, "", v);
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
                    return Ptr<ElementType>(New<ElementType, Core::MemoryAllocCategory::Serialiser>());
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
                serialiser->StartArray(name, arraySize);
                object.resize(arraySize);

                for (auto& v : object)
                {
                    if constexpr (is_insight_smart_pointer_v<T>)
                    {
                        if constexpr (SerialiserType == SerialiserType::Property)
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
                            //::Insight::Serialisation::DeserialiseObject<TypeSerialiser>(serialiser, v);
                            assert(false);
                        }
                    }
                    else if constexpr (is_stl_smart_pointer_v<T>)
                    {
                        if constexpr (SerialiserType == SerialiserType::Property)
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
                        if (v == nullptr)
                        {
                            using TVectorElementType = std::remove_pointer_t<std::decay_t<decltype(*object.begin())>>;
                            v = New<TVectorElementType, Core::MemoryAllocCategory::Serialiser>();
                        }

                        if constexpr (SerialiserType == SerialiserType::Property)
                        {
                            v = std::move(::Insight::Serialisation::DeserialiseProperty<TypeSerialiser, T>(serialiser, name));
                        }
                        else
                        {
                            ::Insight::Serialisation::DeserialiseObject<TypeSerialiser>(serialiser, *v);
                        }
                    }
                    else
                    {
                        if constexpr (SerialiserType == SerialiserType::Property)
                        {
                            v = std::move(::Insight::Serialisation::DeserialiseProperty<TypeSerialiser, T>(serialiser, name));
                        }
                        else if (SerialiserType == SerialiserType::Object)
                        {
                            ::Insight::Serialisation::DeserialiseObject<TypeSerialiser>(serialiser, v);
                        }
                    }
                }
                serialiser->StopArray();
            }
        };

        template<typename T, typename TypeSerialiser, SerialiserType SerialiserType>
        struct ArraySerialiser
        {
            void operator()(ISerialiser* serialiser, std::string_view name, T* object, u64 const size)
            {
                if (!serialiser)
                {
                    return;
                }

                u64 arraySize = size;
                serialiser->StartArray(name, arraySize);
                for (size_t i = 0; i < size; ++i)
                {
                    T* ptr = object + i;
                    ::Insight::Serialisation::Serialise<TypeSerialiser, T, SerialiserType>(serialiser, "", *ptr);
                }
                serialiser->StopArray();
            }
        };

        template<typename T, typename TypeSerialiser, SerialiserType SerialiserType>
        struct ArrayDeserialiser
        {
            void operator()(ISerialiser* serialiser, std::string_view name, T* object, u64 const size)
            {
                if (!serialiser)
                {
                    return;
                }

                u64 arraySize = size;
                serialiser->StartArray(name, arraySize);
                for (size_t i = 0; i < size; ++i)
                {
                    T* ptr = object + i;
                    if constexpr (SerialiserType == SerialiserType::Property)
                    {
                        *ptr = std::move(::Insight::Serialisation::DeserialiseProperty<TypeSerialiser, T>(serialiser, name));
                    }
                    else if (SerialiserType == SerialiserType::Object)
                    {
                        ::Insight::Serialisation::DeserialiseObject<TypeSerialiser>(serialiser, *ptr);
                    }
                }
                serialiser->StopArray();
            }
        };

        template<typename KeySerialiser, typename ValueSerialiser, typename TMap, SerialiserType SerialiserType>
        struct MapSerialiser
        {
            void operator()(ISerialiser* serialiser, std::string_view name, TMap& map)
            {
                if (!serialiser)
                {
                    return;
                }

                u64 arraySize = map.size();
                serialiser->StartArray(name, arraySize);
                for (auto& v : map)
                {
                    using TKeyType = std::remove_const_t<typename std::decay<decltype(v.first)>::type>;
                    using TValueType = typename std::decay<decltype(v.second)>::type;

                    // Key
                    serialiser->StartObject("");
                    //TKeyType const& key = v.first;
                    ::Insight::Serialisation::Serialise<KeySerialiser, TKeyType, SerialiserType>(serialiser, "", const_cast<TKeyType&>(v.first));
                    serialiser->StopObject();

                    // Value
                    serialiser->StartObject("");
                    ::Insight::Serialisation::Serialise<ValueSerialiser, TValueType, SerialiserType>(serialiser, "", v.second);
                    serialiser->StopObject();

                }
                serialiser->StopArray();
            }
        };

        template<typename TypeSerialiser, typename T, typename TObjectType>
        struct ComplexSerialiser
        {
            template<std::enable_if_t<!std::is_same_v<T, void>>>
            void operator()(ISerialiser* serialiser, T& v, TObjectType* object) const
            {
                assert(false);
            }

            template<std::enable_if_t<std::is_same_v<T, void>>>
            void operator()(ISerialiser* serialiser, TObjectType* object) const
            {
                assert(false);
            }
        };

        /// @brief Empty template struct for the serialiser macros to be used to define SerialiserObjects for different types.
        /// @tparam T 
        template<typename T>
        struct DeserialiserObject
        { };
    }
}