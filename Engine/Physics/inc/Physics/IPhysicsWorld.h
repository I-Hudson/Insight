#pragma once

#include "Core/TypeAlias.h"
#include "Core/ReferencePtr.h"
#include "Physics/BodyId.h"

namespace Insight::Physics
{
    class BodyCreationSettings;
    class IShape;

    class IPhysicsWorld
    {
    public:
        virtual ~IPhysicsWorld() { }

        virtual void Initialise() = 0;
        virtual void Shutdown() = 0;

        virtual void Update(const float deltaTime) = 0;

        virtual void StartRecord() = 0;
        virtual void EndRecord() = 0;

        virtual BodyId CreateBody(const BodyCreationSettings& bodyCreationSettings) = 0;
        virtual void DestoryBody(const BodyId bodyId) = 0;

        virtual void AddBody(const BodyId bodyId) = 0;
        virtual void RemoveBody(const BodyId bodyId) = 0;

        virtual void SetBodyShape(const BodyId bodyId, IShape* shape) = 0;
    };
}