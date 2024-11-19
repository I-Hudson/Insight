#include "Input/InputDevices/IInputDevice.h"

namespace Insight::Input
{
    IInputDevice::IInputDevice(const InputDeviceTypes inputDeviceType)
        : m_deviceType(inputDeviceType)
    { }
}
