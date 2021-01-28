#include "ispch.h"
#include "Engine/Core/Maths/Maths.h"

void Maths::SinCos(float angle, float& sine, float& cosine)
{
    sine = sin(angle);
    cosine = cos(angle);
}

U32 Maths::FloorLog2(U32 value)
{
    // References:
    // http://codinggorilla.domemtech.com/?p=81
    // http://en.wikipedia.org/wiki/Binary_logarithm

    U32 pos = 0;
    if (value >= 1 << 16)
    {
        value >>= 16;
        pos += 16;
    }
    if (value >= 1 << 8)
    {
        value >>= 8;
        pos += 8;
    }
    if (value >= 1 << 4)
    {
        value >>= 4;
        pos += 4;
    }
    if (value >= 1 << 2)
    {
        value >>= 2;
        pos += 2;
    }
    if (value >= 1 << 1)
    {
        pos += 1;
    }
    return value == 0 ? 0 : pos;
}