#pragma once

#include "Editor/HotReload/HotReloadOperation.h"
#include "Editor/HotReload/HotReloadMetaData.h"

#include "ECS/Entity.h"

namespace Insight::Editor
{
    class ComponentsOperation : public HotReloadOperation
    {
    public:
    public:
        ComponentsOperation();
        virtual ~ComponentsOperation() override;

        virtual void Reset() override;
        virtual void PreUnloadOperation() override;
        virtual void PostLoadOperation() override;

    private:
        /// @brief Remove all components which exist in the project DLL.
        /// @param metaData 
        void RemoveComponents(const HotReloadMetaData& metaData);
        void AddComponents();
    };
}