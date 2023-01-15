#include "Graphics/RenderFrame.h"

#include "World/World.h"
#include "World/WorldSystem.h"

#include "ECS/Components/TransformComponent.h"
#include "ECS/Components/MeshComponent.h"

#include "Core/Profiler.h"

namespace Insight
{
    //=====================================================
    // RenderMesh
    //=====================================================
    void RenderMesh::SetMesh(Runtime::Mesh* mesh)
    {
        MeshLods = mesh->m_lods;
    }

    void RenderMesh::SetMaterial(Runtime::Material* material)
    {
        Material = {};
        if (!material)
        {
            return;
        }

        for (size_t i = 0; i < static_cast<u64>(Runtime::TextureTypes::Count); ++i)
        {
            Runtime::Texture* texture = material->GetTexture(static_cast<Runtime::TextureTypes>(i));
            if (texture)
            {
                Material.Textures.at(i) = texture->GetRHITexture();
            }
        }
        Material.Properties = material->GetProperties();
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

    RenderFrame CreateRenderFrameFromWorldSystem(Runtime::WorldSystem* worldSystem)
    {
        IS_PROFILE_FUNCTION();
        ASSERT(Platform::IsMainThread());

        RenderFrame renderFrame;
        std::vector<TObjectPtr<Runtime::World>> worlds = worldSystem->GetAllWorlds();
        for (TObjectPtr<Runtime::World> const& world : worlds)
        {
            IS_PROFILE_SCOPE("RenderWorld");
            RenderWorld renderWorld;
            std::vector<Ptr<ECS::Entity>> entities = world->GetAllEntitiesFlatten();

            bool foundMainCamera = false;

            for (Ptr<ECS::Entity>& entity : entities)
            {
                IS_PROFILE_SCOPE("Evaluate Single Entity");
                if (!entity->IsEnabled())
                {
                    continue;
                }

                ECS::TransformComponent* transformComponent = entity->GetComponentByName<ECS::TransformComponent>(ECS::TransformComponent::Type_Name);
                if (entity->HasComponentByName(ECS::CameraComponent::Type_Name))
                {
                    ECS::CameraComponent* cameraComponent = entity->GetComponent<ECS::CameraComponent>();
                    if (foundMainCamera)
                    {
                        renderWorld.AddCamrea(cameraComponent->GetCamera(), transformComponent->GetTransform());
                    }
                    else
                    {
                        renderWorld.SetMainCamera(cameraComponent->GetCamera(), transformComponent->GetTransform());
                        foundMainCamera = true;
                    }
                }

                if (entity->HasComponent<ECS::MeshComponent>())
                {
                    ECS::MeshComponent* meshComponent = entity->GetComponent<ECS::MeshComponent>();
                    if (meshComponent->IsEnabled())
                    {
                        Runtime::Mesh* mesh = meshComponent->GetMesh();

                        RenderMesh renderMesh;
                        renderMesh.Transform = transformComponent->GetTransform();
                        renderMesh.SetMesh(mesh);
                        renderMesh.SetMaterial(meshComponent->GetMaterial());

                        if (renderMesh.Material.Properties.at(static_cast<u64>(Runtime::MaterialProperty::Colour_A)) < 1.0f)
                        {
                            renderWorld.TransparentMeshes.push_back(std::move(renderMesh));
                        }
                        else
                        {
                            renderWorld.Meshes.push_back(std::move(renderMesh));
                        }
                    }
                }
            }
            renderFrame.RenderWorlds.push_back(std::move(renderWorld));
        }
        renderFrame.SortTransparentMeshes();
        return renderFrame;
    }

    void RenderFrame::SortTransparentMeshes()
    {
        IS_PROFILE_FUNCTION();
        for (RenderWorld& world : RenderWorlds)
        {
            if (world.MainCamera.IsSet)
            {
                IS_PROFILE_SCOPE("Sort transparent meshes");
                std::sort(world.TransparentMeshes.begin(), world.TransparentMeshes.end(), [&world](RenderMesh const& meshA, RenderMesh const& meshB)
                    {
                        glm::vec3 const& positionA = meshA.Transform[3].xyz;
                        glm::vec3 const& positionB = meshB.Transform[3].xyz;
                        glm::vec3 const& cameraPositon = world.MainCamera.Transform[3].xyz;
                        return glm::distance(positionA, cameraPositon) < glm::distance(positionB, cameraPositon);
                    });
            }
        }
    }
}