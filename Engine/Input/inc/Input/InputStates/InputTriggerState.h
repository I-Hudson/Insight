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
            u8 GetRawValue() const;

            void SetValue(float value);
            void SetRawValue(u8 value);

        private:
            InputAnalogState<u8> m_trigger;
        };
    }
}