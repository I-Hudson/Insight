#include "Animation/AnimationSystem.h"
#include "World/WorldSystem.h"
#include "ECS/Components/SkinnedMeshComponent.h"

#include "Graphics/Vertex.h"
#include "Graphics/RenderContext.h"
#include "Graphics/RenderGraph/RenderGraph.h"
#include "Graphics/RenderGraph/RenderGraphBuilder.h"

#include "Core/Profiler.h"
#include "Core/Logger.h"

namespace Insight
{
    namespace Runtime
    {
#define BACKGROUND_THREADS 0
#define PARALLEL_FOR 1
#define ANIMATION_THREADING 1

        const u64 c_MaxGPUSkinnedObjects = 1024;
        const u64 c_SkeletonBoneDataIncrementSize = sizeof(Maths::Matrix4) * Skeleton::c_MaxBoneCount;
        const u64 c_SkeletonBoneDataByteSize = c_SkeletonBoneDataIncrementSize * c_MaxGPUSkinnedObjects;

#ifdef VERTEX_SPLIT_STREAMS
        const u64 c_MaxVertexCount = 256_MB / Graphics::Vertices::GetStride(Graphics::Vertices::Stream::Position);
        const u64 c_VertexByteSize = Graphics::Vertices::GetStride(Graphics::Vertices::Stream::Position) * c_MaxVertexCount;
#else
        const u64 c_MaxVertexCount = 256_MB / Graphics::Vertices::GetStride(Graphics::Vertices::Stream::Interleaved);
        const u64 c_VertexByteSize = Graphics::Vertices::GetStride(Graphics::Vertices::Stream::Interleaved) * c_MaxVertexCount;
#endif

        AnimationSystem::AnimationSystem()
        { }

        AnimationSystem::~AnimationSystem()
        { }

        void AnimationSystem::Initialise()
        {
            std::lock_guard lock(m_animationsLock);

            m_enableGPUSkinning = false;

            m_state = Core::SystemStates::Initialised;
        }

        void AnimationSystem::Shutdown()
        {
            std::lock_guard lock(m_animationsLock);
            m_animations.clear();

            DestroyGPUSkinningResoruces();

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
            Threading::ParallelFor<AnimationInstance>("AnimationUpdate", 16, m_animations, [deltaTime](AnimationInstance& instance)
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

            InitGPUSkinningResources();

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
                
#ifdef VERTEX_SPLIT_STREAMS
                Graphics::RHI_BufferView inputSkinnedVertexBuffer(skinnedMesh.GetLOD(0).Vertex_buffer, skinnedMesh.GetLOD(0).Vertex_offset
                    , skinnedMesh.GetLOD(0).Vertex_count * Graphics::Vertices::GetStride(Graphics::Vertices::Stream::Position));
                inputSkinnedVertexBuffer.UAVStartIndex = skinnedMesh.GetLOD(0).Vertex_offset;
                inputSkinnedVertexBuffer.UAVNumOfElements = skinnedMesh.GetLOD(0).Vertex_count;
                inputSkinnedVertexBuffer.Stride = Graphics::Vertices::GetStride(Graphics::Vertices::Stream::Position);
#else
                Graphics::RHI_BufferView inputSkinnedVertexBuffer(skinnedMesh.GetLOD(0).VertexBuffer, skinnedMesh.GetLOD(0).Vertex_offset
                    , skinnedMesh.GetLOD(0).Vertex_count * Graphics::Vertices::GetStride(Graphics::Vertices::Stream::Interleaved));
                inputSkinnedVertexBuffer.UAVStartIndex = skinnedMesh.GetLOD(0).Vertex_offset;
                inputSkinnedVertexBuffer.UAVNumOfElements = skinnedMesh.GetLOD(0).Vertex_count;
                inputSkinnedVertexBuffer.Stride = Graphics::Vertices::GetStride(Graphics::Vertices::Stream::Interleaved);
#endif
                AnimationGPUSkinning animGPUSkinning =
                {
                    skeleton,
                    skeletonGPUBuffer,
                    inputSkinnedVertexBuffer,
                    skinnedVertexBuffer,
                    skinnedMesh.GetLOD(0).Vertex_count,
                    animInstance.Entity
                };
                m_animGPUSkinning.push_back(animGPUSkinning);
            }

            SetupComputeSkinningPass();
        }

