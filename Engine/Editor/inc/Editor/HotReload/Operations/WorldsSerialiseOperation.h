#pragma once

#include "Editor/HotReload/HotReloadOperation.h"
#include "Editor/HotReload/HotReloadMetaData.h"

#include "ECS/Entity.h"

namespace Insight::Runtime
{
    class World;
}

namespace Insight::Editor
{
    class WorldsSerialiseOperation : public HotReloadOperation
    {
    public:
        WorldsSerialiseOperation();
        virtual ~WorldsSerialiseOperation() override;

        virtual void Reset() override;
        virtual void PreUnloadOperation() override;
        virtual void PostLoadOperation() override;

    private:
        std::vector<Byte> m_activeWorldSerialisedData;
    };
}