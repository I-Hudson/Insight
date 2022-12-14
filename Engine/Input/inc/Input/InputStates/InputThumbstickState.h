#pragma once

#include "Core/TypeAlias.h"
#include "Input/InputStates/InputAnalogState.h"
#include <numeric>

namespace Insight
{
    namespace Input
    {
        class InputThumbstickState
        {
        public:
            InputThumbstickState();
            ~InputThumbstickState();

            void Update();
            bool HasInput() const;
            void ClearFrame();

            float GetValueX() const;
            float GetValueY() const;
            i16 GetRawValueX() const;
            i16 GetRawValueY() const;

            void SetValueX(float value);
            void SetValueY(float value);
            void SetRawValueX(i16 value);
            void SetRawValueY(i16 value);

        private:
            InputAnalogState<i16> m_x;
            InputAnalogState<i16> m_y;
        };
    }
}