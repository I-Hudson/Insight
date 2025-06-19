#include "Graphics/RenderFrame.h"

#include "World/World.h"
#include "World/WorldSystem.h"

#include "Graphics/Window.h"

#include "ECS/Components/TransformComponent.h"
#include "ECS/Components/MeshComponent.h"
#include "ECS/Components/SkinnedMeshComponent.h"
#include "ECS/Components/AnimationClipComponent.h"
#include "ECS/Components/PointLightComponent.h"

#include "Maths/Utils.h"
#include "Maths/Vector3.h"

#include "Core/Profiler.h"
#include "Threading/TaskSystem.h"

namespace Insight
{
    void RenderMaterial::SetMaterial(const Ref<Runtime::MaterialAsset> material)
    {
        if (!material)
        {
            return;
        }

        for (size_t i = 0; i < static_cast<u64>(Runtime::TextureAssetTypes::Count); ++i)
        {
            Ref<Runtime::TextureAsset> texture = material->GetTexture(static_cast<Runtime::TextureAssetTypes>(i));
            if (texture)
            {
                Textures[i] = texture->GetRHITexture();
            }
        }
        Properties = material->GetProperties();
    }

    const Runtime::MeshLOD& RenderMesh::GetLOD(u32 lodIndex) const
    {
        ASSERT(MeshLods.size() > 0);
        lodIndex = std::min(lodIndex, static_cast<u32>(MeshLods.size() - 1));
        return MeshLods[lodIndex];
    }

    //=====================================================
    // RenderMesh
    //=====================================================
    void RenderMesh::SetMesh(Runtime::Mesh* mesh)
    {
        IS_PROFILE_FUNCTION();
        BoudingBox = mesh->GetBoundingBox();
        MeshLods = mesh->m_lods;
    }

    void RenderMesh::SetMaterial(const Ref<Runtime::MaterialAsset> material)
    {
        IS_PROFILE_FUNCTION();
        Material = {};
        Material.SetMaterial(material);
    }

    //=====================================================
    // RenderWorld
    //=====================================================
    void RenderWorld::SetMainCamera(ECS::Camera mainCamera, const Maths::Matrix4 transform)
    {
        MainCamera = RenderCamera{ std::move(mainCamera), std::move(transform), true};
    }

    void RenderWorld::AddCamrea(ECS::Camera camera, const Maths::Matrix4 transform)
    {
        Cameras.push_back(RenderCamera{ std::move(camera), std::move(transform), true });
    }

    //=====================================================
    // RenderFrame
    //=====================================================
    RenderFrame::RenderFrame()
    {
    }

    RenderFrame::~RenderFrame()
    {
    }

