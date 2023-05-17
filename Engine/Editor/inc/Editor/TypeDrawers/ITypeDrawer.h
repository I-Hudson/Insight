#pragma once

#include <Reflect.h>

namespace Insight
{
    namespace Editor
    {
        class ITypeDrawer
        {
        public:
            virtual void Draw(void* data) const = 0;
        };
    }
}