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
			virtual void Initialise() = 0;
			virtual void Shutdown() = 0;

			virtual InputDeviceTypes GetDeviceType() const = 0;

			virtual void ProcessInput(GenericInput const& input) = 0;
			virtual void Update(float const deltaTime) = 0;
		};
	}
}