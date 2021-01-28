#include "ispch.h"
#include "Color32.h"
#include "Color.h"
#include "Engine/Core/Utils.h"

Color32 Color32::Transparent(0, 0, 0, 0);
Color32 Color32::Black(0, 0, 0, 255);
Color32 Color32::White(255, 255, 255, 255);

Color32::Color32(const Color& c)
{
    R = static_cast<Byte>(c.R * 255);
    G = static_cast<Byte>(c.G * 255);
    B = static_cast<Byte>(c.B * 255);
    A = static_cast<Byte>(c.A * 255);
}

Color32::Color32(const glm::vec4& c)
{
    R = static_cast<Byte>(c.x * 255);
    G = static_cast<Byte>(c.y * 255);
    B = static_cast<Byte>(c.z * 255);
    A = static_cast<Byte>(c.w * 255);
}

std::string Color32::ToString() const
{
    return StringFormat("R:{0} G:{1} B:{2} A:{3}", R, G, B, A);
}

std::string Color32::ToHexString() const
{
    static const char* digits = "0123456789ABCDEF";

    char result[6];

    result[0] = digits[R >> 4 & 0x0f];
    result[1] = digits[R & 0x0f];

    result[2] = digits[G >> 4 & 0x0f];
    result[3] = digits[G & 0x0f];

    result[4] = digits[B >> 4 & 0x0f];
    result[5] = digits[B & 0x0f];

    return std::string(result, 6);
}

U32 Color32::GetHashCode() const
{
    U32 hashCode = (U32)R;
    hashCode = hashCode * 397 ^ (U32)G;
    hashCode = hashCode * 397 ^ (U32)B;
    hashCode = hashCode * 397 ^ (U32)A;
    return hashCode;
}

Color32 Color32::Random()
{
    return FromRGB(rand());
}
