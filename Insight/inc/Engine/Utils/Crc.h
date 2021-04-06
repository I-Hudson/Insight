// Copyright (c) 2012-2021 Wojciech Figat. All rights reserved.

#pragma once

#include "Engine/Core/InsightAlias.h"
#include "Engine/Core/Templates.h"

// The utilities for CRC hash generation.
class Crc
{
public:

    // Helper lookup table with cached CRC values.
    static U32 CachedCRCTablesSB8[8][256];

    // Initializes the CRC lookup table. Must be called before any of the CRC functions are used.
    static void Init();

    // Generates CRC hash of the memory area
    static U32 MemCrc32(const void* data, I32 length, U32 crc = 0);
};
