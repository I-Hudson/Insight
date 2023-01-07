#pragma once

#include "Input/Defines.h"
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

			NumButtons
		};
		IS_INPUT const char* MouseButtonToString(MouseButtons button);

		class InputMouseState : public InputButtonState<static_cast<u64>(MouseButtons::NumButtons)>
		{
		public:
			InputMouseState();
			~InputMouseState();

			virtual bool HasInput() const override;
			virtual void ClearFrame() override;

			float GetXPosition() const;
			float GetYPosition() const;

			float GetXScollOffset() const;
			float GetYScollOffset() const;
			
			void MouseMoved(float xPosition, float yPosition);
			void MouseScrolled(float xOffset, float yOffset);

		private:
			float m_xPosition = 0.0f;
			float m_yPosition = 0.0f;

			float m_scrollXOffset = 0.0f;
			float m_scrollYOffset = 0.0f;

			bool m_mouseMoved = false;
		};
	}
}