#pragma once

#include "Core/TypeAlias.h"

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
        struct CPUInformation
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
        struct MemoryInformation
        {

        };
    }
}