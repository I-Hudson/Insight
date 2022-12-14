#include "Input/InputStates/InputRumbleState.h"

#include <algorithm>

namespace Insight
{
    namespace Input
    {
        float InputRumbleState::GetValue() const
        {
            return m_scaledValue;
        }

        u16 InputRumbleState::GetRawValue() const
        {
            return m_rawValue;
        }

        void InputRumbleState::SetValue(float value)
        {
            value = std::max(0.0f, std::min(1.0f, value));
            u16 newRawValue = static_cast<u16>(value * c_MaxRawValue);
            SetRawValue(newRawValue);
        }

        void InputRumbleState::SetRawValue(u16 value)
        {
            value = std::max(0ui16, std::min(c_MaxRawValue, value));
            m_rawValue = value;
            m_scaledValue = static_cast<float>(m_rawValue) / static_cast<float>(c_MaxRawValue);
        }
    }
}