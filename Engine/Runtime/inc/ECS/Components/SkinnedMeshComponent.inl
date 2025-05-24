#include "SkinnedMeshComponent.h"

#include "Core/Logger.h"

#include "Asset/AssetRegistry.h"
#include "Asset/Assets/Model.h"

#include "Asset/AssetInfo.h"

namespace Insight
{
    namespace Serialisation
    {
        void ComplexSerialiser<SkeletonToGuid, Ref<Runtime::Skeleton>, ECS::SkinnedMeshComponent>::operator()
            (ISerialiser* serialiser, Ref<Runtime::Skeleton>& skeleton, ECS::SkinnedMeshComponent* component) const
        {
			constexpr const char* c_AssetGuid = "AssetGuid";
			constexpr const char* c_SkeletonName = "SkeletonName";
			if (serialiser->IsReadMode())
			{
				PropertyDeserialiser<Core::GUID> guidDeserialiser;
				std::string serialisedGuid;
				std::string skeletonName;
				serialiser->Read(c_AssetGuid, serialisedGuid);
				serialiser->Read(c_SkeletonName, skeletonName);

				if (!serialisedGuid.empty())
				{
					Core::GUID assetGuid = guidDeserialiser(serialisedGuid);
					// Load resource.
					const Ref<Runtime::ModelAsset> model = Runtime::AssetRegistry::Instance().LoadAsset(assetGuid).As<Runtime::ModelAsset>();
					if (model)
					{
						Ref<Runtime::Skeleton> skeleton = model->GetSkeletonByName(skeletonName);
						component->SetSkeleton(skeleton);
						if (!skeleton)
						{
							IS_LOG_CORE_ERROR("[ComplexSerialiser<SkeletonToGuid>] Unable to load skeleton from model with Guid '{}'.", assetGuid.ToString());
						}
					}
					else
					{
						IS_LOG_CORE_ERROR("[ComplexSerialiser<SkeletonToGuid>] Unable to load model with Guid '{}'.", assetGuid.ToString());
					}
				}
			}
			else
			{
				PropertySerialiser<Core::GUID> guidSerialiser;
				serialiser->Write(c_AssetGuid, skeleton ? guidSerialiser(skeleton->GetAssetInfo()->Guid) : "");
				serialiser->Write(c_SkeletonName, skeleton ? skeleton->GetName().data() : "");
			}
        }
    }
}