    void RenderFrame::CreateRenderFrameFromWorldSystem(Runtime::WorldSystem* worldSystem)
    {
        IS_PROFILE_FUNCTION();
        ASSERT(Platform::IsMainThread());
        Clear();

        std::vector<TObjectPtr<Runtime::World>> worlds = worldSystem->GetAllWorlds();
        for (TObjectPtr<Runtime::World> const& world : worlds)
        {
            IS_PROFILE_SCOPE("RenderWorld");
            if (world->GetWorldType() == Runtime::WorldTypes::Tools)
            {
                continue;
            }

            RenderWorld renderWorld;
            std::vector<Ptr<ECS::Entity>> entities = world->GetAllEntitiesFlatten();
            renderWorld.Meshes.reserve(entities.size());

            std::vector<Ptr<ECS::Entity>> cameraEntities = world->GetAllEntitiesWithComponentByName(ECS::CameraComponent::Type_Name);
            for (Ptr<ECS::Entity>& entity : cameraEntities)
            {
                ECS::CameraComponent* cameraComponent = entity->GetComponent<ECS::CameraComponent>();
                if (renderWorld.MainCamera.IsSet)
                {
                    renderWorld.AddCamrea(cameraComponent->GetCamera(), cameraComponent->GetViewMatrix());
                }
                else
                {
                    renderWorld.SetMainCamera(cameraComponent->GetCamera(), cameraComponent->GetViewMatrix());
                }

                if (!MainCamera.IsSet)
                {
                    MainCamera.Camera = cameraComponent->GetCamera();
                    MainCamera.Transform = cameraComponent->GetViewMatrix();
                    MainCamera.IsSet = true;
                }
            }

            std::mutex renderWorldMutex;
#define PARALLEL_FOR 0
#if PARALLEL_FOR
            Threading::ParallelFor<Ptr<ECS::Entity>>(16, entities, [&](Ptr<ECS::Entity>& entity)
                {
#else
            for (Ptr<ECS::Entity>& entity : entities)
            {
#endif
                IS_PROFILE_SCOPE("Evaluate Single Entity");
                if (!entity->IsEnabled())
                {
#if PARALLEL_FOR
                    return;
#else
                    continue;
#endif
                }

                ECS::TransformComponent* transformComponent = entity->GetComponent<ECS::TransformComponent>();

                if (entity->HasComponent<ECS::MeshComponent>() || entity->HasComponent<ECS::SkinnedMeshComponent>())
                {
                    ECS::MeshComponent* meshComponent = entity->GetComponent<ECS::MeshComponent>();
                    ECS::SkinnedMeshComponent* skinnedMeshComponent = entity->GetComponent<ECS::SkinnedMeshComponent>();

                    if ((meshComponent && meshComponent->IsEnabled())
                        || (skinnedMeshComponent && skinnedMeshComponent->IsEnabled()))
                    {
                        if ((meshComponent && !meshComponent->GetMesh())
                            || (skinnedMeshComponent && !skinnedMeshComponent->GetMesh()))
                        {
#if PARALLEL_FOR
                            return;
#else
                            continue;
#endif
                        }
                        Ref<Runtime::Mesh> mesh = meshComponent ? meshComponent->GetMesh() : skinnedMeshComponent->GetMesh();

                        Ref<Runtime::MaterialAsset> material = meshComponent ? meshComponent->GetMaterial() : skinnedMeshComponent->GetMaterial();
                        if (!material)
                        {
                            //continue;
                        }

                        RenderMesh renderMesh;
                        renderMesh.EntityGuid = entity->GetGUID();
                        {
                            IS_PROFILE_SCOPE("Set Transforms");
                            renderMesh.Transform = transformComponent->GetTransform();
                            renderMesh.PreviousTransform = transformComponent->GetPreviousTransform();
                        }

                        Graphics::BoundingBox boundingBox = mesh->GetBoundingBox();
                        bool isVisible = true;
                        {
                            IS_PROFILE_SCOPE("Visible check");
                            boundingBox = boundingBox.Transform(renderMesh.Transform);
                        }
                        {
                            std::lock_guard l(renderWorldMutex);
                            //isVisible = renderWorld.MainCamera.Camera.IsVisible(boundingBox);
                            if (!renderWorld.MainCamera.IsSet || !isVisible)
                            {
                                // TODO Mid: Setup seperate scene and game worlds and then re-enable this.
                                // The "EditorWorld" should be it's own world then when play is pressed a runtime world 
                                // should be created.
                                //continue;
                            }
                        }

                        renderMesh.SetMesh(mesh.Ptr());
                        renderMesh.SetMaterial(material);
                        {
                            IS_PROFILE_SCOPE("Set SkinnedMesh");
                            renderMesh.SkinnedMesh = skinnedMeshComponent && skinnedMeshComponent->GetSkeleton();
                            if (renderMesh.SkinnedMesh)
                            {
                                ECS::AnimationClipComponent* animationClipComponent = entity->GetComponent<ECS::AnimationClipComponent>();
                                if (animationClipComponent)
                                {
                                    IS_PROFILE_SCOPE("SetBoneTransforms");
                                    renderMesh.SkinnedMeshGuid = animationClipComponent->GetGuid();
#if 0
                                    const u32 skeletonBoneCount = skinnedMeshComponent->GetSkeleton()->GetNumberOfBones();
                                    {
                                        IS_PROFILE_SCOPE("resize");
                                        renderMesh.BoneTransforms.resize(skeletonBoneCount);
                                    }
                                    {
                                        IS_PROFILE_SCOPE("MemCopy");
                                        Platform::MemCopy(renderMesh.BoneTransforms.data(), animationClipComponent->GetAnimator()->GetBoneTransforms().data(), sizeof(Maths::Matrix4) * skeletonBoneCount);
                                    }
#else
                                    // This should just be a RHI_BufferView instead of copying all this data.
                                    renderMesh.BoneTransforms = animationClipComponent->GetAnimator()->GetBoneTransforms();
#endif
                                }
                            }
                        }

                        const bool meshIsTransparent = renderMesh.Material.Properties[static_cast<u64>(Runtime::MaterialAssetProperty::Opacity)] < 1.0f;

                        {
                            std::lock_guard l(renderWorldMutex);
                            u64 meshIndex = meshIndex = renderWorld.Meshes.size();
                            renderWorld.Meshes.push_back(std::move(renderMesh));

                            if (auto materialBatchIter = renderWorld.MaterialBatchLookup.find(material->GetGuid());
                                materialBatchIter != renderWorld.MaterialBatchLookup.end())
                            {
                                RenderMaterailBatch& batch = renderWorld.MaterialBatch[materialBatchIter->second];
                                meshIsTransparent ? batch.TransparentMeshIndex.push_back(meshIndex) : batch.OpaqueMeshIndex.push_back(meshIndex);
                            }
                            else
                            {
                                RenderMaterailBatch batch;
                                batch.Material.SetMaterial(material);
                                meshIsTransparent ? batch.TransparentMeshIndex.push_back(meshIndex) : batch.OpaqueMeshIndex.push_back(meshIndex);

                                const u64 batchIndex = renderWorld.MaterialBatch.size();
                                renderWorld.MaterialBatch.push_back(batch);
                                renderWorld.MaterialBatchLookup[material->GetGuid()] = batchIndex;
                            }

                            if (meshIsTransparent)
                            {
                                IS_PROFILE_SCOPE("TransparentMeshes.push_back");
                                renderWorld.TransparentMeshIndexs.push_back(meshIndex);
                            }
                            else
                            {
                                IS_PROFILE_SCOPE("Meshes.push_back");
                                renderWorld.OpaqueMeshIndexs.push_back(meshIndex);
                            }
                        }
                    }
                }
                if (entity->HasComponent<ECS::PointLightComponent>())
                {
                    ECS::PointLightComponent* pointLightComponent = entity->GetComponent<ECS::PointLightComponent>();
                    RenderPointLight pointLight;

                    const float nearPlane = 0.01f;
                    const float shadowMapResolution = static_cast<float>(pointLightComponent->GetShadowResolution());

                    const Maths::Matrix4 projection = Maths::Matrix4::CreatePerspective(
                        Maths::DegreesToRadians(90.0f)
                        , shadowMapResolution / shadowMapResolution
                        , nearPlane
                        , std::max(0.1f, pointLightComponent->GetRadius()));

                    pointLight.CreateViewMatrixs(projection, Maths::Vector3(transformComponent->GetPosition()));

                    pointLight.LightColour = pointLightComponent->GetLightColour();
                    pointLight.Position = Maths::Vector3(transformComponent->GetPosition());
                    pointLight.Intensity = pointLightComponent->GetIntensity();
                    pointLight.Radius = pointLightComponent->GetRadius();

                    pointLight.DepthTexture = pointLightComponent->GetShadowMap();

                    {
                        std::lock_guard l(renderWorldMutex);
                        renderWorld.PointLights.push_back(std::move(pointLight));
                    }
                }
                if (entity->HasComponent<ECS::DirectionalLightComponent>())
                {
                    const ECS::DirectionalLightComponent* directionalLightComponent = entity->GetComponent<ECS::DirectionalLightComponent>();
                    const ECS::TransformComponent* transformComponent = directionalLightComponent->GetOwnerEntity()->GetComponent<ECS::TransformComponent>();
                    
                    RenderDirectionalLight directionalLight(
                        Maths::Vector3(transformComponent->GetTransform()[2]), directionalLightComponent->GetLightColour(), directionalLightComponent->GetShadowMap());

                    directionalLight.CreateCascasdes(
                        MainCamera.Camera
                        , transformComponent->GetTransform()
                        , ECS::DirectionalLightComponent::c_cascadeCount
                        , directionalLightComponent->GetCascadeSplitLambda());

                    {
                        std::lock_guard l(renderWorldMutex);
                        renderWorld.DirectionalLights.push_back(std::move(directionalLight));
                    }
                }
#if PARALLEL_FOR
                });
#else
                }
#endif
            RenderWorlds.push_back(std::move(renderWorld));
        }

        SortOpaqueMeshes();
        SortTransparentMeshes();
    }

