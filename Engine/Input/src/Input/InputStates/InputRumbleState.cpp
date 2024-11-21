#include "Input/InputStates/InputRumbleState.h"

#include "Maths/MathsUtils.h"

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
            m_value = Maths::Max(0.0f, Maths::Min(c_MaxValue, value));
        }
    }
}