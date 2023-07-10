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
			return Runtime::WorldSystem::Instance().GetEntityByGUID(guidDeserialiser(v));
		}

		std::string PropertySerialiser<ComponentToGuid>::operator()(Ptr<InType> const& v) const
		{
			if (!v)
			{
				return {};
			}
			std::string data;
			Insight::Serialisation::PropertySerialiser<Core::GUID> guidSerialiser;
			data += guidSerialiser(v->GetOwnerEntity()->GetGUID());
			data += "::";
			data += guidSerialiser(v->GetGuid());
			return data;
		}
		std::string PropertySerialiser<ComponentToGuid>::operator()(InType const& v) const
		{
			std::string data;
			Insight::Serialisation::PropertySerialiser<Core::GUID> guidSerialiser;
			data += guidSerialiser(v.GetOwnerEntity()->GetGUID());
			data += "::";
			data += guidSerialiser(v.GetGuid());
			return data;
		}
		ECS::Component* PropertyDeserialiser<ComponentToGuid>::operator()(InType const& v) const
		{
			if (v.empty())
			{
				return nullptr;
			}
			std::string entityGuidString = v.substr(0, v.find("::"));
			std::string componentGuidString = v.substr(v.find("::") + 2);

			Core::GUID entityGuid;
			entityGuid.StringToGuid(entityGuidString);
			Core::GUID componentGuid;
			componentGuid.StringToGuid(componentGuidString);
			
			ECS::Entity* entity = Runtime::WorldSystem::Instance().GetEntityByGUID(entityGuid);
			if (entity == nullptr)
			{
				return nullptr;
			}
			ECS::Component* component = entity->GetComponentByGuid(componentGuid);
			if (component == nullptr)
			{
				return nullptr;
			}
			return component;
		}
	}
}