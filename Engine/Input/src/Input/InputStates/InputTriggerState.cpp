#include "Input/InputStates/InputTriggerState.h"

namespace Insight
{
    namespace Input
    {
        InputTriggerState::InputTriggerState()
        { }

        InputTriggerState::~InputTriggerState()
        { }

        void InputTriggerState::Update()
        {
            m_trigger.Update();
        }

        bool InputTriggerState::HasInput() const
        {
            return m_trigger.HasInput();
        }

        void InputTriggerState::ClearFrame()
        {
            m_trigger.ClearFrame();
        }

        float InputTriggerState::GetValue() const
        {
            return m_trigger.GetValue();
        }

        u8 InputTriggerState::GetRawValue() const
        {
            return m_trigger.GetRawValue();
        }

        void InputTriggerState::SetValue(float value)
        {
            m_trigger.SetValue(value);
        }

        void InputTriggerState::SetRawValue(u8 value)
        {
            m_trigger.SetRawValue(value);
        }
    }
}