#pragma once

#include "Physics/DebugRendererData.h"

#ifdef IS_PHYSICS_JOLT
#include <Jolt/Jolt.h>
#include <Jolt/Renderer/DebugRendererSimple.h>
#endif

namespace Insight::Physics::Jolt
{
#ifdef IS_PHYSICS_JOLT
    class DebugRendererJolt : public JPH::DebugRendererSimple
    {
    public:
        DebugRendererJolt() { Initialize(); }
        virtual ~DebugRendererJolt() override { }

        virtual void DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor) override
        {
            std::lock_guard l(RendererData.LinesMutex);
            RendererData.Lines.push_back(DebugRendererData::Line
                {
                    Maths::Vector3(inFrom.GetX(), inFrom.GetY(), inFrom.GetZ()),
                    Maths::Vector4(inColor.r, inColor.g, inColor.b, inColor.a),
                    Maths::Vector3(inTo.GetX(), inTo.GetY(), inTo.GetZ()),
                    Maths::Vector4(inColor.r, inColor.g, inColor.b, inColor.a),
                });
        }

        virtual void DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor, ECastShadow inCastShadow) override
        {
            // Set alpha to zero if we don't want to cast shadows to notify the pixel shader
            JPH::ColorArg color(inColor, inCastShadow == ECastShadow::Off ? 0 : 0xff);

            DrawLine(inV1, inV2, color);
            DrawLine(inV2, inV3, color);
            DrawLine(inV3, inV1, color);
        }

        virtual void DrawText3D(JPH::RVec3Arg inPosition, const std::string_view& inString, JPH::ColorArg inColor, float inHeight) override
        {
            std::lock_guard l(RendererData.TextsMutex);
            RendererData.Texts.push_back(DebugRendererData::Text
                {
                    Maths::Vector3(inPosition.GetX(), inPosition.GetY(), inPosition.GetZ()),
                    inString.data(),
                    Maths::Vector4(inColor.r, inColor.g, inColor.b, inColor.a),
                    inHeight
                });
        }

        DebugRendererData RendererData;
    };
#endif
}