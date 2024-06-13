#pragma once

#include "Physics/DebugRendererData.h"
#include "Physics/Jolt/PhysicsWorld_Jolt.h"
#include "Core/TypeAlias.h"

#ifdef IS_PHYSICS_JOLT
#include <Jolt/Jolt.h>
#include <Jolt/Renderer/DebugRendererSimple.h>
#endif

namespace Insight::Physics::Jolt
{
#ifdef IS_PHYSICS_JOLT
#define CUSTOM_TRIANGLE_BATCHS 1

    class BatchImplJolt : public JPH::RefTargetVirtual
    {
    public:
        JPH_OVERRIDE_NEW_DELETE

        virtual void AddRef() override { ++mRefCount; }
        virtual void Release() override { if (--mRefCount == 0) delete this; }

        std::vector<Physics::DebugRendererData::Vertex> Vertices;

    private:
        std::atomic<u32> mRefCount = 0;
    };

    class DebugRendererJolt : public JPH::DebugRendererSimple
    {
    public:
        DebugRendererJolt() { Initialize(); }
        virtual ~DebugRendererJolt() override { }

        virtual void DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor) override
        {
            std::lock_guard lock(RendererData.Mutex);

            RendererData.Lines.push_back(DebugRendererData::Line
                {
                    RendererData.Vertices.size(),
                    2
                });
            RendererData.Vertices.push_back(DebugRendererData::Vertex
                {
                    Maths::Vector4(inFrom.GetX(), inFrom.GetY(), inFrom.GetZ(), 1.0f),
                    Maths::Vector4((float)inColor.r / 255.0f, (float)inColor.g / 255.0f, (float)inColor.b / 255.0f, (float)inColor.a / 255.0f)
                });
            RendererData.Vertices.push_back(DebugRendererData::Vertex
                {
                    Maths::Vector4(inTo.GetX(), inTo.GetY(), inTo.GetZ(), 1.0f),
                    Maths::Vector4((float)inColor.r / 255.0f, (float)inColor.g / 255.0f, (float)inColor.b / 255.0f, (float)inColor.a / 255.0f)
                });
        }

