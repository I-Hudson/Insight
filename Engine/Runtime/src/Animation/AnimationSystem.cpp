#include "Animation/AnimationSystem.h"
#include "World/WorldSystem.h"
#include "ECS/Components/SkinnedMeshComponent.h"

#include "Graphics/Vertex.h"
#include "Graphics/RenderContext.h"

#include "Core/Profiler.h"
#include "Core/Logger.h"

namespace Insight
{
    namespace Runtime
    {
#define ANIMATION_THREADING 1
#define PARALLEL_FOR 1
#define BACKGROUND_THREADS 1

        const u64 c_MaxGPUSkinnedObjects = 1024;
        const u64 c_SkeletonBoneDataIncrementSize = sizeof(Maths::Matrix4) * Skeleton::c_MaxBoneCount;
        const u64 c_SkeletonBoneDataByteSize = c_SkeletonBoneDataIncrementSize * c_MaxGPUSkinnedObjects;

        const u64 c_MaxVertexCount = 128_MB / sizeof(Graphics::Vertex);
        const u64 c_VertexByteSize = sizeof(Graphics::Vertex) * c_MaxVertexCount;

        AnimationSystem::AnimationSystem()
        { }

        AnimationSystem::~AnimationSystem()
        { }

        void AnimationSystem::Initialise()
        {
            std::lock_guard lock(m_animationsLock);

            Graphics::RHI_Buffer_Overrides vertexOverrides;
            vertexOverrides.Force_Host_Writeable = true;

            m_GPUSkeletonBonesBuffer = Renderer::CreateRawBuffer(c_SkeletonBoneDataByteSize, vertexOverrides);
            m_GPUSkeletonBonesBuffer->SetName("GPUSkeletonBonesBuffer");

            m_GPUSkinnedVertexBuffer = Renderer::CreateVertexBuffer(c_VertexByteSize, sizeof(Graphics::Vertex));
            m_GPUSkinnedVertexBuffer->SetName("GPUSkinnedVertexBuffer");

            m_gpuBoneBaseOffset = 0;
            m_gpuVertexBaseOffset = 0;

            m_enableGPUSkinning = true;

            m_state = Core::SystemStates::Initialised;
        }

        void AnimationSystem::Shutdown()
        {
            std::lock_guard lock(m_animationsLock);
            m_animations.clear();

            Renderer::FreeVertexBuffer(m_GPUSkeletonBonesBuffer);
            m_GPUSkeletonBonesBuffer = nullptr;

            Renderer::FreeVertexBuffer(m_GPUSkinnedVertexBuffer);
            m_GPUSkinnedVertexBuffer = nullptr;

            m_state = Core::SystemStates::Not_Initialised;
        }

        void AnimationSystem::Update(const float deltaTime)
        {
            TObjectPtr<World> activeWorld = WorldSystem::Instance().GetActiveWorld();
            if (!activeWorld
                || (activeWorld && activeWorld->GetWorldState() == WorldStates::Paused))
            {
                return;
            }

            std::lock_guard lock(m_animationsLock);
#if BACKGROUND_THREADS
            for (size_t animIdx = 0; animIdx < m_animations.size(); ++animIdx)
            {
                AnimationInstance& instance = m_animations[animIdx];
                instance.UpdateTask = nullptr;
                instance.UpdateTask = Threading::TaskSystem::Instance().CreateTask([&instance, deltaTime]()
                    {
                        instance.Animator.Update(deltaTime);
                    });
            }
#elif PARALLEL_FOR
            Threading::ParallelFor<AnimationInstance>(16, m_animations, [deltaTime](AnimationInstance& instance)
            {
                instance.Animator.Update(deltaTime);
            });
#else
#if ANIMATION_THREADING
            std::vector<std::shared_ptr<Threading::Task>> aniationTasks;
#endif
            for (size_t animIdx = 0; animIdx < m_animations.size(); ++animIdx)
            {
                AnimationInstance& instance = m_animations[animIdx];
#if ANIMATION_THREADING
                aniationTasks.push_back(Threading::TaskSystem::Instance().CreateTask([&]()
                    {
                        instance.Animator.Update(deltaTime);
                    }));
#else
                instance.Animator.Update(deltaTime);
#endif
            }

#if ANIMATION_THREADING
            for (size_t taskIdx = 0; taskIdx < aniationTasks.size(); ++taskIdx)
            {
                IS_PROFILE_SCOPE("Anin wait");
                std::shared_ptr<Threading::Task>& task = aniationTasks[taskIdx];
                task->Wait();
            }
#endif
#endif
        }

        void AnimationSystem::WaitForAllAnimationUpdates() const
        {
            IS_PROFILE_FUNCTION();
            std::lock_guard lock(m_animationsLock);
            for (size_t animIdx = 0; animIdx < m_animations.size(); ++animIdx)
            {
                const AnimationInstance& instance = m_animations[animIdx];
                if (instance.UpdateTask)
                {
                    instance.UpdateTask->Wait();
                }
            }
        }

