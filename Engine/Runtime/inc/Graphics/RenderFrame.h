#pragma once

#include "Core/TypeAlias.h"

#include "Resource/Texture.h"
#include "Resource/Mesh.h"
#include "Resource/Material.h"

#include "ECS/Components/CameraComponent.h"

#include <glm/vec3.hpp>

#include <array>
#include <vector>

namespace Insight
{
    namespace Graphics
    {
        class RHI_Buffer;
        class RHI_Texture;
    }

    namespace Runtime
    {
        class WorldSystem;
        class Mesh;
    }

    struct RenderMaterial
    {
        std::array<Graphics::RHI_Texture*, static_cast<u64>(Runtime::TextureTypes::Count)> Textures;
        std::array<float, static_cast<u32>(Runtime::MaterialProperty::Count)> Properties;
    };

    struct RenderMesh
    {
        glm::mat4 Transform;
        Graphics::BoundingBox BoudingBox;
        /// @brief All render calls for this mesh.
        std::vector<Runtime::MeshLOD> MeshLods;
        RenderMaterial Material;

        void SetMesh(Runtime::Mesh* mesh);
        void SetMaterial(Runtime::Material* material);
    };

    struct RenderCamrea
    {
        ECS::Camera Camra; 
        glm::mat4 Transform;
        bool IsSet = false;
    };

    /// @brief Represent the world for rendering.
    struct RenderWorld
    {
        RenderWorld() = default;
        
        void SetMainCamera(ECS::Camera mainCamera, glm::mat4 transform);
        void AddCamrea(ECS::Camera camera, glm::mat4 transform);

        /// @brief The main rendering camera for this world.
        RenderCamrea MainCamera;
        /// @brief Addition cameras within the world.
        std::vector<RenderCamrea> Cameras;

        /// @brief All meshes within the world.
        std::vector<RenderMesh> Meshes;
        std::vector<RenderMesh> TransparentMeshes;

        glm::vec3 DirectionalLight = glm::vec3(0, 0, 0);
    };

    /// @brief Contain a vector of worlds for rendering.
    struct RenderFrame
    {
        std::vector<RenderWorld> RenderWorlds;

        /// @brief Create our render frame from the world system.
        /// @param world 
        /// @return RenderWorld
        void CreateRenderFrameFromWorldSystem(Runtime::WorldSystem* worldSystem);
        void Sort();

    private:
        void Clear();
        void SortOpaqueMeshes();
        void SortTransparentMeshes();
    };
}