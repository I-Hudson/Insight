#pragma once

#include "Input/InputButtonState.h"

namespace Insight
{
	namespace Input
	{
		enum class MouseButtons
		{
			Left,
			Right,
			Middle,
			Side0,
			Side1,

			MouseButtonsCount
		};

		class InputMouseState : public InputButtonState<static_cast<u64>(MouseButtons::MouseButtonsCount)>
		{
		public:
			InputMouseState();
			~InputMouseState();

			virtual void ClearFrame() override;

			float GetXPosition() const;
			float GetYPosition() const;
			void MouseMoved(float xPosition, float yPosition);

		private:
			float m_xPosition;
			float m_yPosition;
			float m_scrollAmount = 0.0f;
		};
	}
}