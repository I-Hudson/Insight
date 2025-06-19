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
    private:
        /*
        struct BoundingBox
        {
            static constexpr size_t CORNER_COUNT = 8;

            void CreateFromPoints(const Maths::Vector4 aabbMin, const Maths::Vector4 aabbMax)
            {
                const Maths::Vector4 min = Maths::Min(aabbMin, aabbMax);
                const Maths::Vector4 max = Maths::Max(aabbMin, aabbMax);

                Centre = (min + max) * 0.5f;
                Extents = (max - min) * 0.5f;
            }

            void GetCorners(Maths::Vector3* corners) const
            {
                ASSERT(corners != nullptr);

                // Load the box
                Maths::Vector4 vCenter = Maths::Vector4(Centre, 0.0f);
                Maths::Vector4 vExtents = Maths::Vector4(Extents, 0.0f);

                const Maths::Vector4 g_BoxOffset[8] =
                {
                    { -1.0f, -1.0f,  1.0f, 0.0f },
                    {  1.0f, -1.0f,  1.0f, 0.0f },
                    {  1.0f,  1.0f,  1.0f, 0.0f },
                    { -1.0f,  1.0f,  1.0f, 0.0f },
                    { -1.0f, -1.0f, -1.0f, 0.0f },
                    {  1.0f, -1.0f, -1.0f, 0.0f },
                    {  1.0f,  1.0f, -1.0f, 0.0f },
                    { -1.0f,  1.0f, -1.0f, 0.0f },
                };

                for (size_t i = 0; i < CORNER_COUNT; ++i)
                {
                    Maths::Vector4 C = (vExtents* g_BoxOffset[i]) + vCenter;
                    corners[i] = C;
                }
            }

            Maths::Vector3 Centre;
            Maths::Vector3 Extents;
        };

        struct Frustum
        {
            Frustum() {}
            void CreateFromProjection(const Maths::Matrix4& projection)
            {
                {
                    // Corners of the projection frustum in NDC space.
                    static Maths::Vector4 NDCPoints[6] =
                    {
                        {  1.0f,  0.0f, 1.0f, 1.0f },   // right (at far plane)
                        { -1.0f,  0.0f, 1.0f, 1.0f },   // left
                        {  0.0f,  1.0f, 1.0f, 1.0f },   // top
                        {  0.0f, -1.0f, 1.0f, 1.0f },   // bottom

                        { 0.0f, 0.0f, 0.0f, 1.0f },     // near
                        { 0.0f, 0.0f, 1.0f, 1.0f }      // far
                    };

                    const Maths::Matrix4 projInv = projection.Inversed();

                    // Compute the frustum corners in world space.
                    Maths::Vector4 Points[6];

                    for (size_t i = 0; i < 6; ++i)
                    {
                        // Transform point.
                        Points[i] = projInv * NDCPoints[i];
                    }

                    Origin = Maths::Vector3(0.0f, 0.0f, 0.0f);
                    Orientation = Maths::Vector4(0.0f, 0.0f, 0.0f, 1.0f);

                    // Compute the slopes.
                    Points[0] = Points[0] * Maths::Vector4(Points[0].z).Reciprocal();
                    Points[1] = Points[1] * Maths::Vector4(Points[1].z).Reciprocal();
                    Points[2] = Points[2] * Maths::Vector4(Points[2].z).Reciprocal();
                    Points[3] = Points[3] * Maths::Vector4(Points[3].z).Reciprocal();

                    RightSlope = Points[0].x;
                    LeftSlope = Points[1].x;
                    TopSlope = Points[2].y;
                    BottomSlope = Points[3].y;

                    // Compute near and far.
                    Points[4] = Points[4] * Maths::Vector4(Points[4].w).Reciprocal();
                    Points[5] = Points[5] * Maths::Vector4(Points[5].w).Reciprocal();

                    Near = Points[4].z;
                    Far = Points[5].z;
                }

                {
                    Frustum Out;

                    // Corners of the projection frustum in NDC space.
                    static DirectX::XMVECTORF32 NDCPoints[6] =
                    {
                        { { {  1.0f,  0.0f, 1.0f, 1.0f } } },   // right (at far plane)
                        { { { -1.0f,  0.0f, 1.0f, 1.0f } } },   // left
                        { { {  0.0f,  1.0f, 1.0f, 1.0f } } },   // top
                        { { {  0.0f, -1.0f, 1.0f, 1.0f } } },   // bottom

                        { { { 0.0f, 0.0f, 0.0f, 1.0f } } },     // near
                        { { { 0.0f, 0.0f, 1.0f, 1.0f } } }      // far
                    };

                    DirectX::XMVECTOR Determinant;
                    DirectX::XMMATRIX matInverse = DirectX::XMMatrixInverse(&Determinant, projection.xmmatrix);

                    // Compute the frustum corners in world space.
                    DirectX::XMVECTOR Points[6];

                    for (size_t i = 0; i < 6; ++i)
                    {
                        // Transform point.
                        Points[i] = XMVector4Transform(NDCPoints[i], matInverse);
                    }

                    // Compute the slopes.
                    Points[0] = DirectX::XMVectorMultiply(Points[0], DirectX::XMVectorReciprocal(DirectX::XMVectorSplatZ(Points[0])));
                    Points[1] = DirectX::XMVectorMultiply(Points[1], DirectX::XMVectorReciprocal(DirectX::XMVectorSplatZ(Points[1])));
                    Points[2] = DirectX::XMVectorMultiply(Points[2], DirectX::XMVectorReciprocal(DirectX::XMVectorSplatZ(Points[2])));
                    Points[3] = DirectX::XMVectorMultiply(Points[3], DirectX::XMVectorReciprocal(DirectX::XMVectorSplatZ(Points[3])));

                    Out.RightSlope = DirectX::XMVectorGetX(Points[0]);
                    Out.LeftSlope = DirectX::XMVectorGetX(Points[1]);
                    Out.TopSlope = DirectX::XMVectorGetY(Points[2]);
                    Out.BottomSlope = DirectX::XMVectorGetY(Points[3]);

                    // Compute near and far.
                    Points[4] = DirectX::XMVectorMultiply(Points[4], DirectX::XMVectorReciprocal(DirectX::XMVectorSplatW(Points[4])));
                    Points[5] = DirectX::XMVectorMultiply(Points[5], DirectX::XMVectorReciprocal(DirectX::XMVectorSplatW(Points[5])));

                    Out.Near = DirectX::XMVectorGetZ(Points[4]);
                    Out.Far = DirectX::XMVectorGetZ(Points[5]);
                }
            }

            const size_t CORNER_COUNT = 8;

            Maths::Vector3 Origin;
            Maths::Vector4 Orientation;

            float RightSlope;           // Positive X (X/Z)
            float LeftSlope;            // Negative X
            float TopSlope;             // Positive Y (Y/Z)
            float BottomSlope;          // Negative Y
            float Near, Far;            // Z of the near plane and far plane.
        };
*/

        public:
        RenderDirectionalLight() { }
        RenderDirectionalLight(const Maths::Vector3 lightDriection, const Maths::Vector3 lightColour, Graphics::RHI_Texture* depthTexture)
            : LightDriection(lightDriection)
            , LightColour(lightColour)
            , DepthTexture(depthTexture)
        {
            Platform::MemSet(SplitDephts, 0, sizeof(float) * ECS::DirectionalLightComponent::c_cascadeCount);
        }

        Maths::Matrix4 ProjectionView[ECS::DirectionalLightComponent::c_cascadeCount + 1];
        float SplitDephts[ECS::DirectionalLightComponent::c_cascadeCount];
        Maths::Vector3 LightDriection;
        Maths::Vector3 LightColour;

        Graphics::RHI_Texture* DepthTexture = nullptr; // In HLSL this is just 8 bytes worth of padding.

        void CreateCascasdes(const ECS::Camera& mainCamera, const Maths::Matrix4& ligthViewMatrix, u32 cascadeCount, const float splitLambda)
        {
#if 1
            const float nearClip = mainCamera.GetNearPlane();
            const float farClip = mainCamera.GetFarPlane();
            const float clipRange = farClip - nearClip;

            const float minZ = nearClip;
            const float maxZ = nearClip + clipRange;

            const float range = maxZ - minZ;
            const float ratio = maxZ / minZ > 0 ? minZ : 1;

            const float cascadeSplitLambda = splitLambda;

#if 0
            cascadeCount += 1;
#endif
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
#if 0
                if (i == 0)
                {
                    ProjectionView[i] = GetLightProjViewForCascade(mainCamera, mainCamera.GetNearPlane(), -SplitDephts[i]);
                }
                else if(i < cascadeCount - 1)
                {
                    ProjectionView[i] = GetLightProjViewForCascade(mainCamera, -SplitDephts[i - 1], -SplitDephts[i]);
                }
                else
                {
                    ProjectionView[i] = GetLightProjViewForCascade(mainCamera, -SplitDephts[i], mainCamera.GetFarPlane());
                }
            }

            return;
