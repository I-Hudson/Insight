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
        void InputTriggerState::SetValue(float value)
        {
            m_trigger.SetValue(value);
        }
    }
}