#pragma once

#include "Core/TypeAlias.h"
#include "Core/Asserts.h"

#include <array>

namespace Insight
{
	namespace Input
	{
		enum class ButtonStates
		{
			None,
			Pressed,
			Released,
			Held
		};

		enum class ButtonUpdate
		{
			UpdatedThisFrame,
			UpdateProgression
		};

		/// <summary>
		/// Single state for a button.
		/// </summary>
		class InputButton
		{
		public:
			InputButton();
			~InputButton();

			ButtonStates GetState() const;
			bool WasPressed() const;
			bool WasReleased() const;
			bool WasHeld() const;
			float GetHeldTIme() const;

			void Update(float const deltaTime);
			bool HasInput() const;
			void ClearFrame();

			void SetState(ButtonStates newState);

		private:
			ButtonStates m_state = ButtonStates::None;
			ButtonUpdate m_buttonUpdate = ButtonUpdate::UpdatedThisFrame;
			float m_heldTime = 0.0f;
		};

		template<u64 NumButtons>
		class InputButtonState
		{
		public:
			InputButtonState() { }
			~InputButtonState() { }

			virtual void Update(float const deltaTime)
			{
				for (auto& button : m_buttons)
				{
					button.Update(deltaTime);
				}
			}
			virtual bool HasInput() const
			{
				bool hasInput = false;
				for (auto& button : m_buttons)
				{
					hasInput |= button.HasInput();
				}
				return hasInput;
			}
			virtual void ClearFrame()
			{
				for (auto& button : m_buttons)
				{
					button.ClearFrame();
				}
			}

			ButtonStates GetState(u32 buttonIdx) const
			{
				ASSERT(buttonIdx < NumButtons);
			}

			bool WasPressed(u32 buttonIdx) const
			{
				ASSERT(buttonIdx < NumButtons);
				return m_buttons.at(buttonIdx).WasPressed();
			}
			bool WasReleased(u32 buttonIdx) const
			{
				ASSERT(buttonIdx < NumButtons);
				return m_buttons.at(buttonIdx).WasReleased();
			}
			bool WasHeld(u32 buttonIdx) const
			{
				ASSERT(buttonIdx < NumButtons);
				return m_buttons.at(buttonIdx).WasHeld();
			}
			float GetHeldTime(u32 buttonIdx) const
			{
				ASSERT(buttonIdx < NumButtons);
				return m_buttons.at(buttonIdx).GetHeldTime();
			}

			void ButtonPressed(u32 buttonIdx)
			{
				ASSERT(buttonIdx < NumButtons);
				m_buttons.at(buttonIdx).SetState(ButtonStates::Pressed);
			}
			void ButtonReleased(u32 buttonIdx)
			{
				ASSERT(buttonIdx < NumButtons);
				m_buttons.at(buttonIdx).SetState(ButtonStates::Released);
			}

		protected:
			std::array<InputButton, NumButtons> m_buttons;
		};
	}
}