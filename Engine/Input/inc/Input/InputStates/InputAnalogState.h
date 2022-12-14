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
        template<typename IntType>
        class InputAnalogState
        {
            static_assert(std::is_integral_v<IntType>);
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
                return m_rawValue != 0;
            }
            void ClearFrame()
            {
                m_scaledValue = 0.0f;
                m_rawValue = 0;
            }

            float GetValue() const { return m_scaledValue; }
            IntType GetRawValue() const { return m_rawValue; }

            void SetValue(float value)
            {
                m_scaledValue = value;
                m_scaledValue = std::max(-1.0f, std::min(1.0f, m_scaledValue));
            }

            void SetRawValue(IntType value)
            {
                m_rawValue = value;
                m_update = AnalogStateUpdates::UpdatedThisFrame;
            }

        private:
            float m_scaledValue = 0.0f; // Value between -1 to 1.
            IntType m_rawValue = 0;
            AnalogStateUpdates m_update;
        };
    }
}