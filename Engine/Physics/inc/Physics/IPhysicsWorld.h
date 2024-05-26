#pragma once

#include "Core/TypeAlias.h"

namespace Insight::Physics
{
    using BodyId = u32;

    class IPhysicsWorld
    {
    public:
        virtual ~IPhysicsWorld() { }

        virtual void Initialise() = 0;
        virtual void Shutdown() = 0;

        virtual void Update(const float deltaTime) = 0;

        virtual void StartRecord() = 0;
        virtual void EndRecord() = 0;

        virtual BodyId Addbody() = 0;
        virtual void DestoryBody(const BodyId bodyId) = 0;

        virtual void ActivateBody(const BodyId body) = 0;
        virtual void DeactivateBody(const BodyId bodyId) = 0;

    };
}