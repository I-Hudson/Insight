#include "Graphics/RenderFrame.h"

#include "World/World.h"
#include "World/WorldSystem.h"

#include "ECS/Components/TransformComponent.h"
#include "ECS/Components/MeshComponent.h"

#include "Core/Profiler.h"

namespace Insight
{
    void RenderMaterial::SetMaterial(const Runtime::Material* material)
    {
        if (!material)
        {
            return;
        }

        for (size_t i = 0; i < static_cast<u64>(Runtime::TextureTypes::Count); ++i)
        {
            Runtime::Texture* texture = material->GetTexture(static_cast<Runtime::TextureTypes>(i));
            if (texture)
            {
                Textures.at(i) = texture->GetRHITexture();
            }
        }
        Properties = material->GetProperties();
    }

    //=====================================================
    // RenderMesh
    //=====================================================
    void RenderMesh::SetMesh(Runtime::Mesh* mesh)
    {
        BoudingBox = mesh->GetBoundingBox();
        MeshLods = mesh->m_lods;
    }

    void RenderMesh::SetMaterial(Runtime::Material* material)
    {
        Material = {};
        Material.SetMaterial(material);
    }

    //=====================================================
    // RenderWorld
    //=====================================================
    void RenderWorld::SetMainCamera(ECS::Camera mainCamera, glm::mat4 transform)
    {
        MainCamera = RenderCamrea{ std::move(mainCamera), std::move(transform), true};
    }

    void RenderWorld::AddCamrea(ECS::Camera camera, glm::mat4 transform)
    {
        Cameras.push_back(RenderCamrea{ std::move(camera), std::move(transform), true });
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
            RenderWorld renderWorld;
            std::vector<Ptr<ECS::Entity>> entities = world->GetAllEntitiesFlatten();
            renderWorld.Meshes.reserve(entities.size());

            std::vector<Ptr<ECS::Entity>> cameraEntities = world->GetAllEntitiesWithComponentByName(ECS::CameraComponent::Type_Name);
            for (Ptr<ECS::Entity>& entity : cameraEntities)
            {
                ECS::TransformComponent* transformComponent = entity->GetComponentByName<ECS::TransformComponent>
                (ECS::TransformComponent::Type_Name);
                ECS::CameraComponent* cameraComponent = entity->GetComponent<ECS::CameraComponent>();
                if (renderWorld.MainCamera.IsSet)
                {
                    renderWorld.AddCamrea(cameraComponent->GetCamera(), transformComponent->GetTransform());
                }
                else
                {
                    renderWorld.SetMainCamera(cameraComponent->GetCamera(), transformComponent->GetTransform());
                }
            }

            for (Ptr<ECS::Entity>& entity : entities)
            {
                IS_PROFILE_SCOPE("Evaluate Single Entity");
                if (!entity->IsEnabled())
                {
                    continue;
                }

                ECS::TransformComponent* transformComponent = entity->GetComponent<ECS::TransformComponent>();

                if (entity->HasComponent<ECS::MeshComponent>())
                {
                    ECS::MeshComponent* meshComponent = entity->GetComponent<ECS::MeshComponent>();
                    if (meshComponent->IsEnabled())
                    {
                        Runtime::Mesh* mesh = meshComponent->GetMesh();
                        if (!mesh)
                        {
                            //continue;
                        }

                        RenderMesh renderMesh;
                        renderMesh.Transform = transformComponent->GetTransform();

                        Graphics::BoundingBox boundingBox = mesh->GetBoundingBox();
                        bool isVisible = true; 
                        {
                            IS_PROFILE_SCOPE("Visible check");
                            boundingBox = boundingBox.Transform(renderMesh.Transform);
                            //isVisible = renderWorld.MainCamera.Camra.IsVisible(boundingBox);
                        }
                        if (!renderWorld.MainCamera.IsSet || !isVisible)
                        {
                            // TODO: Setup seperate scene and game worlds and then re-enable this.
                            // The "EditorWorld" should be it's own world then when play is pressed a runtime world 
                            // should be created.
                            //continue;
                        }

                        Runtime::Material* material = meshComponent->GetMaterial();
                        if (!material)
                        {
                            continue;
                        }

                        renderMesh.SetMesh(mesh);
                        renderMesh.SetMaterial(material);

                        u64 meshIndex = renderWorld.Meshes.size();
                        renderWorld.Meshes.push_back(std::move(renderMesh));
                        bool meshIsTransparent = renderMesh.Material.Properties.at(static_cast<u64>(Runtime::MaterialProperty::Colour_A)) < 1.0f;

                        if (auto materialBatchIter = renderWorld.MaterialBatchLookup.find(material);
                            materialBatchIter != renderWorld.MaterialBatchLookup.end())
                        {
                            RenderMaterailBatch& batch = renderWorld.MaterialBatch.at(materialBatchIter->second);
                            meshIsTransparent ? batch.TransparentMeshIndex.push_back(meshIndex) : batch.OpaqueMeshIndex.push_back(meshIndex);
                        }
                        else
                        {
                            RenderMaterailBatch batch;
                            batch.Material.SetMaterial(material);
                            meshIsTransparent ? batch.TransparentMeshIndex.push_back(meshIndex) : batch.OpaqueMeshIndex.push_back(meshIndex);

                            const u64 batchIndex = renderWorld.MaterialBatch.size();
                            renderWorld.MaterialBatch.push_back(batch);
                            renderWorld.MaterialBatchLookup[material] = batchIndex;
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
                        const RenderMesh& meshA = world.Meshes.at(a);
                        const RenderMesh& meshB = world.Meshes.at(b);

                        glm::vec3 const& positionA = meshA.Transform[3].xyz;
                        glm::vec3 const& positionB = meshB.Transform[3].xyz;
                        glm::vec3 const& cameraPositon = world.MainCamera.Transform[3].xyz;
                        return glm::distance(positionA, cameraPositon) < glm::distance(positionB, cameraPositon);
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
                        const RenderMesh& meshA = world.Meshes.at(a);
                        const RenderMesh& meshB = world.Meshes.at(b);

                        glm::vec3 const& positionA = meshA.Transform[3].xyz;
                        glm::vec3 const& positionB = meshB.Transform[3].xyz;
                        glm::vec3 const& cameraPositon = world.MainCamera.Transform[3].xyz;
                        return glm::distance(positionA, cameraPositon) < glm::distance(positionB, cameraPositon);
                    });
            }
        }
    }
}