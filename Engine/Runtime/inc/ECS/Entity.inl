#include "World/WorldSystem.h"
#include "Serialisation/SerialiserBase.h"

namespace Insight
{

	namespace Serialisation
	{
		std::string PropertySerialiser<EntityToGuid>::operator()(Ptr<InType> const& v) const
		{
			if (!v)
			{
				return {};
			}
			Insight::Serialisation::PropertySerialiser<Core::GUID> guidSerialiser;
			return guidSerialiser(v->GetGUID());
		}
		std::string PropertySerialiser<EntityToGuid>::operator()(InType const& v) const
		{
			Insight::Serialisation::PropertySerialiser<Core::GUID> guidSerialiser;
			return guidSerialiser(v.GetGUID());
		}

		ECS::Entity* PropertyDeserialiser<EntityToGuid>::operator()(InType const& v) const
		{
			if (v.empty())
			{
				return nullptr;
			}
			Insight::Serialisation::PropertyDeserialiser<Core::GUID> guidDeserialiser;
			return Runtime::WorldSystem::Instance().GetActiveWorld()->GetEntityByGUID(guidDeserialiser(v));
		}
	}
}