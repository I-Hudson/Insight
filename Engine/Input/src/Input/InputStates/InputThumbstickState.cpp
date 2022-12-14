#include "Input/InputStates/InputThumbstickState.h"

namespace Insight
{
    namespace Input
    {
        InputThumbstickState::InputThumbstickState()
        { }

        InputThumbstickState::~InputThumbstickState()
        { }

        void InputThumbstickState::Update()
        {
            m_x.Update();
            m_y.Update();
        }

        bool InputThumbstickState::HasInput() const
        {
            return m_x.HasInput() || m_y.HasInput();
        }

        void InputThumbstickState::ClearFrame()
        {
            m_x.ClearFrame();
            m_y.ClearFrame();
        }

        float InputThumbstickState::GetValueX() const
        {
            return m_x.GetValue();
        }

        float InputThumbstickState::GetValueY() const
        {
            return m_y.GetValue();
        }

        i16 InputThumbstickState::GetRawValueX() const
        {
            return m_x.GetRawValue();
        }

        i16 InputThumbstickState::GetRawValueY() const
        {
            return m_y.GetRawValue();
        }

        void InputThumbstickState::SetValueX(float value)
        {
            m_x.SetValue(value);
        }

        void InputThumbstickState::SetValueY(float value)
        {
            m_y.SetValue(value);
        }

        void InputThumbstickState::SetRawValueX(i16 value)
        {
            m_x.SetRawValue(value);
        }

        void InputThumbstickState::SetRawValueY(i16 value)
        {
            m_y.SetRawValue(value);
        }
    }
}