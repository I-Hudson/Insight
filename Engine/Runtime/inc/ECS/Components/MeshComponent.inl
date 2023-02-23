#include "MeshComponent.h"

#include "Resource/ResourceManager.h"

namespace Insight
{
    namespace Serialisation
    {
        void ComplexSerialiser<MeshToGuid, Runtime::ResourceLFHandle<Runtime::Mesh>, ECS::MeshComponent>::operator()
            (ISerialiser* serialiser, Runtime::ResourceLFHandle<Runtime::Mesh>& mesh, ECS::MeshComponent* meshComponent) const
        {
			constexpr char* c_MeshGuid = "MeshGuid";
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
    }
}