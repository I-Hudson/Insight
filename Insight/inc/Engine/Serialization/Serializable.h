/*
	Simple Serializable class used to save and load data from a file.
*/

#pragma once

#include "File/SerializableFile.h"
#include <map>

#define REGISTER_DEC_TYPE(NAME) \
	static Serialization::TypeRegister<NAME> reg

#define REGISTER_DEF_TYPE(NAME) \
    Serialization::TypeRegister<NAME> NAME::reg(#NAME)

namespace Serialization
{
	template<typename T>
	struct TypeRegister
	{
		TypeRegister(const std::string& s)
		{
			SerializableRegistry::GetTypes().insert(std::pair(s, &Serializable::CreateInstance<T>));;
		}
	};

	struct SerializableRegistry;

	class Serializable
	{
	public:

		Serializable(Serializable* obj, bool isSubObject, const std::string& filePath = "");
		virtual ~Serializable();

		virtual void Serialize(SerializableElement* element, bool force = false) { }
		virtual void Deserialize(SerializableElement* element, bool force = false) { }

		template<typename T>
		static Serializable* CreateInstance()
		{
			//return ::New<T>(); 
			return nullptr;
		}

		template<typename T>
		static T* NewInstanceFromType(const std::string& type)
		{
			auto t = SerializableRegistry::GetTypes().find(type);
			if (t != SerializableRegistry::GetTypes().end())
			{
				T* tValue = dynamic_cast<T*>(t->second());
				if (tValue)
				{
					return tValue;
				}

				IS_CORE_ERROR("[Serializable::CreateInstanceFromType] Could not cast to T.");
			}
			return {};
		}

		static std::vector<Serializable*> m_serializableObjects;

	protected:
		std::string m_fileName;

	private:
		friend SerializableRegistry;
	};

	struct SerializableRegistry
	{
	public:
		typedef std::map<std::string, Serializable* (*)()> MapTypes;

		static MapTypes& GetTypes()
		{
			static MapTypes types;
			return types;
		}

	private:
		static bool m_inUse;
	};
}