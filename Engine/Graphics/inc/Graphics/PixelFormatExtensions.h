#pragma once

#include "Graphics/Defines.h"
#include "PixelFormat.h"

//// <summary>
//// Extensions to <see cref="PixelFormat"/>.
//// </summary>
class PixelFormatExtensions
{
public:

    //// <summary>
    //// Initializes cache.
    //// </summary>
    IS_GRAPHICS static void Init();

public:

    //// <summary>
    //// Calculates the size of a <see cref="PixelFormat"/> in bytes.
    //// </summary>
    //// <param name="format">The Pixel format.</param>
    //// <returns>size of in bytes</returns>
    IS_GRAPHICS static u32 SizeInBytes(const PixelFormat format)
    {
        return SizeInBits(format) / 8;
    }

    //// <summary>
    //// Calculates the size of a <see cref="PixelFormat"/> in bits.
    //// </summary>
    //// <param name="format">The pixel format.</param>
    //// <returns>The size in bits</returns>
    IS_GRAPHICS static u32 SizeInBits(PixelFormat format);

    //// <summary>
    //// Calculate the size of the alpha channel in bits depending on the pixel format.
    //// </summary>
    //// <param name="format">The pixel format</param>
    //// <returns>The size in bits</returns>
    IS_GRAPHICS static u32 AlphaSizeInBits(PixelFormat format);

    //// <summary>
    //// Determines whether the specified <see cref="PixelFormat"/> contains alpha channel.
    //// </summary>
    //// <param name="format">The Pixel Format.</param>
    //// <returns><c>true</c> if the specified <see cref="PixelFormat"/> has alpha; otherwise, <c>false</c>.</returns>
    IS_GRAPHICS static bool HasAlpha(const PixelFormat format)
    {
        return AlphaSizeInBits(format) != 0;
    }

    //// <summary>
    //// Determines whether the specified <see cref="PixelFormat"/> is depth stencil.
    //// </summary>
    //// <param name="format">The Pixel Format.</param>
    //// <returns><c>true</c> if the specified <see cref="PixelFormat"/> is depth stencil; otherwise, <c>false</c>.</returns>
    IS_GRAPHICS static bool IsDepthStencil(PixelFormat format);

    //// <summary>
    //// Determines whether the specified <see cref="PixelFormat"/> is depth.
    //// </summary>
    //// <param name="format"></param>
    //// <returns></returns>
    IS_GRAPHICS static bool IsDepth(PixelFormat format);

    //// <summary>
    //// Determines whether the specified <see cref="PixelFormat"/> has stencil bits.
    //// </summary>
    //// <param name="format">The Pixel Format.</param>
    //// <returns><c>true</c> if the specified <see cref="PixelFormat"/> has stencil bits; otherwise, <c>false</c>.</returns>
    IS_GRAPHICS static bool HasStencil(PixelFormat format);

    //// <summary>
    //// Determines whether the specified <see cref="PixelFormat"/> is Typeless.
    //// </summary>
    //// <param name="format">The <see cref="PixelFormat"/>.</param>
    //// <param name="partialTypeless">Enable/disable partially typeless formats.</param>
    //// <returns><c>true</c> if the specified <see cref="PixelFormat"/> is Typeless; otherwise, <c>false</c>.</returns>
    IS_GRAPHICS static bool IsTypeless(PixelFormat format, bool partialTypeless);

    //// <summary>
    //// Returns true if the <see cref="PixelFormat"/> is valid.
    //// </summary>
    //// <param name="format">A format to validate</param>
    //// <returns>True if the <see cref="PixelFormat"/> is valid.</returns>
    IS_GRAPHICS static bool IsValid(PixelFormat format);

    //// <summary>
    //// Returns true if the <see cref="PixelFormat"/> is a compressed format.
    //// </summary>
    //// <param name="format">The format to check for compressed.</param>
    //// <returns>True if the <see cref="PixelFormat"/> is a compressed format.</returns>
    IS_GRAPHICS static bool IsCompressed(PixelFormat format);

    //// <summary>
    //// Returns true if the <see cref="PixelFormat"/> is a compressed format from BC formats family (BC1, BC2, BC3, BC4, BC5, BC6H, BC7).
    //// </summary>
    //// <param name="format">The format to check for compressed.</param>
    //// <returns>True if the <see cref="PixelFormat"/> is a compressed format from BC formats family.</returns>
    IS_GRAPHICS static bool IsCompressedBC(PixelFormat format);

    //// <summary>
    //// Determines whether the specified <see cref="PixelFormat"/> is packed.
    //// </summary>
    //// <param name="format">The Pixel Format.</param>
    //// <returns><c>true</c> if the specified <see cref="PixelFormat"/> is packed; otherwise, <c>false</c>.</returns>
    IS_GRAPHICS static bool IsPacked(PixelFormat format);

    //// <summary>
    //// Determines whether the specified <see cref="PixelFormat"/> is planar.
    //// </summary>
    //// <param name="format">The Pixel Format.</param>
    //// <returns><c>true</c> if the specified <see cref="PixelFormat"/> is planar; otherwise, <c>false</c>.</returns>
    IS_GRAPHICS static bool IsPlanar(PixelFormat format);

