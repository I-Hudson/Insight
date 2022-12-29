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

            void SetValueX(float value);
            void SetValueY(float value);

        private:
            InputAnalogState m_x;
            InputAnalogState m_y;
        };
    }
}