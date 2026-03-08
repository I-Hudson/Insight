#pragma once

#include "Core/Defines.h"
#include "Core/TypeAlias.h"

namespace Insight
{
    enum class CompressionTypes
    {
        lz4,
        Count
    };
    constexpr const char* CompressionTypesStrings[] =
    {
        "lz4"
    };

    static_assert(ARRAY_COUNT(CompressionTypesStrings) == static_cast<u64>(CompressionTypes::Count));
}