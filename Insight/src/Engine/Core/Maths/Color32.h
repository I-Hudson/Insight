#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Core/InsightAlias.h"
#include "Engine/Core/Templates.h"
#include "glm/glm.hpp"

struct Color;

class IS_API Color32
{
public:

    union
    {
        struct
        {
            /// <summary>
            /// Red component of the color.
            /// </summary>
            Byte R;

            /// <summary>
            /// Green component of the color.
            /// </summary>
            Byte G;

            /// <summary>
            /// Blue component of the color.
            /// </summary>
            Byte B;

            /// <summary>
            /// Alpha component of the color.
            /// </summary>
            Byte A;
        };

        /// <summary>
        /// Color packed into 32 bits
        /// </summary>
        U32 Raw;
    };

public:

    static Color32 Transparent;
    static Color32 Black;
    static Color32 White;

public:

    /// <summary>
    /// Empty constructor.
    /// </summary>
    Color32()
    {
    }

    /// <summary>
    /// Constructs a new Color32 with given r, g, b, a components.
    /// </summary>
    /// <param name="r">The red component value.</param>
    /// <param name="g">The green component value.</param>
    /// <param name="b">The value component value.</param>
    /// <param name="a">The alpha component value.</param>
    Color32(Byte r, Byte g, Byte b, Byte a)
    {
        R = r;
        G = g;
        B = b;
        A = a;
    }

    explicit Color32(const Color& c);
    explicit Color32(const glm::vec4& c);

public:

    std::string ToString() const;
    std::string ToHexString() const;

public:

    bool operator==(const Color32& other) const
    {
        return R == other.R && G == other.G && B == other.B && A == other.A;
    }

    bool operator!=(const Color32& other) const
    {
        return R != other.R || G != other.G || B != other.B || A != other.A;
    }

    Color32 operator+(const Color32& b) const
    {
        return Color32(R + b.R, G + b.G, B + b.B, A + b.A);
    }

    Color32 operator-(const Color32& b) const
    {
        return Color32(R - b.R, G - b.G, B - b.B, A - b.A);
    }

    Color32 operator*(const Color32& b) const
    {
        return Color32(R * b.R, G * b.G, B * b.B, A * b.A);
    }

    Color32& operator+=(const Color32& b)
    {
        R += b.R;
        G += b.G;
        B += b.B;
        A += b.A;
        return *this;
    }

    Color32& operator-=(const Color32& b)
    {
        R -= b.R;
        G -= b.G;
        B -= b.B;
        A -= b.A;
        return *this;
    }

    Color32& operator*=(const Color32& b)
    {
        R *= b.R;
        G *= b.G;
        B *= b.B;
        A *= b.A;
        return *this;
    }

    Color32 operator*(float b) const
    {
        return Color32((Byte)((float)R * b), (Byte)((float)G * b), (Byte)((float)B * b), (Byte)((float)A * b));
    }

    // Returns true if color is fully transparent (all components are equal zero).
    bool IsTransparent() const
    {
        return R + G + B + A == 0;
    }

    // Returns true if color has opacity channel in use (different from 255).
    bool HasOpacity() const
    {
        return A != 255;
    }

public:

    U32 GetAsABGR() const
    {
        return (R << 24) + (G << 16) + (B << 8) + A;
    }

    U32 GetAsBGRA() const
    {
        return (A << 24) + (R << 16) + (G << 8) + B;
    }

    U32 GetAsARGB() const
    {
        return (B << 24) + (G << 16) + (R << 8) + A;
    }

    U32 GetAsRGB() const
    {
        return (R << 16) + (G << 8) + B;
    }

    U32 GetAsRGBA() const
    {
        return Raw;
    }

public:

    U32 GetHashCode() const;

    static U32 GetHashCode(const Color32& v)
    {
        return v.GetHashCode();
    }

public:

    /// <summary>
    /// Initializes from packed RGB value of the color and separate alpha channel value.
    /// </summary>
    /// <param name="rgb">The packed RGB value.</param>
    /// <param name="a">The alpha channel.</param>
    /// <returns>The color.</returns>
    static Color32 FromRGB(U32 rgb, Byte a = 255)
    {
        return Color32(static_cast<Byte>(rgb >> 16 & 0xff), static_cast<Byte>(rgb >> 8 & 0xff), static_cast<Byte>(rgb & 0xff), a);
    }

    static Color32 FromRGBA(float r, float g, float b, float a)
    {
        return Color32(static_cast<Byte>(r * 255), static_cast<Byte>(g * 255), static_cast<Byte>(b * 255), static_cast<Byte>(a * 255));
    }

    // Gets random color with opaque alpha.
    static Color32 Random();

    /// <summary>
    /// Linearly interpolates between colors a and b by normalized weight t.
    /// </summary>
    /// <param name="a">The start value.</param>
    /// <param name="b">The end value.</param>
    /// <param name="t">The linear blend weight.</param>
    static Color32 Lerp(const Color32& a, const Color32& b, const float t)
    {
        return Color32((Byte)(a.R + (b.R - a.R) * t), (Byte)(a.G + (b.G - a.G) * t), (Byte)(a.B + (b.B - a.B) * t), (Byte)(a.A + (b.A - a.A) * t));
    }
};

inline Color32 operator*(float a, const Color32& b)
{
    return b * a;
}

template<>
struct TIsPODType<Color32>
{
    enum { Value = true };
};

inline U32 GetHash(const Color32& key)
{
    return key.GetHashCode();
}