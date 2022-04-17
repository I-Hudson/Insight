#pragma once

#include <string>

namespace Insight
{
	namespace Graphics
	{
		// GPU vendors IDs
#define GPU_VENDOR_ID_AMD 0x1002
#define GPU_VENDOR_ID_INTEL 0x8086
#define GPU_VENDOR_ID_NVIDIA 0x10DE
#define GPU_VENDOR_ID_MICROSOFT 0x1414

        class RHI_PhysicalDevice
        {
        public:
            RHI_PhysicalDevice()
            {
            }

            RHI_PhysicalDevice(const RHI_PhysicalDevice& other)
                : RHI_PhysicalDevice()
            {
                *this = other;
            }

            RHI_PhysicalDevice& operator=(const RHI_PhysicalDevice& other)
            {
                return *this;
            }

        public:

            virtual bool IsValid() const = 0;
            virtual int GetVendorId() const = 0;
            virtual std::string GetName() const = 0;

        public:

            bool IsAMD() const
            {
                return GetVendorId() == GPU_VENDOR_ID_AMD;
            }

            bool IsIntel() const
            {
                return GetVendorId() == GPU_VENDOR_ID_INTEL;
            }

            bool IsNVIDIA() const
            {
                return GetVendorId() == GPU_VENDOR_ID_NVIDIA;
            }

            bool IsMicrosoft() const
            {
                return GetVendorId() == GPU_VENDOR_ID_MICROSOFT;
            }
        };
	}
}