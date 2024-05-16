#include "MeshComponent.h"

#include "Core/Logger.h"

#include "Asset/AssetRegistry.h"
#include "Asset/Assets/Model.h"

#include "Asset/AssetInfo.h"

namespace Insight
{
    namespace Serialisation
    {
        void ComplexSerialiser<MeshToGuid, Runtime::Mesh*, ECS::MeshComponent>::operator()
            (ISerialiser* serialiser, Runtime::Mesh*& mesh, ECS::MeshComponent* meshComponent) const
        {
			constexpr const char* c_AssetGuid = "AssetGuid";
			constexpr const char* c_MeshGuid = "MeshNameHash";
			if (serialiser->IsReadMode())
			{
				PropertyDeserialiser<Core::GUID> guidDeserialiser;
				std::string serialisedGuid;
				std::string meshName;
				serialiser->Read(c_AssetGuid, serialisedGuid);
				serialiser->Read(c_MeshGuid, meshName);

				if (!serialisedGuid.empty())
				{
					Core::GUID assetGuid = guidDeserialiser(serialisedGuid);
					// Load resource.
					const Ref<Runtime::ModelAsset> model = Runtime::AssetRegistry::Instance().LoadAsset(assetGuid).As<Runtime::ModelAsset>();
					if (model)
					{
						for (u32 meshIdx = 0; meshIdx < model->GetMeshCount(); ++meshIdx)
						{
							Runtime::Mesh* assetResourceMesh = model->GetMeshByIndex(meshIdx);
							if (assetResourceMesh->GetName() == meshName)
							{
								mesh = assetResourceMesh;
								break;
							}
						}
					}
					else
					{
						IS_LOG_CORE_ERROR("[ComplexSerialiser<MaterialToGuid>] Unable to load model with Guid '{}'.", assetGuid.ToString());
					}
				}
			}
			else
			{
				PropertySerialiser<Core::GUID> guidSerialiser;
				serialiser->Write(c_AssetGuid, mesh ? guidSerialiser(mesh->GetAssetInfo()->Guid) : "");
				serialiser->Write(c_MeshGuid, mesh ? mesh->GetName().data() : "");
			}
        }

		void ComplexSerialiser<MaterialToGuid, Ref<Runtime::MaterialAsset>, ECS::MeshComponent>::operator()
			(ISerialiser* serialiser, Ref<Runtime::MaterialAsset>& material, ECS::MeshComponent* meshComponent) const
		{
			constexpr const char* c_AssetGuid = "AssetGuid";
			constexpr const char* c_MaterialGuid = "MaterialGuid";
			if (serialiser->IsReadMode())
			{
				PropertyDeserialiser<Core::GUID> guidDeserialiser;
				std::string serialisedGuid;
				std::string mateiralName;
				serialiser->Read(c_AssetGuid, serialisedGuid);
				serialiser->Read(c_MaterialGuid, mateiralName);

				if (!serialisedGuid.empty())
				{
					Core::GUID assetGuid = guidDeserialiser(serialisedGuid);
					// Load resource.
					const Ref<Runtime::ModelAsset> model = Runtime::AssetRegistry::Instance().LoadAsset(assetGuid).As<Runtime::ModelAsset>();
					if (model)
					{
						for (u32 materialIdx = 0; materialIdx < model->GetMaterialCount(); ++materialIdx)
						{
							Ref<Runtime::MaterialAsset> assetResourceMaterial = model->GetMaterialByIndex(materialIdx);
							if (assetResourceMaterial->GetName() == mateiralName)
							{
								material = assetResourceMaterial;
								break;
							}
						}
					}
					else
					{
						IS_LOG_CORE_ERROR("[ComplexSerialiser<MaterialToGuid>] Unable to load model with Guid '{}'.", assetGuid.ToString());
					}
				}
			}
			else
			{
				PropertySerialiser<Core::GUID> guidSerialiser;
				serialiser->Write(c_AssetGuid, material ? guidSerialiser(material->GetAssetInfo()->Guid) : "");
				serialiser->Write(c_MaterialGuid, material ? material->GetName().data() : "");
			}
		}
    }
}