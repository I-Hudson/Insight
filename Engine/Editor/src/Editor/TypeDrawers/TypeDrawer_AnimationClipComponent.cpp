#include "Editor/TypeDrawers/TypeDrawer_AnimationClipComponent.h"
#include "Editor/EditorGUI.h"
#include "Editor/EditorWindows/ContentWindow.h"

#include "Asset/AssetRegistry.h"

#include "ECS/Components/AnimationClipComponent.h"

#include <imgui.h>

namespace Insight
{
    namespace Editor
    {
        std::string TypeDrawer_AnimationClipComponent::GetTypeName()
        {
            return ECS::AnimationClipComponent::Type_Name;
        }

        void TypeDrawer_AnimationClipComponent::Draw(void* data, const Reflect::MemberInfo& memberInfo, const std::string_view label) const
        {
            ECS::AnimationClipComponent* animationClipComponent = static_cast<ECS::AnimationClipComponent*>(data);
            
            Ref<Runtime::AnimationClip> animationClip = animationClipComponent->GetAnimationClip();
            std::string animationClipName;
            if (animationClip)
            {
                animationClipName = animationClip->GetGuid().ToString();
            }
            ImGui::InputText("Clip", animationClipName.data(), ImGuiInputTextFlags_ReadOnly);


            std::string animationClipGuid;
            if (EditorGUI::ObjectFieldTarget(ContentWindow::c_ContentWindowAssetDragSource, animationClipGuid))
            {
                Core::GUID assetGuid;
                assetGuid.StringToGuid(animationClipGuid);
                Ref<Runtime::AnimationClip> newAnimationClip = Runtime::AssetRegistry::Instance().LoadAsset(assetGuid).As<Runtime::AnimationClip>();
                if (!newAnimationClip)
                {
                    return;
                }
                animationClipComponent->SetAnimationClip(newAnimationClip);
            }

            if (ImGui::Button("Play"))
            {
                animationClipComponent->Play();
            }
            ImGui::SameLine();
            if (ImGui::Button("Stop"))
            {
                animationClipComponent->Stop();
            }
        }
    }
}