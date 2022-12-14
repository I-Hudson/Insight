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
            u16 GetRawValue() const;

            void SetValue(float value);
            void SetRawValue(u16 value);

        private:
            float m_scaledValue;
            u16 m_rawValue;
            static constexpr u16 c_MaxRawValue = _UI16_MAX;
        };
    }
}