#endif

#if 0
            const float cascadeDistances[4] =
            {
                cascadeSplits[0],
                cascadeSplits[1],
                cascadeSplits[2],
                cascadeSplits[3],
            };

                Maths::Matrix4 dxProjView[4];
            
                const Maths::Quaternion dxlightRotationQ(LightDriection.x, LightDriection.y, LightDriection.z);
                DirectX::XMVECTOR dxLightQ;
                dxLightQ.m128_f32[0] = dxlightRotationQ.w;
                dxLightQ.m128_f32[1] = dxlightRotationQ.x;
                dxLightQ.m128_f32[2] = dxlightRotationQ.y;
                dxLightQ.m128_f32[3] = dxlightRotationQ.z;

                const DirectX::XMMATRIX dxlightRotation = DirectX::XMMatrixRotationQuaternion(dxLightQ);
                const DirectX::XMVECTOR dxto = DirectX::XMVector3TransformNormal(DirectX::XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f), dxlightRotation);
                const DirectX::XMVECTOR dxup = DirectX::XMVector3TransformNormal(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), dxlightRotation);
                const DirectX::XMMATRIX dxlightView = DirectX::XMMatrixLookToLH(DirectX::XMVectorZero(), dxto, dxup); // important to not move (zero out eye vector) the light view matrix itself because texel snapping must be done on projection matrix!
                const float dxfarPlane = mainCamera.GetFarPlane();

                // Unproject main frustum corners into world space (notice the reversed Z projection!):
                const DirectX::XMMATRIX dxunproj = mainCamera.GetInvertedProjectionViewMatrix().xmmatrix;
                const DirectX::XMVECTOR dxfrustum_corners[] =
                {
                    DirectX::XMVector3TransformCoord(DirectX::XMVectorSet(-1, -1, 1, 1), dxunproj),	// near
                    DirectX::XMVector3TransformCoord(DirectX::XMVectorSet(-1, -1, 0, 1), dxunproj),	// far
                    DirectX::XMVector3TransformCoord(DirectX::XMVectorSet(-1, 1, 1, 1), dxunproj),	// near
                    DirectX::XMVector3TransformCoord(DirectX::XMVectorSet(-1, 1, 0, 1), dxunproj),	// far
                    DirectX::XMVector3TransformCoord(DirectX::XMVectorSet(1, -1, 1, 1), dxunproj),	// near
                    DirectX::XMVector3TransformCoord(DirectX::XMVectorSet(1, -1, 0, 1), dxunproj),	// far
                    DirectX::XMVector3TransformCoord(DirectX::XMVectorSet(1, 1, 1, 1), dxunproj),	// near
                    DirectX::XMVector3TransformCoord(DirectX::XMVectorSet(1, 1, 0, 1), dxunproj),	// far
                };

                // Compute shadow cameras:
                for (int cascade = 0; cascade < cascadeCount; ++cascade)
                {
                    // Compute cascade bounds in light-view-space from the main frustum corners:
                    const float split_near = cascade == 0 ? 0 : cascadeDistances[cascade - 1];
                    const float split_far = cascadeDistances[cascade];
                    const DirectX::XMVECTOR dxcorners[] =
                    {
                        DirectX::XMVector3Transform(DirectX::XMVectorLerp(dxfrustum_corners[0], dxfrustum_corners[1], split_near), dxlightView),
                        DirectX::XMVector3Transform(DirectX::XMVectorLerp(dxfrustum_corners[0], dxfrustum_corners[1], split_far), dxlightView),
                        DirectX::XMVector3Transform(DirectX::XMVectorLerp(dxfrustum_corners[2], dxfrustum_corners[3], split_near), dxlightView),
                        DirectX::XMVector3Transform(DirectX::XMVectorLerp(dxfrustum_corners[2], dxfrustum_corners[3], split_far), dxlightView),
                        DirectX::XMVector3Transform(DirectX::XMVectorLerp(dxfrustum_corners[4], dxfrustum_corners[5], split_near), dxlightView),
                        DirectX::XMVector3Transform(DirectX::XMVectorLerp(dxfrustum_corners[4], dxfrustum_corners[5], split_far), dxlightView),
                        DirectX::XMVector3Transform(DirectX::XMVectorLerp(dxfrustum_corners[6], dxfrustum_corners[7], split_near), dxlightView),
                        DirectX::XMVector3Transform(DirectX::XMVectorLerp(dxfrustum_corners[6], dxfrustum_corners[7], split_far), dxlightView),
                    };

                    // Compute cascade bounding sphere center:
                    DirectX::XMVECTOR dxcenter = DirectX::XMVectorZero();
                    for (int j = 0; j < ARRAY_COUNT(dxcorners); ++j)
                    {
                        dxcenter = DirectX::XMVectorAdd(dxcenter, dxcorners[j]);
                    }
                    dxcenter = DirectX::XMVectorDivide(dxcenter, DirectX::XMVectorReplicate(ARRAY_COUNT(dxcorners)));

                    // Compute cascade bounding sphere radius:
                    float dxradius = 0;
                    for (int j = 0; j < ARRAY_COUNT(dxcorners); ++j)
                    {
                        dxradius = Maths::Max(dxradius, DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(dxcorners[j], dxcenter))));
                    }

                    // Fit AABB onto bounding sphere:
                    DirectX::XMVECTOR dxvRadius = DirectX::XMVectorReplicate(dxradius);
                    DirectX::XMVECTOR dxvMin = DirectX::XMVectorSubtract(dxcenter, dxvRadius);
                    DirectX::XMVECTOR dxvMax = DirectX::XMVectorAdd(dxcenter, dxvRadius);

                    /*
                    // Snap cascade to texel grid:
                    const DirectX::XMVECTOR extent = DirectX::XMVectorSubtract(vMax, vMin);
                    const DirectX::XMVECTOR texelSize = extent / float(shadow_rect.w);
                    vMin = DirectX::XMVectorFloor(vMin / texelSize) * texelSize;
                    vMax = DirectX::XMVectorFloor(vMax / texelSize) * texelSize;
                    center = (vMin + vMax) * 0.5f;
                    */

                    DirectX::XMFLOAT3 dx_center;
                    DirectX::XMFLOAT3 dx_min;
                    DirectX::XMFLOAT3 dx_max;
                    XMStoreFloat3(&dx_center, dxcenter);
                    XMStoreFloat3(&dx_min, dxvMin);
                    XMStoreFloat3(&dx_max, dxvMax);

                    // Extrude bounds to avoid early shadow clipping:
                    float dxext = abs(dx_center.z - dx_min.z);
                    dxext = Maths::Max(dxext, Maths::Min(1500.0f, dxfarPlane) * 0.5f);
                    dx_min.z = dx_center.z - dxext;
                    dx_max.z = dx_center.z + dxext;

                    const DirectX::XMMATRIX lightProjection = DirectX::XMMatrixOrthographicOffCenterLH(dx_min.x, dx_max.x, dx_min.y, dx_max.y, dx_min.z, dx_max.z); // notice reversed Z!

                    dxProjView[cascade] = DirectX::XMMatrixMultiply(dxlightView, lightProjection);
                }
            
            
            const Maths::Quaternion lightRotationQ(LightDriection.x, LightDriection.y, LightDriection.z);
            const Maths::Matrix4 lightRotation(lightRotationQ);
            const Maths::Vector3 to = Maths::Vector3TransformFromNormal(lightRotation, Maths::Vector3(0.0f, -1.0f, 0.0f));
            const Maths::Vector3 up = Maths::Vector3TransformFromNormal(lightRotation, Maths::Vector3(0.0f, 0.0f, 1.0f));
            const Maths::Matrix4 lightView = Maths::Matrix4::LookAt(Maths::Vector3::Zero, to, up); // important to not move (zero out eye vector) the light view matrix itself because texel snapping must be done on projection matrix!

            const float farPlane = mainCamera.GetFarPlane();

            // Unproject main frustum corners into world space (notice the reversed Z projection!):
            const Maths::Matrix4 unproj = mainCamera.GetInvertedProjectionViewMatrix();
            const Maths::Vector3 frustum_corners[] =
            {
                Maths::Vector3TransformCoord(Maths::Vector4(-1.0f, -1.0f, 1.0f, 1.0f), unproj),	// near
                Maths::Vector3TransformCoord(Maths::Vector4(-1.0f, -1.0f, 0.0f, 1.0f), unproj),	// far
                Maths::Vector3TransformCoord(Maths::Vector4(-1.0f,  1.0f, 1.0f, 1.0f), unproj),	// near
                Maths::Vector3TransformCoord(Maths::Vector4(-1.0f,  1.0f, 0.0f, 1.0f), unproj),	// far
                Maths::Vector3TransformCoord(Maths::Vector4( 1.0f, -1.0f, 1.0f, 1.0f), unproj),	// near
                Maths::Vector3TransformCoord(Maths::Vector4( 1.0f, -1.0f, 0.0f, 1.0f), unproj),	// far
                Maths::Vector3TransformCoord(Maths::Vector4( 1.0f,  1.0f, 1.0f, 1.0f), unproj),	// near
                Maths::Vector3TransformCoord(Maths::Vector4( 1.0f,  1.0f, 0.0f, 1.0f), unproj),	// far
            };

            // Compute shadow cameras:
            for (int cascade = 0; cascade < cascadeCount; ++cascade)
            {
                // Compute cascade bounds in light-view-space from the main frustum corners:
                const float split_near = cascade == 0 ? 0 : cascadeDistances[cascade - 1];
                const float split_far = cascadeDistances[cascade];
                const Maths::Vector4 corners[] =
                {
                    Maths::Vector3Transform(Maths::Lerp(frustum_corners[0], frustum_corners[1], split_near), lightView),
                    Maths::Vector3Transform(Maths::Lerp(frustum_corners[0], frustum_corners[1], split_far), lightView),
                    Maths::Vector3Transform(Maths::Lerp(frustum_corners[2], frustum_corners[3], split_near), lightView),
                    Maths::Vector3Transform(Maths::Lerp(frustum_corners[2], frustum_corners[3], split_far), lightView),
                    Maths::Vector3Transform(Maths::Lerp(frustum_corners[4], frustum_corners[5], split_near), lightView),
                    Maths::Vector3Transform(Maths::Lerp(frustum_corners[4], frustum_corners[5], split_far), lightView),
                    Maths::Vector3Transform(Maths::Lerp(frustum_corners[6], frustum_corners[7], split_near), lightView),
                    Maths::Vector3Transform(Maths::Lerp(frustum_corners[6], frustum_corners[7], split_far), lightView),
                };

                // Compute cascade bounding sphere center:
                Maths::Vector4 center = Maths::Vector4::Zero;
                for (int j = 0; j < ARRAY_COUNT(corners); ++j)
                {
                    center = center + corners[j];
                }
                center = center / float(ARRAY_COUNT(corners));

                // Compute cascade bounding sphere radius:
                float radius = 0;
                for (int j = 0; j < ARRAY_COUNT(corners); ++j)
                {
                    radius = Maths::Max(radius, (Maths::Vector3(corners[j] - center)).Length());
                }

                // Fit AABB onto bounding sphere:
                Maths::Vector4 vRadius = Maths::Vector4(radius);
                Maths::Vector4 vMin = center - vRadius;
                Maths::Vector4 vMax = center + vRadius;

                // Snap cascade to texel grid:
                /*
                const Maths::Vector4 extent = vMax - vMin;
                const Maths::Vector4 texelSize = extent / float(shadow_rect.w);
                vMin = (vMin / texelSize).Floor() * texelSize;
                vMax = (vMax / texelSize).Floor() * texelSize;
                center = (vMin + vMax) * 0.5f;
                */

                Maths::Vector3 _center = center;
                Maths::Vector3 _min = vMin;
                Maths::Vector3 _max = vMax;

                // Extrude bounds to avoid early shadow clipping:
                float ext = abs(_center.z - _min.z);
                ext = Maths::Max(ext, Maths::Min(1500.0f, farPlane) * 0.5f);
                _min.z = _center.z - ext;
                _max.z = _center.z + ext;

                const Maths::Matrix4 lightProjection = Maths::Matrix4::CreateOrthographic(_min.x, _max.x, _min.y, _max.y, _min.z, _max.z);

                ProjectionView[cascade] = lightProjection * lightView;
                SplitDephts[cascade] = (mainCamera.GetNearPlane() + cascadeSplits[cascade] * clipRange) * -1.0f;;
#endif
            
            float lastSplitDist = 0.0;
            for (u32 i = 0; i < cascadeCount; i++)
            {
                float splitDist = cascadeSplits[i];

                Maths::Vector3 frustumCorners[8] =
                {
                    Maths::Vector3(-1.0f,  1.0f, 0.0f),
                    Maths::Vector3( 1.0f,  1.0f, 0.0f),
                    Maths::Vector3( 1.0f, -1.0f, 0.0f),
                    Maths::Vector3(-1.0f, -1.0f, 0.0f),
                    Maths::Vector3(-1.0f,  1.0f, 1.0f),
                    Maths::Vector3( 1.0f,  1.0f, 1.0f),
                    Maths::Vector3( 1.0f, -1.0f, 1.0f),
                    Maths::Vector3(-1.0f, -1.0f, 1.0f),
                };

                /// Project frustum corners into world space
                Maths::Matrix4 invCam = mainCamera.GetProjectionViewMatrix().Inversed();
                for (u32 cornerIdx = 0; cornerIdx < 8; ++cornerIdx)
                {
                    Maths::Vector4 invCorner = invCam * Maths::Vector4(frustumCorners[cornerIdx], 1.0f);
                    frustumCorners[cornerIdx] = invCorner / invCorner.w;
                }

                for (u32 cornerIdx = 0; cornerIdx < 4; ++cornerIdx)
                {
                    Maths::Vector3 dist = frustumCorners[cornerIdx + 4] - frustumCorners[cornerIdx];
                    frustumCorners[cornerIdx + 4] = frustumCorners[cornerIdx] + (dist * splitDist);
                    frustumCorners[cornerIdx] = frustumCorners[cornerIdx] + (dist * lastSplitDist);
                }

                /// Get frustum center
                Maths::Vector3 frustumCenter = Maths::Vector3(0.0f);
                for (u32 i = 0; i < 8; ++i)
                {
                    frustumCenter += frustumCorners[i];
                }
                frustumCenter /= 8.0f;

                float radius = 0.0f;
                for (u32 cornerIdx = 0; cornerIdx < 8; ++cornerIdx)
                {
                    const float distance = (frustumCorners[cornerIdx] - frustumCenter).Length();
                    const float glmDistance = Maths::Vector3Distance(Maths::Vector3(frustumCorners[i].x, frustumCorners[i].y, frustumCorners[i].z), Maths::Vector3(frustumCenter.x, frustumCenter.y, frustumCenter.z));
                    radius = std::max(radius, distance);
                }
                radius = std::ceil(radius * 16.0f) / 16.0f;

                Maths::Vector3 maxExtents = Maths::Vector3(radius);
                Maths::Vector3 minExtents = -maxExtents;

                /// Construct our matrixs required for the light.
                const Maths::Vector3 lightDir =  (-LightDriection).Normalised();
                const Maths::Vector4 newLightPos = frustumCenter - lightDir * -minExtents;
                const Maths::Matrix4 newLightViewMatrix = Maths::Matrix4::LookAt(newLightPos, frustumCenter, Maths::Vector3(0.0f, 1.0f, 0.0f));
                const Maths::Matrix4 newLightOrthoMatrix = Maths::Matrix4::CreateOrthographic(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f, maxExtents.z - minExtents.z);

                const Maths::Vector3 lightPosition = frustumCenter + LightDriection.Normalised() * -minExtents.z;
                const Maths::Matrix4 lightViewMatrix = Maths::Matrix4::LookAt(lightPosition, frustumCenter, Maths::Vector3(0.0f, 1.0f, 0.0f));
                Maths::Matrix4 lightOrthoMatrix = Maths::Matrix4::CreateOrthographic(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f, maxExtents.z - minExtents.z);

                if (Graphics::RenderContext::Instance().IsRenderOptionsEnabled(Graphics::RenderOptions::ReverseZ))
                {
                    Maths::Matrix4 proj = Maths::Matrix4::CreateOrthographic(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, maxExtents.z - minExtents.z, 0.0f);
                    //glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, maxExtents.z - minExtents.z, 0.0f);
                    Maths::Matrix4 reverse_z = Maths::Matrix4::Identity;
                    reverse_z[2][2] = -1;
                    reverse_z[2][3] = 1;

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
                    SplitDephts[i] = (mainCamera.GetNearPlane() + splitDist * clipRange) * -1.0f;
                    ProjectionView[i] = lightOrthoMatrix * lightViewMatrix.Inversed();
                }
                lastSplitDist = cascadeSplits[i];
            }
#endif
        }

        private:
            /*
            void CreateFrustumPointsFromCascadeInterval(const float cascadeIntervalBegin, const float cascadeIntervalEnd, const Maths::Matrix4& projection, Maths::Vector4* cornerPointsWorld)
            {
                {
                    Frustum viewFrustum;
                    viewFrustum.CreateFromProjection(projection);

                    viewFrustum.Near = cascadeIntervalBegin;
                    viewFrustum.Far = cascadeIntervalEnd;

                    static const Maths::Vector4 vGrabY = { 0x00000000,(float)0xFFFFFFFF,0x00000000,0x00000000 };
                    static const Maths::Vector4 vGrabX = { (float)0xFFFFFFFF, 0x00000000,0x00000000,0x00000000 };

                    Maths::Vector4 vRightTop = { viewFrustum.RightSlope,viewFrustum.TopSlope,1.0f,1.0f };
                    Maths::Vector4 vLeftBottom = { viewFrustum.LeftSlope,viewFrustum.BottomSlope,1.0f,1.0f };
                    Maths::Vector4 vNear = { viewFrustum.Near,viewFrustum.Near,viewFrustum.Near,1.0f };
                    Maths::Vector4 vFar = { viewFrustum.Far,viewFrustum.Far,viewFrustum.Far,1.0f };

                    Maths::Vector4 vRightTopNear = vRightTop * vNear;
                    Maths::Vector4 vRightTopFar = vRightTop * vFar;
                    Maths::Vector4 vLeftBottomNear = vLeftBottom * vNear;
                    Maths::Vector4 vLeftBottomFar = vLeftBottom * vFar;

                    cornerPointsWorld[0] = vRightTopNear;
                    cornerPointsWorld[1] = Maths::Select(vRightTopNear, vLeftBottomNear, vGrabX);
                    cornerPointsWorld[2] = vLeftBottomNear;
                    cornerPointsWorld[3] = Maths::Select(vRightTopNear, vLeftBottomNear, vGrabY);

                    cornerPointsWorld[4] = vRightTopFar;
                    cornerPointsWorld[5] = Maths::Select(vRightTopFar, vLeftBottomFar, vGrabX);
                    cornerPointsWorld[6] = vLeftBottomFar;
                    cornerPointsWorld[7] = Maths::Select(vRightTopFar, vLeftBottomFar, vGrabY);
                }
            }

            Maths::Matrix4 GetLightProjViewForCascade(const ECS::Camera& mainCamera, const float nearPlane, const float farPlane)
            {
                ECS::Camera mainCameraProjView;
                mainCameraProjView.SetViewMatrix(mainCamera.GetViewMatrix());
                mainCameraProjView.SetProjectionMatrix(
                    Maths::Matrix4::CreatePerspective(mainCamera.GetFovY(), mainCamera.GetAspect(), nearPlane, farPlane));

                std::vector<Maths::Vector4> frustumCorners;
                frustumCorners.reserve(8);

                const Maths::Matrix4 invProjView = mainCameraProjView.GetProjectionViewMatrix().Inversed();
                for (unsigned int x = 0; x < 2; ++x)
                {
                    for (unsigned int y = 0; y < 2; ++y)
                    {
                        for (unsigned int z = 0; z < 2; ++z)
                        {
                            const Maths::Vector4 pt =
                                invProjView * Maths::Vector4(
                                    2.0f * x - 1.0f,
                                    2.0f * y - 1.0f,
                                    2.0f * z - 1.0f,
                                    1.0f);
                            frustumCorners.push_back(pt / pt.w);
                        }
                    }
                }

                Maths::Vector3 center = Maths::Vector3::Zero;
                for (const Maths::Vector3& v : frustumCorners)
                {
                    center += Maths::Vector3(v);
                }
                center /= frustumCorners.size();

                const Maths::Matrix4 lightView = Maths::Matrix4::LookAt(
                    center + LightDriection,
                    center,
                    Maths::Vector3(0.0f, 1.0f, 0.0f)
                );

                float minX = std::numeric_limits<float>::max();
                float maxX = std::numeric_limits<float>::lowest();
                float minY = std::numeric_limits<float>::max();
                float maxY = std::numeric_limits<float>::lowest();
                float minZ = std::numeric_limits<float>::max();
                float maxZ = std::numeric_limits<float>::lowest();

                for (const Maths::Vector4& v : frustumCorners)
                {
                    const auto trf = lightView * v;
                    minX = std::min(minX, trf.x);
                    maxX = std::max(maxX, trf.x);
                    minY = std::min(minY, trf.y);
                    maxY = std::max(maxY, trf.y);
                    minZ = std::min(minZ, trf.z);
                    maxZ = std::max(maxZ, trf.z);
                }

                // Tune this parameter according to the scene
                constexpr float zMult = 10.0f;
                if (minZ < 0)
                {
                    minZ *= zMult;
                }
                else
                {
                    minZ /= zMult;
                }
                if (maxZ < 0)
                {
                    maxZ /= zMult;
                }
                else
                {
                    maxZ *= zMult;
                }

                const Maths::Matrix4 lightProjection = Maths::Matrix4::CreateOrthographic(minX, maxX, minY, maxY, minZ, maxZ);
                return lightProjection * lightView;
            }
            */
    };

    struct RenderPointLight
    {
        Maths::Matrix4 ProjectionView[6];
        Maths::Vector3 LightColour;
        Maths::Vector3 Position;
        float Intensity;
        float Radius;
        Graphics::RHI_Texture* DepthTexture; // In HLSL this is just 8 bytes worth of padding.

        void CreateViewMatrixs(const Maths::Matrix4 projection, const Maths::Vector3& position)
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
                ProjectionView[i] = projection * lightView;
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