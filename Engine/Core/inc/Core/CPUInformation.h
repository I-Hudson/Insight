#pragma once

#include "Core/TypeAlias.h"
#include "Core/Defines.h"

#include <string>

namespace Insight
{
    namespace Core
    {
        enum class CPUArchitectures
        {
            AMD64,
            ARM,
            ARM64,
            IA64,
            INTEL,
            Unknown
        };
        constexpr const char* CPUArchitectureToString(CPUArchitectures architecture)
        {
            switch (architecture)
            {
            case Insight::Core::CPUArchitectures::AMD64: return "AMD64";
            case Insight::Core::CPUArchitectures::ARM:   return "ARM";
            case Insight::Core::CPUArchitectures::ARM64: return "ARM64";
            case Insight::Core::CPUArchitectures::IA64:  return "IA64";
            case Insight::Core::CPUArchitectures::INTEL: return "INTEL";
            case Insight::Core::CPUArchitectures::Unknown:
            default:
                break;
            }
            return "Unknown";
        }

        /// @brief Store general information about the CPU of the system.
        struct IS_CORE CPUInformation
        {
            CPUInformation() = default;

            std::string Vendor;
            std::string Model;
            CPUArchitectures CPUArchitecture = CPUArchitectures::Unknown;

            u32 PhysicalCoreCount = 0;
            u32 LogicalCoreCount = 0;
            u32 SpeedInMHz = 0;

            bool IsHyperThreaded = false;

            bool IsSSE = false;
            bool IsSSE2 = false;
            bool IsSSE3 = false;
            bool IsSSE41 = false;
            bool IsSSE42 = false;
            bool IsAVX = false;
            bool IsAVX2 = false;

            bool Initialised = false;
        };

        /// @brief Store general information about the memory of the system.
        // All information is in bytes.
        struct IS_CORE MemoryInformation
        {
            constexpr static u64 ToKB(u64 value) { return value / 1024; }
            constexpr static u64 ToMB(u64 value) { return value / 1024 / 1024; }
            constexpr static u64 ToGB(u64 value) { return value / 1024 / 1024 / 1024; }

            /// @brief System total RAM.
            u64 TotalPhyscialMemoryBytes = 0;
            u64 TotalVirtualMemoryBytes = 0;

            /// @brief System free RAM.
            u64 TotalPhyscialMemoryFreeBytes = 0;
            u64 TotalVirtualMemoryFreeBytes = 0;

            /// @brief System used RAM (not process/application).
            u64 TotalPhyscialMemoryUsedBytes = 0;
            u64 TotalVirtualMemoryUsedBytes = 0;

            /// @brief Process (application) used RAM.
            u64 ProcessMemoryUsageBytes = 0;
            /// @brief Process (application) number of pages currently committed by the system.
            u64 ProcessNumOfPagesCommitted = 0;

            /// @brief Size in bytes of a single page from the system.
            u64 PageSizeBytes = 0;

            u64 LastUpdateTimeStamp = 0;
            bool Initialised = false;
        };
    }
}