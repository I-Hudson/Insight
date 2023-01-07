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

	template <typename T>
	struct FactoryMapHasFactoryFunc
	{
	private:
		typedef std::true_type yes;
		typedef std::false_type no;

		template<typename U> static auto test(int) -> decltype(std::declval<U>().New() == 1, yes());
		template<typename> static no test(...);

	public:
		static constexpr bool value = std::is_same<decltype(test<T>(0)), yes>::value;
	};

	//// <summary>
	//// struct used to get the 'static T New()' function for Type.
	//// </summary>
	//// <typeparam name="T"></typeparam>
	template<typename T>
	struct FactoryMapFuncType
	{
		using Type = std::remove_pointer_t<T>;

		static decltype(&Type::New) get() { return &Type::New; } /// Class is missing a 'static T* New()' function.
	};

	//// <summary>
	//// Manage objects via hash with a custom facroy funciton.
	//// </summary>
	template<typename THash, typename T, typename TDestrcutorStruct = DefaultFactoryMapObjectDestructor>
	class FactoryMap
	{
		using Type = std::remove_reference_t<std::remove_pointer_t<T>>;
	public:
		FactoryMap()
		{
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
			//m_factoryFunc = other.m_factoryFunc;
			m_destrcutorStruct = other.m_destrcutorStruct;
		}

		FactoryMap& operator=(FactoryMap&& other)
		{
			m_objects = other.m_objects;
			//m_factoryFunc = other.m_factoryFunc;
			m_destrcutorStruct = other.m_destrcutorStruct;
		}

		T GetObject(THash hash = { })
		{
			auto itr = m_objects.find(hash);
			if (itr != m_objects.end())
			{
				return itr->second;
			}

			/// Get our static New function.
			auto factoryFunc = FactoryMapFuncType<Type>::get();
			T newObject = factoryFunc();
			m_objects[hash] = newObject;
			return m_objects[hash];
		}

		void Clear()
		{

		}

	private:
		std::map<THash, T> m_objects;
		///FactoryFunc m_factoryFunc;
		TDestrcutorStruct m_destrcutorStruct;
	};
}