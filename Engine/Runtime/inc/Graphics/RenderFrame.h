#pragma once

#include "Core/TypeAlias.h"
#include "Core/NonCopyable.h"

#include "Resource/Mesh.h"
#include "Asset/Assets/Texture.h"
#include "Asset/Assets/Material.h"

#include "Graphics/RenderContext.h"

#include "ECS/Components/CameraComponent.h"
#include "ECS/Components/DirectionalLightComponent.h"

#include "Maths/Vector3.h"
#include "Maths/Matrix4.h"

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
        Maths::Matrix4 Transform;
        Maths::Matrix4 PreviousTransform;
        Graphics::BoundingBox BoudingBox;
        Core::GUID EntityGuid;

        /// @brief All render calls for this mesh.
        std::vector<Runtime::MeshLOD> MeshLods;
        RenderMaterial Material;

        std::vector<Maths::Matrix4> BoneTransforms;
        Core::GUID SkinnedMeshGuid;
        bool SkinnedMesh = false;

        const Runtime::MeshLOD& GetLOD(u32 lodIndex) const;

        void SetMesh(Runtime::Mesh* mesh);
        void SetMaterial(const Ref<Runtime::MaterialAsset> material);
    };

    struct IS_RUNTIME RenderMaterailBatch
    {
        RenderMaterailBatch()
        {
            OpaqueMeshIndex.reserve(1024);
            TransparentMeshIndex.reserve(1024);
        }

        RenderMaterial Material;
        std::vector<u64> OpaqueMeshIndex;
        std::vector<u64> TransparentMeshIndex;
    };

    struct RenderDirectionalLight
    {
        RenderDirectionalLight() { }
        RenderDirectionalLight(const Maths::Vector3 lightDriection, const Maths::Vector3 lightColour, Graphics::RHI_Texture* depthTexture)
            : LightDriection(lightDriection)
            , LightColour(lightColour)
            , DepthTexture(depthTexture)
        { }

        Maths::Matrix4 ProjectionView[ECS::DirectionalLightComponent::c_cascadeCount];
        float SplitDephts[ECS::DirectionalLightComponent::c_cascadeCount];
        Maths::Vector3 LightDriection;
        Maths::Vector3 LightColour;

        Graphics::RHI_Texture* DepthTexture; // In HLSL this is just 8 bytes worth of padding.

        void CreateCascasdes(const Maths::Matrix4 projView, const float nearPlane, const float farPlane, const u32 cascadeCount, const float splitLambda)
        {
            const float nearClip = nearPlane;
            const float farClip = farPlane;
            const float clipRange = farClip - nearClip;

            const float minZ = nearClip;
            const float maxZ = nearClip + clipRange;

            const float range = maxZ - minZ;
            const float ratio = maxZ / minZ > 0 ? minZ : 1;

            const float cascadeSplitLambda = splitLambda;

            std::vector<float> cascadeSplits;
            cascadeSplits.resize(cascadeCount);

            /// Calculate split depths based on view camera frustum
            /// Based on method presented in https:///developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
            for (u32 i = 0; i < cascadeCount; ++i)
            {
                float p = (static_cast<float>(i) + 1.0f) / static_cast<float>(cascadeCount);
                float log = minZ * std::pow(ratio, p);
                float uniform = minZ + range * p;
                float d = cascadeSplitLambda * (log - uniform) + uniform;
                cascadeSplits[i] = (d - nearClip) / clipRange;
            }

            float lastSplitDist = 0.0;
            for (u32 i = 0; i < cascadeCount; i++)
            {
                float splitDist = cascadeSplits[i];

                Maths::Vector3 frustumCorners[8] =
                {
                    Maths::Vector3(-1.0f,  1.0f, -1.0f),
                    Maths::Vector3(1.0f,  1.0f, -1.0f),
                    Maths::Vector3(1.0f, -1.0f, -1.0f),
                    Maths::Vector3(-1.0f, -1.0f, -1.0f),
                    Maths::Vector3(-1.0f,  1.0f,  1.0f),
                    Maths::Vector3(1.0f,  1.0f,  1.0f),
                    Maths::Vector3(1.0f, -1.0f,  1.0f),
                    Maths::Vector3(-1.0f, -1.0f,  1.0f),
                };

                /// Project frustum corners into world space
                Maths::Matrix4 invCam = projView.Inversed();
                for (u32 i = 0; i < 8; ++i)
                {
                    Maths::Vector4 invCorner = invCam * Maths::Vector4(frustumCorners[i], 1.0f);
                    frustumCorners[i] = Maths::Vector3((invCorner / invCorner.w));
                }

                for (u32 i = 0; i < 4; ++i)
                {
                    Maths::Vector3 dist = frustumCorners[i + 4] - frustumCorners[i];
                    frustumCorners[i + 4] = frustumCorners[i] + (dist * splitDist);
                    frustumCorners[i] = frustumCorners[i] + (dist * lastSplitDist);
                }

                /// Get frustum center
                Maths::Vector3 frustumCenter = Maths::Vector3(0.0f);
                for (u32 i = 0; i < 8; ++i)
                {
                    frustumCenter += frustumCorners[i];
                }
                frustumCenter /= 8.0f;

                float radius = 0.0f;
                for (u32 i = 0; i < 8; ++i)
                {
                    float distance = (frustumCorners[i] - frustumCenter).Length();
                    float glmDistance = Maths::Vector3Distance(Maths::Vector3(frustumCorners[i].x, frustumCorners[i].y, frustumCorners[i].z), Maths::Vector3(frustumCenter.x, frustumCenter.y, frustumCenter.z));
                    radius = std::max(radius, distance);
                }
                radius = std::ceil(radius * 16.0f) / 16.0f;

                Maths::Vector3 maxExtents = Maths::Vector3(radius);
                Maths::Vector3 v;
                v = -v;
                Maths::Vector3 minExtents = -maxExtents;

                Maths::Matrix4 reverse_z = Maths::Matrix4::Identity;
                reverse_z[2][2] = -1;
                reverse_z[2][3] = 1;

                /// Construct our matrixs required for the light.
                Maths::Vector3 lightDirection(0.5f, -0.7f, 0.5f);
                Maths::Vector3 lightPosition = frustumCenter - lightDirection.Normalised() * -minExtents.z;
                Maths::Matrix4 lightViewMatrix = Maths::Matrix4::LookAt(lightPosition, frustumCenter, Maths::Vector3(0.0f, 1.0f, 0.0f));
                Maths::Matrix4 lightOrthoMatrix = Maths::Matrix4::CreateOrthographic(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f, maxExtents.z - minExtents.z);

                if (Graphics::RenderContext::Instance().IsRenderOptionsEnabled(Graphics::RenderOptions::ReverseZ))
                {
                    Maths::Matrix4 proj = Maths::Matrix4::CreateOrthographic(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, maxExtents.z - minExtents.z, 0.0f);
                    //glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, maxExtents.z - minExtents.z, 0.0f);
                    lightOrthoMatrix = reverse_z * lightOrthoMatrix;
                    lightOrthoMatrix = proj;
                }

                {
                    if (Graphics::RenderContext::Instance().GetGraphicsAPI() == Graphics::GraphicsAPI::Vulkan)
                    {
                        /// Invert the projection if vulkan. This is because vulkan's coord space is from top left, not bottom left.
                        lightOrthoMatrix[1][1] *= -1;
                    }

                    /// Store split distance and matrix in cascade
                    ProjectionView[i] = lightOrthoMatrix * lightViewMatrix;
                    SplitDephts[i] = (nearPlane + splitDist * clipRange) * -1.0f;
                    //outCascades.Projection[i] = lightOrthoMatrix;
                    //outCascades.View[i] = lightViewMatrix;
                }
                lastSplitDist = cascadeSplits[i];
            }
        }
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
                    // +X
                    lightCentre += Maths::Vector3(1, 0, 0);
                    break;
                }
                case 1:
                {
                    // -X
                    lightCentre += Maths::Vector3(-1, 0, 0);
                    break;
                }
                case 2:
                {
                    // +Y
                    lightCentre += Maths::Vector3(0, 1, 0);
                    upDirection = Maths::Vector3(0, 0, -1);
                    break;
                }
                case 3:
                {
                    // -Y
                    lightCentre += Maths::Vector3(0, -1, 0);
                    upDirection = Maths::Vector3(0, 0, 1);
                    break;
                }
                case 4:
                {
                    // +Z
                    lightCentre += Maths::Vector3(0, 0, 1);
                    break;
                }
                case 5:
                {
                    // -Z
                    lightCentre += Maths::Vector3(0, 0, -1);
                    break;
                }
                default:
                    break;
                }

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
        std::vector<RenderDirectionalLight> DirectionalLights;

        std::vector<u64> OpaqueMeshIndexs;
        std::vector<u64> TransparentMeshIndexs;

        std::vector<RenderMaterailBatch> MaterialBatch;
        std::unordered_map<Core::GUID, u64> MaterialBatchLookup;

        Maths::Vector3 DirectionalLight = Maths::Vector3(0, 0, 0);
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

        RenderMesh& GetRenderMesh(const ECS::Entity* entity);
        const RenderMesh& GetRenderMesh(const ECS::Entity* entity) const;

    private:
        void Clear();
        void SortOpaqueMeshes();
        void SortTransparentMeshes();
    };
}