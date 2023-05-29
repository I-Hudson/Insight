#pragma once

#include "Editor/Defines.h"

#include <Reflect.h>

namespace Insight
{
    namespace Editor
    {
        class IS_EDITOR ITypeDrawer
        {
        public:
            virtual void Draw(void* data) const = 0;
        };
    }
}