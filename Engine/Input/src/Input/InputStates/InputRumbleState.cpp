#include "Input/InputStates/InputRumbleState.h"

#include <algorithm>

namespace Insight
{
    namespace Input
    {
        float InputRumbleState::GetValue() const
        {
            return m_value;
        }

        void InputRumbleState::SetValue(float value)
        {
            m_value = std::max(0.0f, std::min(c_MaxValue, value));
        }
    }
}