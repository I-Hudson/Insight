#pragma once

#include "Core/TypeAlias.h"
#include "Input/Defines.h"

namespace Insight
{
    namespace Input
    {
        enum class ControllerVendors
        {
            Unknown,
            Microsoft,
            Sony,

            NumControllerVendors
        };
        IS_INPUT const char* ControllerVendorToString(ControllerVendors vendor);
        IS_INPUT ControllerVendors VendorIdToControllerVendor(u16 vendorId);

        /// <summary>
        /// Define the which controller "InputDevice_Controller" is.
        /// Note: This defines the internal representation of the controller, this might not be the 
        /// actual controller type. 
        /// (Example if XInput is used then all controllers are defined as Xbox360 even if they are not Xbox360 controllers).
        /// </summary>
        enum class ControllerSubTypes : u16
        {
            Unknown,
            Xbox360,
            XboxOne,
            XboxOneS,
            XboxOneElite,

            PlayStationDualSensePS5,
            PlayStationPS3,
            PlayStationClassic,

            NumControllerSubTypes
        };
        IS_INPUT const char* ControllerSubTypeToString(ControllerSubTypes subType);

        struct ControllerType
        {
            u16 VendorId;
            u16 ProductId;
            u16 RevisionId;
            ControllerVendors Vendor;
            ControllerSubTypes SubType;
        };
        extern ControllerType ControllerTypes[];
        IS_INPUT ControllerSubTypes ProductIdToControllerSubType(u16 productId);
    }
}