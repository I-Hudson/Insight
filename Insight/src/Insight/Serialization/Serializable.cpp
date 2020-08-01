#include "ispch.h"

#include "Insight/Serialization/Serializable.h"

namespace Insight
{
	namespace Serialization
	{
		std::vector<Serializable*> Serializable::m_serializableObjects;
	}
}