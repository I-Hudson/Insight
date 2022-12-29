#pragma once

#include "Core/TypeAlias.h"
#include <numeric>

namespace Insight
{
    namespace Input
    {
        enum class AnalogStateUpdates
        {
            UpdatedThisFrame,
            UpdateProgression
        };

        /// <summary>
        /// Represent a single analog input value.
        /// </summary>
        class InputAnalogState
        {
        public:
            InputAnalogState() = default;
            ~InputAnalogState() = default;

            void Update()
            {
                if (m_update == AnalogStateUpdates::UpdatedThisFrame)
                {
                    m_update = AnalogStateUpdates::UpdateProgression;
                }
            }
            bool HasInput() const
            {
                return m_value != 0;
            }
            void ClearFrame()
            {
                m_value = 0.0f;
            }

            float GetValue() const { return m_value; }

            void SetValue(float value)
            {
                m_value = std::max(-1.0f, std::min(1.0f, value));
            }

        private:
            float m_value = 0.0f; // Value between -1 to 1.
            AnalogStateUpdates m_update;
        };
    }
}