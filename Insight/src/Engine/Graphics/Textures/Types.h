#pragma once

#include "Engine/Core/Enums.h"
#include "Engine/Core/Core.h"
#include "Engine/Core/InsightAlias.h"
#include "Engine/Graphics/PixelFormat.h"

/// <summary>
/// Describes texture compression format type
/// </summary>
DECLARE_ENUM_EX_7(TextureFormatType, Byte, 0, Unknown, ColorRGB, ColorRGBA, NormalMap, GrayScale, HdrRGBA, HdrRGB);

/// <summary>
/// Texture header structure
/// </summary>
struct IS_API TextureHeader
{
    /// <summary>
    /// Top mip width in pixels
    /// </summary>
    I32 Width;

    /// <summary>
    /// Top mip height in pixels
    /// </summary>
    I32 Height;

    /// <summary>
    /// Amount of mip levels
    /// </summary>
    I32 MipLevels;

    /// <summary>
    /// Texture pixels format
    /// </summary>
    PixelFormat Format;

    /// <summary>
    /// Texture compression type
    /// </summary>
    TextureFormatType Type;

    /// <summary>
    /// True if texture is a cubemap (has 6 array slices per mip).
    /// </summary>
    bool IsCubeMap;

    /// <summary>
    /// True if disable dynamic texture streaming
    /// </summary>
    bool NeverStream;

    /// <summary>
    /// True if texture contains sRGB colors data
    /// </summary>
    bool IsSRGB;

    /// <summary>
    /// The custom data to be used per texture storage layer (faster access).
    /// </summary>
    Byte CustomData[17];
};

static_assert(sizeof(TextureHeader) == 10 * sizeof(I32), "Invalid TextureHeader size.");