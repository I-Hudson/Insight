#pragma once

#include "Core/ISysytem.h"
#include "Core/Singleton.h"
#include "Core/GUID.h"

#include "Animation/Animator.h"

#include "Graphics/RHI/RHI_Buffer.h"

#include <mutex>

namespace Insight
{
    namespace ECS
    {
        class SkinnedMeshComponent;
    }

    namespace Runtime
    {
        /// @brief Store a singe instance of something that is being animated by the animation system.
        struct AnimationInstance
        {
            Animator Animator;
            Core::GUID OwnerObject;
        };

        /// @brief Store a single instance of skinned vertex data which has been updated on the GPU.
        struct AnimationGPUSkinning
        {
            Skeleton Skeleton;
            Graphics::RHI_BufferView SkeletonBones;
            Graphics::RHI_BufferView SkinnedVertex;
            Core::GUID OwnerObject;
        };

        class AnimationSystem : public Core::ISystem, public Core::Singleton<AnimationSystem>
        {
        public:
            AnimationSystem();
            virtual ~AnimationSystem() override;

            IS_SYSTEM(AnimationSystem);

            void Update(const float deltaTime);

            AnimationInstance* AddAnimationInstance(const Core::GUID& guid);
            void RemoveAnimationInstance(const Core::GUID& guid);

            AnimationInstance* GetAnimationInstance(const Core::GUID& guid);
            const AnimationInstance* GetAnimationInstance(const Core::GUID& guid) const;

        private:
            void GPUSkinning();

        private:
            std::vector<AnimationInstance> m_animations;
            mutable std::mutex m_animationsLock;
            
            bool m_enableGPUSkinning = false;
            Graphics::RHI_Buffer* m_GPUSkinningBuffer = nullptr;
            std::unordered_map<Core::GUID, Graphics::RHI_BufferView> m_skeletonBonesBuffers;
            std::unordered_map<Core::GUID, Graphics::RHI_BufferView> m_VertexBuffers;
        };
    }
}