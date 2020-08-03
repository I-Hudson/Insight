/*
	Simple Serializable class used to save and load data from a file.
*/

#pragma once

#include "Insight/Core.h"
#include "Insight/Memory/MemoryManager.h"
#include <json.hpp>

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

// for convenience
using json = nlohmann::json;

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
				SerializableRegistry::GetTypes().insert(std::pair(s, &Serializable::CreateInstance<T>));;
			}
		};

		struct SerializableRegistry;

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

			virtual void Serialize(json& data) = 0;
			virtual void Deserialize(json data) = 0;

			template<typename T> 
			static Serializable* CreateInstance()
			{ 
				return NEW_ON_HEAP(T); 
			}

			static std::vector<Serializable*> m_serializableObjects;

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

			static void SerializeAll()
			{
				std::ofstream out;
				out.open("serializeData.json");

				std::vector<json> sceneItems;
				for (auto it = Serializable::m_serializableObjects.begin(); it != Serializable::m_serializableObjects.end(); ++it)
				{
					json data;
					(*it)->Serialize(data);
					sceneItems.push_back(data);
				}

				json data;
				for (size_t i = 0; i < sceneItems.size(); ++i)
				{
					data[sceneItems[i]["UUID"].get<std::string>()] = sceneItems[i];
				}
				out << data.dump(4);
				out.close();
			}

			static void DeserializeAll()
			{
				std::ifstream in;
				in.open("serializeData.json");

				json data;
				in >> data;

				//std::vector<std::string> data;
				//bool record = false;
				//
				//std::string line;
				//Serializable* s = nullptr;
				//while (std::getline(in, line))
				//{
				//	if (line == "\\")
				//	{
				//		record = true;
				//	}
				//	else if (line == "//")
				//	{
				//		DELETE_ON_HEAP(s);
				//		record = false;
				//	}
				//	else
				//	{
				//
				//		std::string type = line.substr(0, line.find_first_of(':'));
				//		MapTypes::iterator it = GetTypes().find(type);
				//		if (it != GetTypes().end())
				//		{ 
				//			s = it->second();
				//		}
				//		else
				//		{
				//			data.push_back(line);
				//		}
				//	}
				//}

				in.close();
			}
		};
	}
}