// Copyright (c) 2012-2021 Wojciech Figat. All rights reserved.

#pragma once
#include "Core/InsightAlias.h"

/// <summary>
/// Contains information about current memory usage and capacity.
/// </summary>
struct MemoryStats
{
DECLARE_SCRIPTING_TYPE_MINIMAL(MemoryStats);

    /// <summary>
    /// Total amount of physical memory in bytes.
    /// </summary>
    U64 TotalPhysicalMemory;

    /// <summary>
    /// Amount of used physical memory in bytes.
    /// </summary>
    U64 UsedPhysicalMemory;

    /// <summary>
    /// Total amount of virtual memory in bytes.
    /// </summary>
    U64 TotalVirtualMemory;

    /// <summary>
    /// Amount of used virtual memory in bytes.
    /// </summary>
    U64 UsedVirtualMemory;
};

/// <summary>
/// Contains information about current memory usage by the process.
/// </summary>
struct ProcessMemoryStats
{
    /// <summary>
    /// Amount of used physical memory in bytes.
    /// </summary>
    U64 UsedPhysicalMemory;

    /// <summary>
    /// Amount of used virtual memory in bytes.
    /// </summary>
    U64 UsedVirtualMemory;
};
