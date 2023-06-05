#pragma once

#include "Editor/HotReload/HotReloadOperation.h"

#include <string>
#include <vector>

namespace Insight::Editor
{
    class TypeDrawersOperation : public HotReloadOperation
    {
    public:
        TypeDrawersOperation();
        virtual ~TypeDrawersOperation() override;

        virtual void Reset() override;
        virtual void PreUnloadOperation() override;
        virtual void PostLoadOperation() override;

    private:
    };
}