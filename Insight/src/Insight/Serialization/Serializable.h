/*
	Simple Serializable class used to save and load data from a file.
*/

#pragma once

#include "Insight/Core.h"
#include "Insight/Memory/MemoryManager.h"
#include "File/SerializableFile.h"
#include "SerializeHelper.h"
#include <map>

#define REGISTER_DEC_TYPE(NAME) \
	static Insight::Serialization::TypeRegister<NAME> reg

#define REGISTER_DEF_TYPE(NAME) \
    Insight::Serialization::TypeRegister<NAME> NAME::reg(#NAME)

namespace Insight
{
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
				return NEW_ON_HEAP(T); 
			}

			template<typename T>
			static Serializable* CreateInstanceFromType(const std::string& type)
			{
				auto t = SerializableRegistry::GetTypes().find(type);
				if (t != SerializableRegistry::GetTypes().end())
				{
					return t->second();
				}
				return nullptr;
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
}