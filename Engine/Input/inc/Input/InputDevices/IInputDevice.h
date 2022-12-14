#pragma once
#include "Input/Defines.h"

#include "Input/GenericInput.h"

namespace Insight
{
	namespace Input
	{
		/// <summary>
		/// Define a device which accepts inputs from the system.
		/// </summary>
		class IS_INPUT IInputDevice
		{
		public:
			virtual ~IInputDevice() = default;
			
			virtual void Initialise(u32 id) = 0;
			virtual void Shutdown() = 0;

			virtual InputDeviceTypes GetDeviceType() const = 0;
			u32 GetId() const { return m_id; }

			virtual void ProcessInput(GenericInput const& input) = 0;
			virtual bool HasInput() const = 0;
			virtual void Update(float const deltaTime) = 0;
			virtual void ClearFrame() = 0;

		protected:
			u32 m_id = 0;
		};
	}
}