#pragma once

#include "Core/TypeAlias.h"
#include "Input/InputStates/InputAnalogState.h"
#include <numeric>

namespace Insight
{
    namespace Input
    {
        class InputTriggerState
        {
        public:
            InputTriggerState();
            ~InputTriggerState();

            void Update();
            bool HasInput() const;
            void ClearFrame();

            float GetValue() const;
            void SetValue(float value);

        private:
            InputAnalogState m_trigger;
        };
    }
}