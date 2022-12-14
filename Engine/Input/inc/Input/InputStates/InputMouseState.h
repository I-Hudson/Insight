#pragma once

#include "Input/InputStates/InputButtonState.h"

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

			virtual bool HasInput() const override;
			virtual void ClearFrame() override;

			float GetXPosition() const;
			float GetYPosition() const;
			void MouseMoved(float xPosition, float yPosition);
			void MouseScrolled(float xOffset, float yOffset);

		private:
			float m_xPosition = 0.0f;
			float m_yPosition = 0.0f;
			float m_scrollXOffset = 0.0f;
			float m_scrollYOffset = 0.0f;
		};
	}
}