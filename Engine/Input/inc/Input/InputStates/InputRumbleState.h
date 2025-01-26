#pragma once

#include "Core/TypeAlias.h"

namespace Insight
{
    namespace Input
    {
        class InputRumbleState
        {
        public:
            InputRumbleState() = default;
            ~InputRumbleState() = default;

            float GetValue() const;
            void SetValue(float value);

        private:
            float m_value = 0.0f;
            static constexpr float c_MaxValue = 1.0f;
        };
    }
}