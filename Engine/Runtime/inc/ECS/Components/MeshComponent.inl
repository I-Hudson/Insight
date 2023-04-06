#include "MeshComponent.h"

#include "Resource/ResourceManager.h"

namespace Insight
{
    namespace Serialisation
    {
        void ComplexSerialiser<MeshToGuid, Runtime::ResourceLFHandle<Runtime::Mesh>, ECS::MeshComponent>::operator()
            (ISerialiser* serialiser, Runtime::ResourceLFHandle<Runtime::Mesh>& mesh, ECS::MeshComponent* meshComponent) const
        {
			constexpr const char* c_MeshGuid = "MeshGuid";
			if (serialiser->IsReadMode())
			{
				PropertyDeserialiser<Core::GUID> guidDeserialiser;
				std::string serialisedGuid;
				serialiser->Read(c_MeshGuid, serialisedGuid);
				if (!serialisedGuid.empty())
				{
					Core::GUID guid = guidDeserialiser(serialisedGuid);
					// Load resource.
					Runtime::IResource* meshResource = Runtime::ResourceManager::GetResourceFromGuid(guid);
					mesh = static_cast<Runtime::Mesh*>(meshResource);
				}
			}
			else
			{
				PropertySerialiser<Core::GUID> guidSerialiser;
				serialiser->Write(c_MeshGuid, mesh ? guidSerialiser(mesh->GetGuid()) : "");
			}
        }

		void ComplexSerialiser<MaterialToGuid, Runtime::ResourceLFHandle<Runtime::Material>, ECS::MeshComponent>::operator()
			(ISerialiser* serialiser, Runtime::ResourceLFHandle<Runtime::Material>& material, ECS::MeshComponent* meshComponent) const
		{
			constexpr const char* c_MaterialGuid = "MaterialGuid";
			if (serialiser->IsReadMode())
			{
				PropertyDeserialiser<Core::GUID> guidDeserialiser;
				std::string serialisedGuid;
				serialiser->Read(c_MaterialGuid, serialisedGuid);
				if (!serialisedGuid.empty())
				{
					Core::GUID guid = guidDeserialiser(serialisedGuid);
					// Load resource.
					Runtime::IResource* meshResource = Runtime::ResourceManager::GetResourceFromGuid(guid);
					material = static_cast<Runtime::Material*>(meshResource);
				}
			}
			else
			{
				PropertySerialiser<Core::GUID> guidSerialiser;
				serialiser->Write(c_MaterialGuid, material ? guidSerialiser(material->GetGuid()) : "");
			}
		}
    }
}