    void RenderFrame::Sort()
    {
        SortOpaqueMeshes();
        SortTransparentMeshes();
    }

    void RenderFrame::Clear()
    {
        RenderWorlds.clear();
        MainCamera = {};
        MainCamera.Transform = Maths::Matrix4::Zero;
    }

    void RenderFrame::SortOpaqueMeshes()
    {
        IS_PROFILE_FUNCTION();
        for (RenderWorld& world : RenderWorlds)
        {
            if (world.MainCamera.IsSet)
            {
                std::sort(world.OpaqueMeshIndexs.begin(), world.OpaqueMeshIndexs.end(), [&world](u64 a, u64 b)
                    {
                        const RenderMesh& meshA = world.Meshes[a];
                        const RenderMesh& meshB = world.Meshes[b];

                        Maths::Vector3 const& positionA = meshA.Transform[3];
                        Maths::Vector3 const& positionB = meshB.Transform[3];
                        Maths::Vector3 const& cameraPositon = Maths::Vector3(world.MainCamera.Transform[3].x, world.MainCamera.Transform[3].y, world.MainCamera.Transform[3].z);
                        return Maths::Vector3Distance(positionA, cameraPositon) < Maths::Vector3Distance(positionB, cameraPositon);
                    });
            }
        }
    }

    void RenderFrame::SortTransparentMeshes()
    {
        IS_PROFILE_FUNCTION();
        for (RenderWorld& world : RenderWorlds)
        {
            if (world.MainCamera.IsSet)
            {
                IS_PROFILE_SCOPE("Sort transparent meshes");
                std::sort(world.TransparentMeshIndexs.begin(), world.TransparentMeshIndexs.end(), [&world](u64 a, u64 b)
                    {
                        const RenderMesh& meshA = world.Meshes[a];
                        const RenderMesh& meshB = world.Meshes[b];

                        Maths::Vector3 const& positionA = meshA.Transform[3];
                        Maths::Vector3 const& positionB = meshB.Transform[3];
                        Maths::Vector3 const& cameraPositon = Maths::Vector3(world.MainCamera.Transform[3].x, world.MainCamera.Transform[3].y, world.MainCamera.Transform[3].z);
                        return Maths::Vector3Distance(positionA, cameraPositon) < Maths::Vector3Distance(positionB, cameraPositon);
                    });
            }
        }
    }

    void RenderFrame::SetCameraForAllWorlds(ECS::Camera mainCamera, const Maths::Matrix4 transform)
    {
        for (RenderWorld& world : RenderWorlds)
        {
            world.SetMainCamera(mainCamera, transform);
        }
    }

    RenderMesh& RenderFrame::GetRenderMesh(const ECS::Entity* entity)
    {
        return RemoveConst(const_cast<const RenderFrame&>(*this).GetRenderMesh(entity));
    }

    const RenderMesh& RenderFrame::GetRenderMesh(const ECS::Entity* entity) const
    {
        IS_PROFILE_FUNCTION();
        for (const RenderWorld& world : RenderWorlds)
        {
            for (size_t meshIdx = 0; meshIdx < world.Meshes.size(); ++meshIdx)
            {
                const RenderMesh& renderMesh = world.Meshes[meshIdx];
                if (renderMesh.EntityGuid == entity->GetGUID())
                {
                    return renderMesh;
                }
            }
        }
        FAIL_ASSERT();
    }
}