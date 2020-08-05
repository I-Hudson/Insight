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

			virtual void Serialize(json& data, bool force = false) = 0;
			virtual void Deserialize(json data, bool force = false) = 0;

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

			static void SerializeAll()
			{
				if (!m_inUse)
				{
					m_inUse = true;
					std::ofstream out;
					out.open("serializeData.json");

					std::vector<json> sceneItems;
					int i = 0;
					for (auto it = Serializable::m_serializableObjects.begin(); it != Serializable::m_serializableObjects.end(); ++it)
					{
						json data;
						if ((*it)->m_fileName.empty())
						{
							(*it)->Serialize(data);
							if (!data.empty())
							{
								sceneItems.push_back(data);
							}
						}
						else
						{
							std::ofstream o;
							o.open((*it)->m_fileName);
							if (o.is_open())
							{
								json data;
								(*it)->Serialize(data);
								o << data.dump(4);
								o.close();
							}
						}
						i++;
					}

					json data;
					for (size_t i = 0; i < sceneItems.size(); ++i)
					{
						data[sceneItems[i]["UUID"].get<std::string>()] = sceneItems[i];
					}
					out << data.dump(4);
					out.close();
					m_inUse = false;
				}
			}

			static void DeserializeAll()
			{
				if (!m_inUse)
				{
					m_inUse = true;
					std::ifstream in;
					in.open("serializeData.json");

					json data;
					in >> data;

					for (auto it = data.begin(); it != data.end(); ++it)
					{
						std::string type = (*it)["Type"];
						Serializable* s = GetTypes().find(type)->second();
						s->Deserialize(*it);
					}
					in.close();
					m_inUse = false;
				}
			}

		private:
			static bool m_inUse;
		};
	}
}