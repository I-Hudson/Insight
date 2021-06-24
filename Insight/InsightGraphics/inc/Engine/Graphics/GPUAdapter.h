#pragma once

#include "Engine/Core/Common.h"

// GPU vendors IDs
#define GPU_VENDOR_ID_AMD 0x1002
#define GPU_VENDOR_ID_INTEL 0x8086
#define GPU_VENDOR_ID_NVIDIA 0x10DE
#define GPU_VENDOR_ID_MICROSOFT 0x1414

/// <summary>
/// Interface for GPU device adapter.
/// </summary>
class IS_API GPUAdapter
{
public:

    GPUAdapter()
    {
    }

    GPUAdapter(const GPUAdapter& other)
        : GPUAdapter()
    {
        *this = other;
    }

    GPUAdapter& operator=(const GPUAdapter& other)
    {
        return *this;
    }

public:

    /// <summary>
    /// Checks if adapter is valid and returns true if it is.
    /// </summary>
    /// <returns>True if valid, otherwise false.</returns>
    virtual bool IsValid() const = 0;

    /// <summary>
    /// Gets the GPU vendor identifier.
    /// </summary>
    virtual u32 GetVendorId() const = 0;

    /// <summary>
    /// Gets a string that contains the adapter description. Used for presentation to the user.
    /// </summary>
    virtual std::string GetDescription() const = 0;

public:

    // Returns true if adapter's vendor is AMD.
    FORCE_INLINE bool IsAMD() const
    {
        return GetVendorId() == GPU_VENDOR_ID_AMD;
    }

    // Returns true if adapter's vendor is Intel.
    FORCE_INLINE bool IsIntel() const
    {
        return GetVendorId() == GPU_VENDOR_ID_INTEL;
    }

    // Returns true if adapter's vendor is Nvidia.
    FORCE_INLINE bool IsNVIDIA() const
    {
        return GetVendorId() == GPU_VENDOR_ID_NVIDIA;
    }

    // Returns true if adapter's vendor is Microsoft.
    FORCE_INLINE bool IsMicrosoft() const
    {
        return GetVendorId() == GPU_VENDOR_ID_MICROSOFT;
    }
};