        void AnimationSystem::InitGPUSkinningResources()
        {
            if (!m_GPUSkeletonBonesUploadBuffer.GetCurrent())
            {
                Graphics::RHI_Buffer_Overrides gpuSkinningUploadBufferOverrides;
                gpuSkinningUploadBufferOverrides.Force_Host_Writeable = true;
             
                m_GPUSkeletonBonesUploadBuffer.GetCurrent() = Renderer::CreateRawBuffer(c_SkeletonBoneDataByteSize, gpuSkinningUploadBufferOverrides);
                m_GPUSkeletonBonesUploadBuffer.GetCurrent()->SetName("GPUSkeletonBonesUploadBuffer1");

                m_GPUSkeletonBonesUploadBuffer.GetPending() = Renderer::CreateRawBuffer(c_SkeletonBoneDataByteSize, gpuSkinningUploadBufferOverrides);
                m_GPUSkeletonBonesUploadBuffer.GetPending()->SetName("GPUSkeletonBonesUploadBuffer2");
            }

            if (!m_GPUSkeletonBonesBuffer)
            {
                Graphics::RHI_Buffer_Overrides overrides;
                overrides.AllowUnorderedAccess = true;
                m_GPUSkeletonBonesBuffer = Renderer::CreateRawBuffer(c_SkeletonBoneDataByteSize, overrides);
                m_GPUSkeletonBonesBuffer->SetName("GPUSkeletonBonesBuffer");
            }

            if (!m_GPUSkinnedVertexBuffer)
            {
                Graphics::RHI_Buffer_Overrides gpuSkinningVertexBufferOverrides;
                gpuSkinningVertexBufferOverrides.AllowUnorderedAccess = true;
                gpuSkinningVertexBufferOverrides.InitialUploadState = Graphics::DeviceUploadStatus::Completed;
#ifdef VERTEX_SPLIT_STREAMS
                m_GPUSkinnedVertexBuffer = Renderer::CreateVertexBuffer(c_VertexByteSize, Graphics::Vertices::GetStride(Graphics::Vertices::Stream::Position), gpuSkinningVertexBufferOverrides);
#else
                m_GPUSkinnedVertexBuffer = Renderer::CreateVertexBuffer(c_VertexByteSize, Graphics::Vertices::GetStride(Graphics::Vertices::Stream::Interleaved), gpuSkinningVertexBufferOverrides);
#endif
                m_GPUSkinnedVertexBuffer->SetName("GPUSkinnedVertexBuffer");
            }

            m_gpuBoneBaseOffset = 0;
            m_gpuVertexBaseOffset = 0;
        }

