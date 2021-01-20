// Copyright (c) 2012-2021 Wojciech Figat. All rights reserved.

#pragma once
#include "Core/InsightAlias.h"

/// <summary>
/// Contains information about CPU (Central Processing Unit).
/// </summary>
struct CPUInfo
{
DECLARE_SCRIPTING_TYPE_MINIMAL(CPUInfo);

    /// <summary>
    /// The number of physical processor packages.
    /// </summary>
    U32 ProcessorPackageCount;

    /// <summary>
    /// The number of processor cores (physical).
    /// </summary>
    U32 ProcessorCoreCount;

    /// <summary>
    /// The number of logical processors (including hyper-threading).
    /// </summary>
    U32 LogicalProcessorCount;

    /// <summary>
    /// The size of processor L1 caches (in bytes).
    /// </summary>
    U32 L1CacheSize;

    /// <summary>
    /// The size of processor L2 caches (in bytes).
    /// </summary>
    U32 L2CacheSize;

    /// <summary>
    /// The size of processor L3 caches (in bytes).
    /// </summary>
    U32 L3CacheSize;

    /// <summary>
    /// The CPU memory page size (in bytes).
    /// </summary>
    U32 PageSize;

    /// <summary>
    /// The CPU clock speed (in Hz).
    /// </summary>
    uint64 ClockSpeed;

    /// <summary>
    /// The CPU cache line size (in bytes).
    /// </summary>
    U32 CacheLineSize;
};
