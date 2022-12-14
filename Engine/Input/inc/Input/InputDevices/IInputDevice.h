#pragma once
#include "Input/Defines.h"

#include "Input/GenericInput.h"

namespace Insight
{
	namespace Input
	{
		struct DeviceInfo
		{
			u32 VendorId = 0;
			u32 ProductId = 0;
			u32 RevisionId = 0;
		};

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
			DeviceInfo GetDeviceInfo() const { return m_deviceInfo; }

			virtual void ProcessInput(GenericInput const& input) = 0;
			virtual bool HasInput() const = 0;
			virtual void Update(float const deltaTime) = 0;
			virtual void ClearFrame() = 0;

		protected:
			u32 m_id = 0;
			DeviceInfo m_deviceInfo;
		};
	}
}