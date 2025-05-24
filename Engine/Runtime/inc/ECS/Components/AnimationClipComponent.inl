#include "AnimationClipComponent.h"

#include "Core/Logger.h"

#include "Asset/AssetRegistry.h"
#include "Asset/Assets/Model.h"

#include "Asset/AssetInfo.h"

namespace Insight
{
    namespace Serialisation
    {
        void ComplexSerialiser<AnimationClipToGuid, Ref<Runtime::AnimationClip>, ECS::AnimationClipComponent>::operator()
            (ISerialiser* serialiser, Ref<Runtime::AnimationClip>& animationClip, ECS::AnimationClipComponent* component) const
        {
			constexpr const char* c_AssetGuid = "AssetGuid";
			constexpr const char* c_AnimationName = "AnimationName";
			if (serialiser->IsReadMode())
			{
				PropertyDeserialiser<Core::GUID> guidDeserialiser;
				std::string serialisedGuid;
				std::string animationName;
				serialiser->Read(c_AssetGuid, serialisedGuid);
				serialiser->Read(c_AnimationName, animationName);

				if (!serialisedGuid.empty())
				{
					Core::GUID assetGuid = guidDeserialiser(serialisedGuid);
					// Load resource.
					const Ref<Runtime::ModelAsset> model = Runtime::AssetRegistry::Instance().LoadAsset(assetGuid).As<Runtime::ModelAsset>();
					if (model)
					{
						component->SetAnimationClip(model->GetAnimationByName(animationName));
						component->SetSkeleton(model->GetSkeleton(0));
						if (!component->GetAnimationClip() || !component->GetSkeleton())
						{
							IS_LOG_CORE_ERROR("[ComplexSerialiser<AnimationClipToGuid>] Unable to load animation clip or skeleton from model with Guid '{}'.", assetGuid.ToString());
						}
					}
					else
					{
						IS_LOG_CORE_ERROR("[ComplexSerialiser<AnimationClipToGuid>] Unable to load model with Guid '{}'.", assetGuid.ToString());
					}
				}
			}
			else
			{
				PropertySerialiser<Core::GUID> guidSerialiser;
				serialiser->Write(c_AssetGuid, animationClip ? guidSerialiser(animationClip->GetAssetInfo()->Guid) : "");
				serialiser->Write(c_AnimationName, animationClip ? animationClip->GetName().data() : "");
			}
        }
    }
}