        void AnimationSystem::DestroyGPUSkinningResoruces()
        {
            Renderer::FreeRawBuffer(m_GPUSkeletonBonesBuffer);
            m_GPUSkeletonBonesBuffer = nullptr;

            Renderer::FreeVertexBuffer(m_GPUSkeletonBonesUploadBuffer.GetCurrent());
            m_GPUSkeletonBonesUploadBuffer.GetCurrent() = nullptr;
            Renderer::FreeVertexBuffer(m_GPUSkeletonBonesUploadBuffer.GetPending());
            m_GPUSkeletonBonesUploadBuffer.GetPending() = nullptr;

            Renderer::FreeVertexBuffer(m_GPUSkinnedVertexBuffer);
            m_GPUSkinnedVertexBuffer = nullptr;
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
            
            Graphics::RHI_BufferView gpuBoneData = m_GPUSkeletonBonesUploadBuffer.GetCurrent()->Upload(bones.data(), bonesBytesSize, m_gpuBoneBaseOffset + m_gpuBoneOffset, 8);
            gpuBoneData = Graphics::RHI_BufferView(m_GPUSkeletonBonesBuffer, gpuBoneData.GetOffset(), gpuBoneData.GetSize());

            gpuBoneData.UAVStartIndex = m_gpuBoneOffset > 0 ? m_gpuBoneOffset / sizeof(bones[0]) : 0;
            gpuBoneData.UAVNumOfElements = (m_gpuBoneOffset + bonesBytesSize) / sizeof(bones[0]);
            gpuBoneData.Stride = sizeof(bones[0]);

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

#ifdef VERTEX_SPLIT_STREAMS
            const u64 vertexByteSize = Graphics::Vertices::GetStride(Graphics::Vertices::Stream::Position) * vertexCount;
            ASSERT(m_gpuVertexOffset + vertexByteSize < c_VertexByteSize);

            Graphics::RHI_BufferView gpuVertexData(m_GPUSkinnedVertexBuffer, m_gpuVertexBaseOffset + m_gpuVertexOffset, vertexByteSize);
            gpuVertexData.UAVStartIndex = m_gpuVertexOffset > 0 ? m_gpuVertexOffset / Graphics::Vertices::GetStride(Graphics::Vertices::Stream::Position) : 0;
            gpuVertexData.UAVNumOfElements = vertexCount;
            gpuVertexData.Stride = Graphics::Vertices::GetStride(Graphics::Vertices::Stream::Position);

            skinnedMesh.MeshLods[0].VertexBufferView = gpuVertexData;
#else
            const u64 vertexByteSize = Graphics::Vertices::GetStride(Graphics::Vertices::Stream::Interleaved) * vertexCount;
            ASSERT(m_gpuVertexOffset + vertexByteSize < c_VertexByteSize);

            Graphics::RHI_BufferView gpuVertexData(m_GPUSkinnedVertexBuffer, m_gpuVertexBaseOffset + m_gpuVertexOffset, vertexByteSize);
            gpuVertexData.UAVStartIndex = m_gpuVertexOffset > 0 ? m_gpuVertexOffset / Graphics::Vertices::GetStride(Graphics::Vertices::Stream::Interleaved) : 0;
            gpuVertexData.UAVNumOfElements = vertexCount;
            gpuVertexData.Stride = Graphics::Vertices::GetStride(Graphics::Vertices::Stream::Interleaved);

            skinnedMesh.MeshLods[0].VertexBufferView = gpuVertexData;
#endif
            m_vertexBuffers[anim.Entity->GetGUID()] = gpuVertexData;

            m_gpuVertexOffset += vertexByteSize;

            return gpuVertexData;
        }

