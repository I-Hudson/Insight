#pragma once

#include "Core/TypeAlias.h"
#include <map>
#include <type_traits>

#undef GetObject

namespace Core
{
	struct DefaultFactoryMapObjectDestructor
	{
		void operator()(void)
		{

		}
	};

	template<typename, typename T>
	struct FactoryMapHasFactoryFunc
	{
		using Type = std::remove_pointer_t<T>;
		static_assert(
			std::integral_constant<Type, false>::value,
			"T must have a 'static T* New()' function used as a factory function."
			);
	};

	// Specialization that does the checking
	template<typename C, typename Ret, typename... Args>
	struct FactoryMapHasFactoryFunc<C, Ret(Args...)>
	{
		using Type = std::remove_reference_t<std::remove_pointer_t<C>>;
	private:
		template<typename T>
		static constexpr auto check(T*) -> typename
			std::is_same< decltype(std::declval<T>().New(std::declval<Args>()...)),
			Ret    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
			>::type;  // attempt to call it and see if the return type is correct

		template<typename>
		static constexpr std::false_type check(...);

		typedef decltype(check<Type>(0)) type;

	public:
		static constexpr bool value = type::value;
	};

	/// <summary>
	/// struct used to get the 'static T New()' function for Type.
	/// </summary>
	/// <typeparam name="T"></typeparam>
	template<typename T>
	struct FactoryMapFuncType
	{
		using Type = std::remove_pointer_t<T>;
		static decltype(&Type::New) get() { return &Type::New; }
	};

	/// <summary>
	/// Manage objects via hash with a custom facroy funciton.
	/// </summary>
	template<typename T, typename FactoryFunc = FactoryMapFuncType<T>,
		typename TReturn = T, typename TDestrcutorStruct = DefaultFactoryMapObjectDestructor>
	class FactoryMap
	{
		using Type = std::remove_reference_t<std::remove_pointer_t<T>>;
	public:
		FactoryMap()
		{
			static_assert(FactoryMapHasFactoryFunc<Type, T()>::value == 1);
		}

		FactoryMap(const FactoryMap& other)
		{
			*this = other;
		}

		FactoryMap(FactoryMap&& other)
		{
			*this = std::move(other);
		}

		~FactoryMap()
		{
			Clear();
		}

		FactoryMap& operator=(const FactoryMap& other)
		{
			m_objects = other.m_objects;
			m_factoryFunc = other.m_factoryFunc;
			m_destrcutorStruct = other.m_destrcutorStruct;
		}

		FactoryMap& operator=(FactoryMap&& other)
		{
			m_objects = other.m_objects;
			m_factoryFunc = other.m_factoryFunc;
			m_destrcutorStruct = other.m_destrcutorStruct;
		}

		TReturn GetObject(u64 hash)
		{
			auto itr = m_objects.find(hash);
			if (itr != m_objects.end())
			{
				return itr->second;
			}

			// Get our static New function.
			auto factoryFunc = FactoryMapFuncType<T>::get();
			T newObject = factoryFunc();
			m_objects[hash] = newObject;
			return newObject;
		}

		void Clear()
		{

		}

	private:
		std::map<u64, T> m_objects;
		//FactoryFunc m_factoryFunc;
		TDestrcutorStruct m_destrcutorStruct;
	};
}