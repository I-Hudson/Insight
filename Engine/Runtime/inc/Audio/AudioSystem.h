#pragma once

#include "Runtime/Defines.h"
#include "Core/ISysytem.h"
#include "Core/Singleton.h"

#include <vector>

namespace Insight
{
    namespace Runtime
    {
        class IAudioBackend;

        class IS_RUNTIME AudioSystem : public Core::ISystem
        {
        public:
            AudioSystem();
            virtual ~AudioSystem() override;

            IS_SYSTEM(AudioSystem);

            virtual void Initialise() override;
            virtual void Shutdown() override;

            static u32 PlaySound(const char* fileName);
            static void StopSound(const u32 soundId);

        private:
            static IAudioBackend* m_audioBackend;
        };
    }
}