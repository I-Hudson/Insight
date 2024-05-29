#pragma once

#include "Core/TypeAlias.h"
#include "Core/ReferencePtr.h"
#include "Physics/Body.h"

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

        virtual Ref<Body> CreateBody(const BodyCreationSettings& bodyCreationSettings) = 0;
        virtual void DestoryBody(const Ref<Body>& body) = 0;

        virtual void AddBody(const Ref<Body>& body) = 0;
        virtual void RemoveBody(const Ref<Body>& body) = 0;
    };
}