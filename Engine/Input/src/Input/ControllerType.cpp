#include "Input/ControllerTypes.h"

#include "Core/Defines.h"

namespace Insight
{
    namespace Input
    {
		// https://devicehunt.com/view/type/usb/vendor/045E#search-results-table
		constexpr u16 MICROSOFT_VENDOR_ID = 0x045E;
		constexpr u16 PLAYSTATION_VENDOR_ID = 0x054C;

		char const* ControllerVendorToString(ControllerVendors vendor)
		{
			switch (vendor)
			{
			case ControllerVendors::Microsoft:	   return "Microsoft";
			case ControllerVendors::Sony:		   return "Sony";
			case ControllerVendors::NumControllerVendors:
			default:
				break;
			}
			return "Unknown";
		}

		ControllerVendors VendorIdToControllerVendor(u16 vendorId)
		{
			switch (vendorId)
			{
			case MICROSOFT_VENDOR_ID:   return ControllerVendors::Microsoft;
			case PLAYSTATION_VENDOR_ID: return ControllerVendors::Sony;
			default:
				break;
			}
			return ControllerVendors::Unknown;
		}

		const char* ControllerSubTypeToString(ControllerSubTypes subType)
		{
			switch (subType)
			{
			case ControllerSubTypes::Xbox360:                 return "Xbox360";
			case ControllerSubTypes::XboxOne:                 return "XboxOne";
			case ControllerSubTypes::XboxOneS:                return "XboxOneS";
			case ControllerSubTypes::XboxOneElite:            return "XboxOneElite";
			case ControllerSubTypes::PlayStationDualSensePS5: return "PlayStationDualSensePS5";
			case ControllerSubTypes::PlayStationPS3:          return "PlayStationPS3";
			case ControllerSubTypes::PlayStationClassic:      return "PlayStationClassic";
				break;
			default:
				break;
			}
			return "Unknown";
		}

        ControllerType ControllerTypes[] =
        {
			ControllerType{ MICROSOFT_VENDOR_ID, 0xB12, 0, ControllerVendors::Microsoft, ControllerSubTypes::XboxOneS },
			ControllerType{ MICROSOFT_VENDOR_ID, 0x02EA, 0, ControllerVendors::Microsoft, ControllerSubTypes::XboxOneS },
			ControllerType{ MICROSOFT_VENDOR_ID, 0x02FD, 0, ControllerVendors::Microsoft, ControllerSubTypes::XboxOneS },
			ControllerType{ MICROSOFT_VENDOR_ID, 0x02E3, 0, ControllerVendors::Microsoft, ControllerSubTypes::XboxOneElite },
			ControllerType{ MICROSOFT_VENDOR_ID, 0x02D1, 0, ControllerVendors::Microsoft, ControllerSubTypes::XboxOne },
			ControllerType{ MICROSOFT_VENDOR_ID, 0x2FF, 0, ControllerVendors::Microsoft, ControllerSubTypes::XboxOne },
			ControllerType{ MICROSOFT_VENDOR_ID, 0x02DD, 0, ControllerVendors::Microsoft, ControllerSubTypes::XboxOne },
			ControllerType{ MICROSOFT_VENDOR_ID, 0x045E, 0, ControllerVendors::Microsoft, ControllerSubTypes::Xbox360 },

			ControllerType{ PLAYSTATION_VENDOR_ID, 0x0CE6, 0, ControllerVendors::Sony, ControllerSubTypes::PlayStationDualSensePS5 },
			ControllerType{ PLAYSTATION_VENDOR_ID, 0x0268, 0, ControllerVendors::Sony, ControllerSubTypes::PlayStationPS3 },
			ControllerType{ PLAYSTATION_VENDOR_ID, 0x0CDA, 0, ControllerVendors::Sony, ControllerSubTypes::PlayStationClassic },
        };
 
		ControllerSubTypes ProductIdToControllerSubType(u16 productId)
		{
			for (size_t i = 0; i < ARRAY_COUNT(ControllerTypes); ++i)
			{
				if (ControllerTypes[i].ProductId == productId)
				{
					return ControllerTypes[i].SubType;
				}
			}
			return ControllerSubTypes::Unknown;
		}
    }
}