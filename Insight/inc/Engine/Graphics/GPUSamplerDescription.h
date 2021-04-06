#pragma once

/// <summary>
/// GPU sampler filter modes.
/// </summary>
enum class GPUSamplerFilter
{
    Point = 0,
    Bilinear = 1,
    Trilinear = 2,
    Anisotropic = 3,

    MAX
};

/// <summary>
/// GPU sampler address modes.
/// </summary>
enum class GPUSamplerAddressMode
{
    Wrap = 0,
    Clamp = 1,
    Mirror = 2,
    Border = 3,

    MAX
};

/// <summary>
/// GPU sampler comparision function types.
/// </summary>
enum class GPUSamplerCompareFunction
{
    Never = 0,
    Less = 1,

    MAX
};

/// <summary>
/// GPU sampler border color types.
/// </summary>
enum class GPUSamplerBorderColor
{
    /// <summary>
    /// Indicates black, with the alpha component as fully transparent.
    /// </summary>
    TransparentBlack = 0,

    /// <summary>
    /// Indicates black, with the alpha component as fully opaque.
    /// </summary>
    OpaqueBlack = 1,

    /// <summary>
    /// Indicates white, with the alpha component as fully opaque.
    /// </summary>
    OpaqueWhite = 2,

    Maximum
};