    //// <summary>
    //// Determines whether the specified <see cref="PixelFormat"/> is video.
    //// </summary>
    //// <param name="format">The <see cref="PixelFormat"/>.</param>
    //// <returns><c>true</c> if the specified <see cref="PixelFormat"/> is video; otherwise, <c>false</c>.</returns>
    IS_GRAPHICS static bool IsVideo(PixelFormat format);

    //// <summary>
    //// Determines whether the specified <see cref="PixelFormat"/> is a sRGB format.
    //// </summary>
    //// <param name="format">The <see cref="PixelFormat"/>.</param>
    //// <returns><c>true</c> if the specified <see cref="PixelFormat"/> is a sRGB format; otherwise, <c>false</c>.</returns>
    IS_GRAPHICS static bool IsSRGB(PixelFormat format);

    //// <summary>
    //// Determines whether the specified <see cref="PixelFormat"/> is HDR (either 16 or 32bits Float)
    //// </summary>
    //// <param name="format">The format.</param>
    //// <returns><c>true</c> if the specified pixel format is HDR (Floating poInt); otherwise, <c>false</c>.</returns>
    IS_GRAPHICS static bool IsHDR(PixelFormat format);

    //// <summary>
    //// Determines whether the specified format is in RGBA order.
    //// </summary>
    //// <param name="format">The format.</param>
    //// <returns><c>true</c> if the specified format is in RGBA order; otherwise, <c>false</c>.</returns>
    IS_GRAPHICS static bool IsRgbAOrder(PixelFormat format);

    //// <summary>
    //// Determines whether the specified format is in BGRA order.
    //// </summary>
    //// <param name="format">The format.</param>
    //// <returns><c>true</c> if the specified format is in BGRA order; otherwise, <c>false</c>.</returns>
    IS_GRAPHICS static bool IsBGRAOrder(PixelFormat format);

    //// <summary>
    //// Determines whether the specified format contains normalized data. It indicates that values stored in an integer format are to be mapped to the range [-1,1] (for signed values) or [0,1] (for unsigned values) when they are accessed and converted to floating point.
    //// </summary>
    //// <param name="format">The <see cref="PixelFormat"/>.</param>
    //// <returns>True if given format contains normalized data type, otherwise false.</returns>
    IS_GRAPHICS static bool IsNormalized(PixelFormat format);

    //// <summary>
    //// Determines whether the specified format is integer data type (signed or unsigned).
    //// </summary>
    //// <param name="format">The <see cref="PixelFormat"/>.</param>
    //// <returns>True if given format contains integer data type (signed or unsigned), otherwise false.</returns>
    IS_GRAPHICS static bool IsInteger(PixelFormat format);

    //// <summary>
    //// Computes the scanline count (number of scanlines).
    //// </summary>
    //// <param name="format">The <see cref="PixelFormat"/>.</param>
    //// <param name="height">The height.</param>
    //// <returns>The scanline count.</returns>
    IS_GRAPHICS static u32 ComputeScanlineCount(PixelFormat format, int height);

    //// <summary>
    //// Computes the format components count (number of R, G, B, A channels).
    //// </summary>
    //// <param name="format">The <see cref="PixelFormat"/>.</param>
    //// <returns>The components count.</returns>
    IS_GRAPHICS static u32 ComputeComponentsCount(PixelFormat format);

    //// <summary>
    //// Finds the equivalent sRGB format to the provided format.
    //// </summary>
    //// <param name="format">The non sRGB format.</param>
    //// <returns>The equivalent sRGB format if any, the provided format else.</returns>
    IS_GRAPHICS static PixelFormat TosRGB(PixelFormat format);

    //// <summary>
    //// Finds the equivalent non sRGB format to the provided sRGB format.
    //// </summary>
    //// <param name="format">The non sRGB format.</param>
    //// <returns>The equivalent non sRGB format if any, the provided format else.</returns>
    IS_GRAPHICS static PixelFormat ToNonsRGB(PixelFormat format);

    //// <summary>
    //// Converts the format to typeless.
    //// </summary>
    //// <param name="format">The format.</param>
    //// <returns>The typeless format.</returns>
    IS_GRAPHICS static PixelFormat MakeTypeless(PixelFormat format);

    //// <summary>
    //// Converts the typeless format to float.
    //// </summary>
    //// <param name="format">The typeless format.</param>
    //// <returns>The float format.</returns>
    IS_GRAPHICS static PixelFormat MakeTypelessFloat(PixelFormat format);

    //// <summary>
    //// Converts the typeless format to unorm.
    //// </summary>
    //// <param name="format">The typeless format.</param>
    //// <returns>The unorm format.</returns>
    IS_GRAPHICS static PixelFormat MakeTypelessUNorm(PixelFormat format);

public:

    IS_GRAPHICS static PixelFormat FindShaderResourceFormat(PixelFormat format, bool bSRGB);
    IS_GRAPHICS static PixelFormat FindUnorderedAccessFormat(PixelFormat format);
    IS_GRAPHICS static PixelFormat FindDepthStencilFormat(PixelFormat format);
};
