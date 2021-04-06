#pragma once

#include "Engine/Core/Common.h"
#include "Engine/Graphics/Enums.h"
#include "Engine/Graphics/PixelFormat.h"

struct IS_API GPUImageDescription
{
	U32 Width = 0;
	U32 Height = 0;
	U32 Depth = 0;
	U32 Levels = 0; // mipmaps
	U32 Samples = 0;
	U32 Layers = 0;

	ImageDomain Domain = ImageDomain::Physical;
	ImageLayout InitLayout = ImageLayout::Undefined;
	ImageUsageFlags Usage = 0;
	ImageMiscFlags MiscFlags = 0;
	ImageCreateFlags CreateFlags = 0;
	PixelFormat Format = PixelFormat::Unknown;
	ImageType Type = ImageType::Image_2D;

	static GPUImageDescription Image2D(U32 width, U32 height, U32 depth, U32 levels, U32 samples, U32 layers,
		ImageLayout initLayout, ImageUsageFlags usage, PixelFormat format);

	static GPUImageDescription RenderTarget(U32 width, U32 height, PixelFormat format);

	static GPUImageDescription TransientRenderTarget(U32 width, U32 height, PixelFormat format);

	bool IsTransient() const { return Domain == ImageDomain::Transient; }

	U32 ImageTypeToInt() { return (U32)Type + 1; }

	bool IsEmpty() const 
	{
		return Width == 0 &&
			Height == 0 &&
			Depth == 0 &&
			Levels == 0 &&
			Samples == 0 &&
			Layers == 0 &&
			Domain == ImageDomain::Physical &&
			InitLayout == ImageLayout::Undefined &&
			Usage == 0 &&
			MiscFlags == 0 &&
			CreateFlags == 0 &&
			Format == PixelFormat::Unknown &&
			Type == ImageType::Image_2D;
	}
};