        void AnimationSystem::SetupComputeSkinningPass()
        {
            struct AnimationSkinnedData
            {
                Graphics::RHI_Buffer* GPUBonesData;
                Graphics::RHI_Buffer* GPUSkinnedVertexData;
                std::vector<AnimationGPUSkinning> AnimGPUSkinning;
            };
            AnimationSkinnedData animationSkinnedData =
            {
                m_GPUSkeletonBonesBuffer,
                m_GPUSkinnedVertexBuffer,
                std::move(m_animGPUSkinning)
            };

            Graphics::RHI_Buffer* boneUploadBuffer = m_GPUSkeletonBonesUploadBuffer.GetCurrent();
            Graphics::GPUDeferedManager::Instance().Push([this, boneUploadBuffer](Graphics::RHI_CommandList* cmdList)
                {
                    cmdList->CopyBufferToBuffer(m_GPUSkeletonBonesBuffer, 0, boneUploadBuffer,0, boneUploadBuffer->GetSize());
                });
            Graphics::RenderGraph::Instance().AddSyncPoint([this]()
                {
                    m_GPUSkeletonBonesUploadBuffer.Swap();
                });

            Graphics::RenderGraph::Instance().AddPassFront<AnimationSkinnedData>("ComputeSkinning",
            [](AnimationSkinnedData& data, Graphics::RenderGraphBuilder& builder)
            {
                IS_PROFILE_SCOPE("ComputeSkinning pass setup");

                Graphics::ShaderDesc shaderDesc("ComputeSkinning", {}, Graphics::ShaderStageFlagBits::ShaderStage_Compute);
                builder.SetShader(shaderDesc);

                Graphics::ComputePipelineStateObject pso = { };
                {
                    IS_PROFILE_SCOPE("SetPipelineStateObject");
                    pso.Name = "ComputeSkinning";
                    pso.ShaderDescription = shaderDesc;
                }
                builder.SetComputePipeline(pso);
            },
            [this](AnimationSkinnedData& data, Graphics::RenderGraph& render_graph, Graphics::RHI_CommandList* cmdList)
            {
                IS_PROFILE_SCOPE("ComputeSkinning pass execute");

                Graphics::PipelineBarrier beforeBarreir;
                beforeBarreir.SrcStage = static_cast<u32>(Graphics::PipelineStageFlagBits::TopOfPipe);
                beforeBarreir.DstStage = static_cast<u32>(Graphics::PipelineStageFlagBits::ComputeShader);

                Graphics::BufferBarrier beforeBufferBarrier;
                beforeBufferBarrier.SrcAccessFlags = Graphics::AccessFlagBits::None;
                beforeBufferBarrier.DstAccessFlags = Graphics::AccessFlagBits::ShaderWrite;

                beforeBufferBarrier.NewLayout = Graphics::BufferType::UnorderedAccess;

                beforeBufferBarrier.Buffer = data.GPUBonesData;
                beforeBarreir.BufferBarriers.push_back(beforeBufferBarrier);

                beforeBufferBarrier.Buffer = data.GPUSkinnedVertexData;
                beforeBarreir.BufferBarriers.push_back(beforeBufferBarrier);

                cmdList->PipelineBarrier(beforeBarreir);

                Graphics::ComputePipelineStateObject pso = render_graph.GetComputePipelineStateObject("ComputeSkinning");
                cmdList->BindPipeline(pso);

                struct MeshInfo
                {
                    u32 VertexCount;
                    u32 __pad0 = 0;
                    u32 __pad1 = 0;
                    u32 __pad2 = 0;
                };

                for (size_t i = 0; i < data.AnimGPUSkinning.size(); ++i)
                {
                    const AnimationGPUSkinning& animGPUSkinning = data.AnimGPUSkinning[i];
                    cmdList->SetUnorderedAccess(0, 0, animGPUSkinning.InputMeshVertices);
                    cmdList->SetUnorderedAccess(0, 1, animGPUSkinning.SkinnedVertex);
                    cmdList->SetUnorderedAccess(0, 2, animGPUSkinning.SkeletonBones);
                    cmdList->SetUniform(0, 0, MeshInfo{ animGPUSkinning.InputVertexCount });

                    const u32 threadGroupCount = 64;
                    const u32 threadGroupCountX = static_cast<uint32_t>(
                        std::ceil(static_cast<float>(animGPUSkinning.SkinnedVertex.UAVNumOfElements) / static_cast<float>(threadGroupCount)));

                    cmdList->Dispatch(threadGroupCountX, 1);
                }

                Graphics::PipelineBarrier afterBarreir;
                afterBarreir.SrcStage = static_cast<u32>(Graphics::PipelineStageFlagBits::ComputeShader);
                afterBarreir.DstStage = static_cast<u32>(Graphics::PipelineStageFlagBits::FragmentShader);

                Graphics::BufferBarrier afterBufferBarrier;
                afterBufferBarrier.SrcAccessFlags = beforeBufferBarrier.DstAccessFlags;
                afterBufferBarrier.DstAccessFlags = Graphics::AccessFlagBits::ShaderRead;

                afterBufferBarrier.NewLayout = Graphics::BufferType::Raw;
                afterBufferBarrier.Buffer = data.GPUBonesData;
                afterBarreir.BufferBarriers.push_back(afterBufferBarrier);

                afterBufferBarrier.DstAccessFlags = Graphics::AccessFlagBits::VertexAttributeRead;
                afterBufferBarrier.NewLayout = Graphics::BufferType::Vertex;
                afterBufferBarrier.Buffer = data.GPUSkinnedVertexData;
                afterBarreir.BufferBarriers.push_back(afterBufferBarrier);

                cmdList->PipelineBarrier(afterBarreir);
            }, [](AnimationSkinnedData&, Graphics::RenderGraph&, Graphics::RHI_CommandList*) { }, std::move(animationSkinnedData));
        }

    }
}