        virtual void DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor, ECastShadow inCastShadow) override
        {
            DrawTriangle(inV1, inV2, inV3, inColor, inCastShadow, Maths::Vector2::Zero, Maths::Vector2::Zero, Maths::Vector2::Zero);
        }

        virtual void DrawText3D(JPH::RVec3Arg inPosition, const std::string_view& inString, JPH::ColorArg inColor, float inHeight) override
        {
            std::lock_guard lock(RendererData.Mutex);

            RendererData.Texts.push_back(DebugRendererData::Text
                {
                    Maths::Vector3(inPosition.GetX(), inPosition.GetY(), inPosition.GetZ()),
                    inString.data(),
                    Maths::Vector4(inColor.r, inColor.g, inColor.b, inColor.a),
                    inHeight
                });
        }

        DebugRendererData RendererData;
    
    private:
        virtual void DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor, ECastShadow inCastShadow, Maths::Vector2 inUV1, Maths::Vector2 inUV2, Maths::Vector2 inUV3)
        {
            // Set alpha to zero if we don't want to cast shadows to notify the pixel shader
            JPH::ColorArg color(inColor, inCastShadow == ECastShadow::Off ? 0 : 0xff);

            std::lock_guard lock(RendererData.Mutex);

            static int count = 0;
            static bool colourFlip = true;
            if (RendererData.Triangles.empty())
            {
                count = 0;
                colourFlip = true;
            }

            const Maths::Vector4 colour = Maths::Vector4((float)inColor.r / 255.0f, (float)inColor.g / 255.0f, (float)inColor.b / 255.0f, (float)inColor.a / 255.0f);

            RendererData.Triangles.push_back(DebugRendererData::Triangle
                {
                    RendererData.Vertices.size(),
                    RendererData.Indices.size(),
                    3
                });
            RendererData.Vertices.push_back(DebugRendererData::Vertex
                {
                    Maths::Vector4(inV1.GetX(), inV1.GetY(), inV1.GetZ(), 1),
                    colour,
                    inUV1
                });
            RendererData.Vertices.push_back(DebugRendererData::Vertex
                {
                    Maths::Vector4(inV2.GetX(), inV2.GetY(), inV2.GetZ(), 1),
                    colour,
                    inUV2
                });
            RendererData.Vertices.push_back(DebugRendererData::Vertex
                {
                    Maths::Vector4(inV3.GetX(), inV3.GetY(), inV3.GetZ(), 1),
                    colour,
                    inUV3
                });
            RendererData.Indices.push_back(0);
            RendererData.Indices.push_back(1);
            RendererData.Indices.push_back(2);

            ++count;
        }

    protected:
        virtual Batch CreateTriangleBatch(const Triangle* inTriangles, int inTriangleCount) override
        {
#if CUSTOM_TRIANGLE_BATCHS
            BatchImplJolt* batch = new BatchImplJolt;
            if (inTriangles == nullptr || inTriangleCount == 0)
                return batch;

            for (size_t triangleIdx = 0; triangleIdx < inTriangleCount; ++triangleIdx)
            {
                for (size_t vertexIdx = 0; vertexIdx < 3; ++vertexIdx)
                {
                    const JPH::DebugRenderer::Vertex v = inTriangles[triangleIdx].mV[vertexIdx];

                        batch->Vertices.push_back(
                            {
                                Maths::Vector4(v.mPosition.x, v.mPosition.y, v.mPosition.z, 1.0f),
                                Maths::Vector4(v.mColor.r, v.mColor.g, v.mColor.b, v.mColor.a),
                                Maths::Vector2(v.mUV.x, v.mUV.y),
                            });
                }
            }
            return batch;
#else
            return __super::CreateTriangleBatch(inTriangles, inTriangleCount);
#endif
        }
        virtual Batch CreateTriangleBatch(const Vertex* inVertices, int inVertexCount, const u32* inIndices, int inIndexCount) override
        {
#if CUSTOM_TRIANGLE_BATCHS
            BatchImplJolt* batch = new BatchImplJolt;
            if (inVertices == nullptr || inVertexCount == 0 || inIndices == nullptr || inIndexCount == 0)
                return batch;

            const int indexCount = inIndexCount / 3;
            for (size_t triangleIdx = 0; triangleIdx < indexCount; ++triangleIdx)
            {
                for (size_t vertexIdx = 0; vertexIdx < 3; ++vertexIdx)
                {
                    const u32 indexIdx = triangleIdx * 3;
                    const u32 verticesIdx = indexIdx + vertexIdx;
                    const u32 indicesIdx = inIndices[verticesIdx];
                    const JPH::DebugRenderer::Vertex v = inVertices[indicesIdx];

                    batch->Vertices.push_back(
                        {
                            Maths::Vector4(v.mPosition.x, v.mPosition.y, v.mPosition.z, 1.0f),
                            Maths::Vector4(v.mColor.r, v.mColor.g, v.mColor.b, v.mColor.a),
                            Maths::Vector2(v.mUV.x, v.mUV.y),
                        });
                }
            }

            return batch;
#else
            return __super::CreateTriangleBatch(inVertices, inVertexCount, inIndices, inIndexCount);
#endif
        }

        void DrawGeometry(JPH::RMat44Arg inModelMatrix, const JPH::AABox& inWorldSpaceBounds, float inLODScaleSq, JPH::ColorArg inModelColor, const GeometryRef& inGeometry, ECullMode inCullMode, ECastShadow inCastShadow, EDrawMode inDrawMode)
        {
#if CUSTOM_TRIANGLE_BATCHS
            // Figure out which LOD to use
            const LOD* lod = inGeometry->mLODs.data();

            // Draw the batch
            const BatchImplJolt* batch = static_cast<const BatchImplJolt*>(lod->mTriangleBatch.GetPtr());
            for (size_t i = 0; i < batch->Vertices.size(); i += 3)
            {
                const Physics::DebugRendererData::Vertex& V0 = batch->Vertices[i + 0];
                const Physics::DebugRendererData::Vertex& V1 = batch->Vertices[i + 1];
                const Physics::DebugRendererData::Vertex& V2 = batch->Vertices[i + 2];

                JPH::RVec3 v0 = inModelMatrix * JPH::Vec3(PhysicsWorld_Jolt::Vector4ToJolt(V0.Position));
                JPH::RVec3 v1 = inModelMatrix * JPH::Vec3(PhysicsWorld_Jolt::Vector4ToJolt(V1.Position));
                JPH::RVec3 v2 = inModelMatrix * JPH::Vec3(PhysicsWorld_Jolt::Vector4ToJolt(V2.Position));
                JPH::Color color = inModelColor * JPH::Color(V0.Colour.x, V0.Colour.y, V0.Colour.z, V0.Colour.w);

                switch (inDrawMode)
                {
                case EDrawMode::Wireframe:
                    DrawLine(v0, v1, color);
                    DrawLine(v1, v2, color);
                    DrawLine(v2, v0, color);
                    break;

                case EDrawMode::Solid:
                    DrawTriangle(v0, v1, v2, color, inCastShadow, V0.UV, V1.UV, V2.UV);
                    break;
                }
            }
#else
            return __super::DrawGeometry(inModelMatrix, inWorldSpaceBounds, inLODScaleSq, inModelColor, inGeometry, inCullMode, inCastShadow, inDrawMode);
#endif
        }
    };
#endif
}