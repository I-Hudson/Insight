/*
	Simple Serializable class used to save and load data from a file.
*/

#pragma once

#include "Insight/Core.h"
#include "Insight/Memory/MemoryManager.h"

#include <ostream>
#include <istream>
#include <map>

#define REGISTER_DEC_TYPE(NAME) \
	static Insight::Serialization::TypeRegister<NAME> reg

#define REGISTER_DEF_TYPE(NAME) \
    Insight::Serialization::TypeRegister<NAME> NAME::reg(#NAME)

#define START_SER << "\\" << '\n'
#define END_SER << "//" << '\n'
#define ENDL << '\n'

namespace Insight
{
	namespace Serialization
	{
		struct SerializeData
		{
			std::string SerializeString;

			void AddData(const std::string& type, const std::string& data)
			{

			}
		};

		template<typename T>
		struct TypeRegister
		{
			TypeRegister(const std::string& s)
			{
				SerializableRegistry::GetTypes().insert(std::pair(s, &Serializable::CreateInstance<T>));
			}
		};

		class Serializable
		{
		public:

			Serializable(Serializable* obj, bool isSubObject)
			{
				if (!isSubObject)
				{
					m_serializableObjects.push_back(obj);
				}
			}

			virtual ~Serializable()
			{
				auto it = std::find(m_serializableObjects.begin(), m_serializableObjects.end(), this);
				if (it != m_serializableObjects.end())
				{
					m_serializableObjects.erase(it);
				}
			}

			virtual void Serialize(std::ostream& out) = 0;
			virtual void Deserialize(std::istream& in) = 0;

			template<typename T> 
			static Serializable* CreateInstance()
			{ 
				return NEW_ON_HEAP(T); 
			}

			static std::vector<Serializable*> m_serializableObjects;
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

			static void SerializeAll()
			{
				std::ofstream out;
				out.open("serializeData.txt");

				for (auto it = Serializable::m_serializableObjects.begin(); it != Serializable::m_serializableObjects.end(); ++it)
				{
					out START_SER;
					(*it)->Serialize(out);
					out END_SER;
				}
				out.close();
			}

			static void DeserializeAll()
			{
				std::ifstream in;
				in.open("serializeData.txt");

				std::vector<std::string> data;
				bool record = false;

				std::string line;
				while (std::getline(in, line))
				{
					if (line == "\\")
					{
						record = true;
					}
					else if (line == "//")
					{

						record = false;
					}
					else
					{
						data.push_back(line);
					}
				}

				in.close();
			}
		};
	}
}