//#include "MeshComponent.h"
//
//
//#include "Asset/AssetInfo.h"
//
//namespace Insight
//{
//    namespace Serialisation
//    {
//        void ComplexSerialiser<MeshToGuid, Runtime::ResourceLFHandle<Runtime::Mesh>, ECS::MeshComponent>::operator()
//            (ISerialiser* serialiser, Runtime::ResourceLFHandle<Runtime::Mesh>& mesh, ECS::MeshComponent* meshComponent) const
//        {
//			constexpr const char* c_AssetGuid = "AssetGuid";
//			constexpr const char* c_MeshGuid = "MeshNameHash";
//			if (serialiser->IsReadMode())
//			{
//				PropertyDeserialiser<Core::GUID> guidDeserialiser;
//				std::string serialisedGuid;
//				std::string meshName;
//				serialiser->Read(c_AssetGuid, serialisedGuid);
//				serialiser->Read(c_MeshGuid, meshName);
//
//				if (!serialisedGuid.empty())
//				{
//					Core::GUID assetGuid = guidDeserialiser(serialisedGuid);
//					// Load resource.
//					const Runtime::Model* assetResource = Runtime::ResourceManager::Instance().Instance().LoadSync(assetGuid).CastTo<Runtime::Model>();
//					for (size_t meshIdx = 0; meshIdx < assetResource->GetMeshCount(); ++meshIdx)
//					{
//						Runtime::Mesh* assetResourceMesh = assetResource->GetMeshByIndex(meshIdx);
//						if (assetResourceMesh->GetName() == meshName)
//						{
//							mesh = assetResourceMesh;
//							break;
//						}
//					}
//				}
//			}
//			else
//			{
//				PropertySerialiser<Core::GUID> guidSerialiser;
//				serialiser->Write(c_AssetGuid, mesh ? guidSerialiser(mesh->GetAssetInfo()->Guid) : "");
//				serialiser->Write(c_MeshGuid, mesh ? mesh->GetName().data() : "");
//			}
//        }
//
//		void ComplexSerialiser<MaterialToGuid, Runtime::ResourceLFHandle<Runtime::Material>, ECS::MeshComponent>::operator()
//			(ISerialiser* serialiser, Runtime::ResourceLFHandle<Runtime::Material>& material, ECS::MeshComponent* meshComponent) const
//		{
//			constexpr const char* c_AssetGuid = "AssetGuid";
//			constexpr const char* c_MaterialGuid = "MaterialGuid";
//			if (serialiser->IsReadMode())
//			{
//				PropertyDeserialiser<Core::GUID> guidDeserialiser;
//				std::string serialisedGuid;
//				std::string mateiralName;
//				serialiser->Read(c_AssetGuid, serialisedGuid);
//				serialiser->Read(c_MaterialGuid, mateiralName);
//
//				if (!serialisedGuid.empty())
//				{
//					Core::GUID assetGuid = guidDeserialiser(serialisedGuid);
//					// Load resource.
//					const Runtime::Model* assetResource = Runtime::ResourceManager::Instance().Instance().LoadSync(assetGuid).CastTo<Runtime::Model>();
//					for (size_t materialIdx = 0; materialIdx < assetResource->GetMaterialCount(); ++materialIdx)
//					{
//						Runtime::Material* assetResourceMaterial = assetResource->GetMaterialByIndex(materialIdx);
//						if (assetResourceMaterial->GetName() == mateiralName)
//						{
//							material = assetResourceMaterial;
//							break;
//						}
//					}
//				}
//			}
//			else
//			{
//				PropertySerialiser<Core::GUID> guidSerialiser;
//				serialiser->Write(c_AssetGuid, material ? guidSerialiser(material->GetAssetInfo()->Guid) : "");
//				serialiser->Write(c_MaterialGuid, material ? material->GetName().data() : "");
//			}
//		}
//    }
//}