#pragma once

#include "Core/TypeAlias.h"

namespace Insight
{
    namespace Runtime
    {
        class IAudioBackend
        {
        public:
            virtual ~IAudioBackend() { }

            virtual u32 PlaySound(const char* fileName) = 0;
            virtual void StopSound(const u32 soundId) = 0;
        };
    }
}