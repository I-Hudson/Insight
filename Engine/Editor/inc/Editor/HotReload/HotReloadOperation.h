#pragma once 

namespace Insight::Editor
{
    class HotReloadOperation
    {
    public:
        HotReloadOperation();
        virtual ~HotReloadOperation();

        virtual void Reset() = 0;
        virtual void PreUnloadOperation() = 0;
        virtual void PostLoadOperation() = 0;
    };
}