        AnimationInstance* AnimationSystem::AddAnimationInstance(const ECS::Entity* entity)
        {
            if (AnimationInstance* instance = GetAnimationInstance(entity))
            {
                return instance;
            }

            std::lock_guard lock(m_animationsLock);
            m_animations.push_back(AnimationInstance
            {
                Animator(),
                nullptr,
                entity
            });
            return &m_animations.back();
        }

        void AnimationSystem::RemoveAnimationInstance(const ECS::Entity* entity)
        {
            std::lock_guard lock(m_animationsLock);
            for (size_t animIdx = 0; animIdx < m_animations.size(); ++animIdx)
            {
                const AnimationInstance& instance = m_animations[animIdx];
                if (entity == instance.Entity)
                {
                    m_animations.erase(m_animations.begin() + animIdx);
                    return;
                }
            }
        }

        AnimationInstance* AnimationSystem::GetAnimationInstance(const ECS::Entity* entity)
        {
            std::lock_guard lock(m_animationsLock);
            for (size_t animIdx = 0; animIdx < m_animations.size(); ++animIdx)
            {
                AnimationInstance& instance = m_animations[animIdx];
                if (entity == instance.Entity)
                {
                    return &instance;
                }
            }
            return nullptr;
        }

        const AnimationInstance* AnimationSystem::GetAnimationInstance(const ECS::Entity* entity) const
        {
            std::lock_guard lock(m_animationsLock);
            for (size_t animIdx = 0; animIdx < m_animations.size(); ++animIdx)
            {
                const AnimationInstance& instance = m_animations[animIdx];
                if (entity == instance.Entity)
                {
                    return &instance;
                }
            }
            return nullptr;
        }

        void AnimationSystem::GPUSkinning(RenderFrame& renderFrame)
        {
            std::lock_guard lock(m_animationsLock);

            if (!m_enableGPUSkinning)
            {
                return;
            }

            m_gpuBoneOffset = 0;
            m_gpuVertexOffset = 0;

            m_animGPUSkinning.clear();
            //m_skeletonBonesBuffers.clear();
            //m_vertexBuffers.clear();

            for (size_t animIdx = 0; animIdx < m_animations.size(); ++animIdx)
            {
                const AnimationInstance& animInstance = m_animations[animIdx];

                const Ref<Skeleton> skeleton = animInstance.Animator.GetSkelton();
                const Graphics::RHI_BufferView skeletonGPUBuffer = UploadGPUSkeletonBoneData(animInstance, skeleton);

                RenderMesh& skinnedMesh = renderFrame.GetRenderMesh(animInstance.Entity);
                const Graphics::RHI_BufferView skinnedVertexBuffer = AllocateMeshVertexBuffer(animInstance, skinnedMesh);

                AnimationGPUSkinning animGPUSkinning =
                {
                    skeleton,
                    skeletonGPUBuffer,
                    skinnedVertexBuffer,
                    animInstance.Entity
                };
                m_animGPUSkinning.push_back(animGPUSkinning);
            }
        }

        Graphics::RHI_BufferView AnimationSystem::UploadGPUSkeletonBoneData(const AnimationInstance& anim, const Ref<Skeleton>& skeleton)
        {
            //if (auto iter = m_skeletonBonesBuffers.find(skeleton->GetGuid());
            //    iter != m_skeletonBonesBuffers.end())
            //{
            //    return iter->second;
            //}

            const std::vector<Maths::Matrix4> bones = anim.Animator.GetBoneTransforms();

            const u64 bonesBytesSize = sizeof(bones[0]) * bones.size();
            ASSERT(m_gpuBoneOffset + bonesBytesSize < c_SkeletonBoneDataByteSize);
            
            const Graphics::RHI_BufferView gpuBoneData = m_GPUSkeletonBonesBuffer->Upload(bones.data(), bonesBytesSize, m_gpuBoneBaseOffset + m_gpuBoneOffset, 8);
            m_skeletonBonesBuffers[anim.Entity->GetGUID()] = gpuBoneData;

            ASSERT(bonesBytesSize <= c_SkeletonBoneDataIncrementSize);
            m_gpuBoneOffset += bonesBytesSize;

            return gpuBoneData;
        }

        Graphics::RHI_BufferView AnimationSystem::AllocateMeshVertexBuffer(const AnimationInstance& anim, RenderMesh& skinnedMesh)
        {
            //if (auto iter = m_vertexBuffers.find(mesh->GetGuid());
            //    iter != m_vertexBuffers.end())
            //{
            //    return iter->second;
            //}

            u64 vertexCount = skinnedMesh.MeshLods[0].Vertex_count;

            const u64 vertexByteSize = sizeof(Graphics::Vertex) * vertexCount;
            ASSERT(m_gpuVertexOffset + vertexByteSize < c_VertexByteSize);

            const Graphics::RHI_BufferView gpuVertexData(m_GPUSkinnedVertexBuffer, m_gpuVertexBaseOffset + m_gpuVertexOffset, vertexByteSize);
            m_vertexBuffers[anim.Entity->GetGUID()] = gpuVertexData;
            skinnedMesh.MeshLods[0].VertexBufferView = gpuVertexData;

            m_gpuVertexOffset += vertexByteSize;

            return gpuVertexData;
        }
    }
}
