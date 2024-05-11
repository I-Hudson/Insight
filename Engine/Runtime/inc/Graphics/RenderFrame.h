#pragma once

#include "Core/TypeAlias.h"

#include "Resource/Mesh.h"
#include "Asset/Assets/Texture.h"
#include "Asset/Assets/Material.h"

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
        void SetMaterial(const Ref<Runtime::MaterialAsset> material);
        std::array<Graphics::RHI_Texture*, static_cast<u64>(Runtime::TextureAssetTypes::Count)> Textures;
        std::array<float, static_cast<u32>(Runtime::MaterialAssetProperty::Count)> Properties;
    };

    struct IS_RUNTIME RenderMesh
    {
        glm::mat4 Transform;
        Graphics::BoundingBox BoudingBox;
        /// @brief All render calls for this mesh.
        std::vector<Runtime::MeshLOD> MeshLods;
        RenderMaterial Material;

        const Runtime::MeshLOD& GetLOD(u32 lodIndex) const;

        void SetMesh(Runtime::Mesh* mesh);
        void SetMaterial(const Ref<Runtime::MaterialAsset> material);
    };

    struct IS_RUNTIME RenderMaterailBatch
    {
        RenderMaterial Material;
        std::vector<u64> OpaqueMeshIndex;
        std::vector<u64> TransparentMeshIndex;
    };

    struct RenderPointLight
    {
        Maths::Matrix4 Projection;
        Maths::Matrix4 View[6];
        Maths::Vector3 LightColour;
        Maths::Vector3 Position;
        float Intensity;
        float Radius;
        Graphics::RHI_Texture* DepthTexture; // In HLSL this is just 8 bytes worth of padding.

        void CreateViewMatrixs(const Maths::Vector3& position)
        {
            for (size_t i = 0; i < 6; i++)
            {
                Maths::Vector3 lightCentre = position;
                Maths::Vector3 upDirection(0, 1, 0);
                switch (i)
                {
                case 0:
                {
                    lightCentre += Maths::Vector3(1, 0, 0);
                    break;
                }
                case 1:
                {
                    lightCentre += Maths::Vector3(-1, 0, 0);
                    break;
                }
                case 2:
                {
                    lightCentre += Maths::Vector3(0, 1, 0);
                    upDirection = Maths::Vector3(0, 0, 1);
                    break;
                }
                case 3:
                {
                    lightCentre += Maths::Vector3(0, -1, 0);
                    upDirection = Maths::Vector3(0, 0, -1);
                    break;
                }
                case 4:
                {
                    lightCentre += Maths::Vector3(0, 0, 1);
                    break;
                }
                case 5:
                {
                    lightCentre += Maths::Vector3(0, 0, -1);
                    break;
                }
                default:
                    break;
                }

                const Maths::Vector3 lightDirection = lightCentre - position;
                const Maths::Matrix4 lightView = Maths::Matrix4::LookAt(position, lightCentre, upDirection);
                View[i] = lightView;
            }
        }
    };

    struct IS_RUNTIME RenderCamera
    {
        ECS::Camera Camera; 
        Maths::Matrix4 Transform;
        bool IsSet = false;
    };

    /// @brief Represent the world for rendering.
    struct IS_RUNTIME RenderWorld
    {
        RenderWorld() = default;
        
        void SetMainCamera(ECS::Camera mainCamera, const Maths::Matrix4 transform);
        void AddCamrea(ECS::Camera camera, const Maths::Matrix4 transform);

        /// @brief The main rendering camera for this world.
        RenderCamera MainCamera;
        /// @brief Addition cameras within the world.
        std::vector<RenderCamera> Cameras;

        /// @brief All meshes within the world.
        std::vector<RenderMesh> Meshes;

        std::vector<RenderPointLight> PointLights;

        std::vector<u64> OpaqueMeshIndexs;
        std::vector<u64> TransparentMeshIndexs;

        std::vector<RenderMaterailBatch> MaterialBatch;
        std::unordered_map<Core::GUID, u64> MaterialBatchLookup;

        glm::vec3 DirectionalLight = glm::vec3(0, 0, 0);
    };

    /// @brief Contain a vector of worlds for rendering.
    struct IS_RUNTIME RenderFrame
    {
        RenderFrame();
        ~RenderFrame();

        std::vector<RenderWorld> RenderWorlds;
        /// @brief The main rendering camera for all render worlds.
        RenderCamera MainCamera;

        /// @brief Create our render frame from the world system.
        /// @param world 
        /// @return RenderWorld
        void CreateRenderFrameFromWorldSystem(Runtime::WorldSystem* worldSystem);
        void Sort();
        void SetCameraForAllWorlds(ECS::Camera mainCamera, const Maths::Matrix4 transform);

    private:
        void Clear();
        void SortOpaqueMeshes();
        void SortTransparentMeshes();
    };
}