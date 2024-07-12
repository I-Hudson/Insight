#pragma once

#include "Runtime/Defines.h"
#include "Core/ISysytem.h"
#include "Core/Singleton.h"
#include "Core/GUID.h"
#include "Threading/TaskSystem.h"

#include "Animation/Animator.h"
#include "Graphics/RenderFrame.h"
#include "Graphics/RenderContext.h"

#include "Graphics/RHI/RHI_Buffer.h"

#include <mutex>

namespace Insight
{
    namespace ECS
    {
        class Entity;
        class SkinnedMeshComponent;
    }

    namespace Runtime
    {
        class Mesh;

        /// @brief Store a singe instance of something that is being animated by the animation system.
        struct AnimationInstance
        {
            Animator Animator;
            std::shared_ptr<Threading::Task> UpdateTask = nullptr;
            const ECS::Entity* Entity;
        };

        /// @brief Store a single instance of skinned vertex data which has been updated on the GPU.
        struct AnimationGPUSkinning
        {
            Ref<Skeleton> Skeleton;
            Graphics::RHI_BufferView SkeletonBones;
            Graphics::RHI_BufferView InputMeshVertices;
            Graphics::RHI_BufferView SkinnedVertex;

            u32 InputVertexCount = 0;

            const ECS::Entity* Entity;
        };

        class IS_RUNTIME AnimationSystem : public Core::ISystem, public Core::Singleton<AnimationSystem>
        {
        public:
            AnimationSystem();
            virtual ~AnimationSystem() override;

            IS_SYSTEM(AnimationSystem);

            // Begin - ISystem
            virtual void Initialise() override;
            virtual void Shutdown() override;
            // End - ISystem

            void Update(const float deltaTime);
            void WaitForAllAnimationUpdates() const;
            void GPUSkinning(RenderFrame& renderFrame);

            void SetGPUSkinningEnabled(const bool enabled) { m_enableGPUSkinning = enabled; }
            bool IsGPUSkinningEnabled() const { return m_enableGPUSkinning; }

            AnimationInstance* AddAnimationInstance(const ECS::Entity* entity);
            void RemoveAnimationInstance(const ECS::Entity* entity);

            AnimationInstance* GetAnimationInstance(const ECS::Entity* entity);
            const AnimationInstance* GetAnimationInstance(const ECS::Entity* entity) const;

        private:
            void InitGPUSkinningResources();
            void DestroyGPUSkinningResoruces();

            Graphics::RHI_BufferView UploadGPUSkeletonBoneData(const AnimationInstance& anim, const Ref<Skeleton>& skeleton);
            Graphics::RHI_BufferView AllocateMeshVertexBuffer(const AnimationInstance& anim, RenderMesh& skinnedMesh);
            void SetupComputeSkinningPass();

        private:
            std::vector<AnimationInstance> m_animations;
            mutable std::mutex m_animationsLock;
            
            bool m_enableGPUSkinning = false;

            Graphics::RHI_Buffer* m_GPUSkeletonBonesUploadBuffer = nullptr;

            Graphics::FrameResource<Graphics::RHI_Buffer*> m_GPUSkeletonBonesBuffer;
            std::unordered_map<Core::GUID, Graphics::RHI_BufferView> m_skeletonBonesBuffers;

            Graphics::RHI_Buffer* m_GPUSkinnedVertexBuffer = nullptr;
            std::unordered_map<Core::GUID, Graphics::RHI_BufferView> m_vertexBuffers;

            /// @brief Base offset within 'm_GPUSkinningBuffer' in which the bones data is stored from.
            u64 m_gpuBoneBaseOffset = 0;
            /// @brief The current offset this frame for all bone data.
            u64 m_gpuBoneOffset = 0;

            /// @brief Base offset for all vertex data within 'm_GPUSkinningBuffer'.
            u64 m_gpuVertexBaseOffset = 0;
            /// @brief The current offset this frame for all vertex data.
            u64 m_gpuVertexOffset = 0;

            // Store all skeleton data blocks within our GPU to be allocated to skeletons.
            std::vector<Graphics::RHI_BufferView> m_GPUSkeletonBlocks;

            std::vector<AnimationGPUSkinning> m_animGPUSkinning;
        };
    }
}