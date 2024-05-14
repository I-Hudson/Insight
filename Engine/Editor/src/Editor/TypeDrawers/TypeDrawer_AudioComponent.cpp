#include "Editor/TypeDrawers/TypeDrawer_AudioComponent.h"
#include "Editor/EditorGUI.h"
#include "Editor/EditorWindows/ContentWindow.h"

#include "Asset/AssetRegistry.h"

#include "ECS/Components/AudioComponent.h"

#include <imgui.h>

namespace Insight
{
    namespace Editor
    {
        std::string TypeDrawer_AudioComponent::GetTypeName()
        {
            return ECS::AudioComponent::Type_Name;
        }

        void TypeDrawer_AudioComponent::Draw(void* data, const Reflect::MemberInfo& memberInfo, const std::string_view label) const
        {
            ECS::AudioComponent* audioComponent = static_cast<ECS::AudioComponent*>(data);
            
            Ref<Runtime::AudioClipAsset> audioClipAsset = audioComponent->GetAudioClip();
            std::string audioClipName;
            if (audioClipAsset)
            {
                audioClipName = audioClipAsset->IsMemoryAsset() ? "" : audioClipAsset->GetAssetInfo()->FileName;
            }
            ImGui::InputText("Audio Clip", audioClipName.data(), ImGuiInputTextFlags_ReadOnly);

            std::string audioClipGuid;
            if (EditorGUI::ObjectFieldTarget(ContentWindow::c_ContentWindowAssetDragSource, audioClipGuid))
            {
                Core::GUID assetGuid;
                assetGuid.StringToGuid(audioClipGuid);
                Ref<Runtime::AudioClipAsset> newAudioClipAsset = Runtime::AssetRegistry::Instance().LoadAsset(assetGuid).As<Runtime::AudioClipAsset>();
                if (!newAudioClipAsset)
                {
                    return;
                }

                audioComponent->SetAudioClip(newAudioClipAsset);
            }
        }
    }
}