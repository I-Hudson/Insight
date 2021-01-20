#include "ispch.h"
#include "Insight/Serialization/Serializable.h"

	namespace Serialization
	{
		std::vector<Serializable*> Serializable::m_serializableObjects;
		bool SerializableRegistry::m_inUse = false;
		
		Serializable::Serializable(Serializable* obj, bool autoSeralizable, const std::string& filePath)
		{
			if (!autoSeralizable)
			{
				m_serializableObjects.push_back(obj);
			}
			m_fileName = filePath;
		}

		Serializable::~Serializable()
		{
			auto it = std::find(m_serializableObjects.begin(), m_serializableObjects.end(), this);
			if (it != m_serializableObjects.end())
			{
				m_serializableObjects.erase(it);
			}
		}
	}