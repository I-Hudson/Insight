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

    struct IS_RUNTIME RenderMaterial
    {
        void SetMaterial(const Runtime::Material* material);
        std::array<Graphics::RHI_Texture*, static_cast<u64>(Runtime::TextureTypes::Count)> Textures;
        std::array<float, static_cast<u32>(Runtime::MaterialProperty::Count)> Properties;
    };

    struct IS_RUNTIME RenderMesh
    {
        glm::mat4 Transform;
        Graphics::BoundingBox BoudingBox;
        /// @brief All render calls for this mesh.
        std::vector<Runtime::MeshLOD> MeshLods;
        RenderMaterial Material;

        void SetMesh(Runtime::Mesh* mesh);
        void SetMaterial(Runtime::Material* material);
    };

    struct IS_RUNTIME RenderMaterailBatch
    {
        RenderMaterial Material;
        std::vector<u64> OpaqueMeshIndex;
        std::vector<u64> TransparentMeshIndex;
    };

    struct IS_RUNTIME RenderCamrea
    {
        ECS::Camera Camra; 
        glm::mat4 Transform;
        bool IsSet = false;
    };

    /// @brief Represent the world for rendering.
    struct IS_RUNTIME RenderWorld
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

        std::vector<u64> OpaqueMeshIndexs;
        std::vector<u64> TransparentMeshIndexs;

        std::vector<RenderMaterailBatch> MaterialBatch;
        std::unordered_map<Runtime::Material*, u64> MaterialBatchLookup;

        glm::vec3 DirectionalLight = glm::vec3(0, 0, 0);
    };

    /// @brief Contain a vector of worlds for rendering.
    struct IS_RUNTIME RenderFrame
    {
        std::vector<RenderWorld> RenderWorlds;
        /// @brief The main rendering camera for all render worlds.
        RenderCamrea MainCamera;

        /// @brief Create our render frame from the world system.
        /// @param world 
        /// @return RenderWorld
        void CreateRenderFrameFromWorldSystem(Runtime::WorldSystem* worldSystem);
        void Sort();
        void SetCameraForAllWorlds(ECS::Camera mainCamera, glm::mat4 transform);

    private:
        void Clear();
        void SortOpaqueMeshes();
        void SortTransparentMeshes();
    };
}