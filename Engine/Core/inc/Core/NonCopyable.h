#pragma once

#include "Core/Defines.h"

namespace Insight
{
    class IS_CORE NonCopyable
    {
    public:
        NonCopyable() = default;
        NonCopyable(const NonCopyable&) = delete;
        void operator=(const NonCopyable&) = delete;
    };
}