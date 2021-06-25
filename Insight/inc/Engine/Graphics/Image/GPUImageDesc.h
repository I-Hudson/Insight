#pragma once

#include "Engine/Core/Common.h"
#include "Engine/Graphics/Enums.h"
#include "Engine/Graphics/PixelFormatExtensions.h"
#include "stb_image.h"
#include "Engine/Utils/Hasher.h"
#include "Engine/FileSystem/File.h"

namespace Insight::Graphics
{
	class GPUImage;

	struct GPUSamplerDesc
	{
		SamplerFilter MagFilter = SamplerFilter::Linear;
		SamplerFilter MinFilter = SamplerFilter::Linear;
		SamplerMipmapMode MipmapMode = SamplerMipmapMode::Linear;
		SamplerAddressMode AddressModeU = SamplerAddressMode::Mirrored_Repeat;
		SamplerAddressMode AddressModeV = SamplerAddressMode::Mirrored_Repeat;
		SamplerAddressMode AddressModeW = SamplerAddressMode::Mirrored_Repeat;
		float MipLodBias = 0.0f;
		CompareOp CompareOP = CompareOp::Never;
		float MinLod = 0.0f;
		float MaxLoad = 0.0f;
		float MaxAnisotropy = 1.0f;
		bool AnisortopyEnable = false;
		BorderColor BorderColor = BorderColor::Float_Opaque_White;

		u64 Hash()
		{
			Utils::Hasher hasher;
			hasher.Hash(MagFilter);
			hasher.Hash(MinFilter);
			hasher.Hash(MipmapMode);
			hasher.Hash(AddressModeU);
			hasher.Hash(AddressModeV);
			hasher.Hash(AddressModeW);
			hasher.Hash(MipLodBias);
			hasher.Hash(CompareOP);
			hasher.Hash(MinLod);
			hasher.Hash(MaxLoad);
			hasher.Hash(MaxAnisotropy);
			hasher.Hash(AnisortopyEnable);
			hasher.Hash(BorderColor);
			return hasher.GetHash();
		}
	};

	enum class GPUImageViewType
	{
		Type_1D = 0,
		Type_2D = 1,
		Type_3D = 2,
		Type_Cube = 3,
		Type_1D_Array = 4,
		Type_2D_Array = 5,
		Type_Cube_Array = 6,

		Default,
	};

	struct GPUImageViewDesc
	{
		GPUImageViewDesc(GPUImage* image)
			: Image(image), ViewType(GPUImageViewType::Default)
		{ }
		GPUImageViewDesc(GPUImage* image, GPUImageViewType viewType)
			: Image(image), ViewType(viewType)
		{ }

		GPUImage* Image;
		GPUImageViewType ViewType;
	};

	struct IS_API GPUImageDesc
	{
		GPUImageDesc()
			: Width(0), Height(0)
		{ }

		GPUImageDesc(u32 width, u32 height, u32 depth, u32 levels, SampleLevel samples, u32 layers,
					 ImageDomain domain, ImageLayout initLayout, ImageLayout finalLayout, ImageUsageFlags usageFlags, ImageMiscFlags miscFlags,
					 ImageCreateFlags createFlags, PixelFormat format, ImageType type, ImageUsageType imageUsageType,
					 void* data)
			: Width(width), Height(height), Depth(depth), Levels(levels), Samples(samples), Layers(layers)
			, Domain(domain), InitLayout(initLayout), FinalLayout(finalLayout), Usage(usageFlags), MiscFlags(miscFlags)
			, CreateFlags(createFlags), Format(format), Type(type), UsageType(imageUsageType)
			, Data(data)
		{
			u32 formatSize = PixelFormatExtensions::SizeInBytes(format);
			Size = width * height * formatSize;
		}

		u32 Width;
		u32 Height;
		u32 Depth;
		u32 Levels; // mipmaps
		SampleLevel Samples;
		u32 Layers;

		ImageDomain Domain;
		ImageLayout InitLayout;
		ImageLayout FinalLayout;
		ImageUsageFlags Usage;
		ImageMiscFlags MiscFlags;
		ImageCreateFlags CreateFlags;
		PixelFormat Format;
		ImageType Type;
		ImageUsageType UsageType;
		void* Data;
		u32 Size;
		GPUSamplerDesc Sampler;

		static GPUImageDesc Image2D(u32 width, u32 height, u32 depth, u32 levels, SampleLevel samples, u32 layers,
									ImageDomain domain, ImageLayout initLayout, ImageLayout finalLayout, ImageUsageFlags usageFlags, ImageMiscFlags miscFlags,
									ImageCreateFlags createFlags, PixelFormat format, ImageType type, ImageUsageType imageUsageType,
									void* data);

		static GPUImageDesc Texture(u32 width, u32 height, u32 levels, SampleLevel samples, PixelFormat format, void* data)
		{
			return Image2D(width, height, 1, levels, samples, 1, ImageDomain::Physical, ImageLayout::Undefined, ImageLayout::Shader_Read_Only, ImageUsageFlagsBits::Transfer_Dst | ImageUsageFlagsBits::Sampled, 0,
						   0, format, ImageType::Image_2D, ImageUsageType::Texture, data);
		}

		static GPUImageDesc Texture(u32 levels, SampleLevel samples, PixelFormat format, std::string const& dataPath)
		{
			int x, y, c;
			void* data = stbi_load(File::WindowsToUinxFilePath(dataPath).c_str(), &x, &y, &c, STBI_rgb_alpha);
			if (!data)
			{
				IS_CORE_ERROR("[GPUImageDesc::Texture] stbi couldn't load. '{0}'", stbi_failure_reason());
				Image2D(1, 1, 1, levels, samples, 1, ImageDomain::Physical, ImageLayout::Undefined, ImageLayout::Shader_Read_Only, ImageUsageFlagsBits::Transfer_Dst | ImageUsageFlagsBits::Sampled, 0,
						0, format, ImageType::Image_2D, ImageUsageType::Texture, nullptr);
			}
			return Image2D(x, y, 1, levels, samples, 1, ImageDomain::Physical, ImageLayout::Undefined, ImageLayout::Shader_Read_Only, ImageUsageFlagsBits::Transfer_Dst | ImageUsageFlagsBits::Sampled, 0,
						   0, format, ImageType::Image_2D, ImageUsageType::Texture, data);
		}

		static GPUImageDesc RenderTarget(u32 width, u32 height, PixelFormat format)
		{
			return Image2D(width, height, 1, 1, SampleLevel::None, 1, ImageDomain::Physical,
						   ImageLayout::Undefined,
						   PixelFormatExtensions::IsDepthStencil(format) ?
						   ImageLayout::Depth_Stencil_Attachment :
						   ImageLayout::Undefined,
						   (PixelFormatExtensions::IsDepthStencil(format) ? ImageUsageFlagsBits::Depth_Stencil_Attachment :
							ImageUsageFlagsBits::Color_Attachment) |
						   ImageUsageFlagsBits::Transfer_Src | ImageUsageFlagsBits::Transfer_Dst,
						   0,
						   0, format, ImageType::Image_2D, ImageUsageType::Render_Target, nullptr);
		}

		static GPUImageDesc TransientRenderTarget(u32 width, u32 height, PixelFormat format)
		{
			return Image2D(width, height, 1, 1, SampleLevel::None, 1, ImageDomain::Transient, ImageLayout::Undefined, ImageLayout::Undefined,
						   (PixelFormatExtensions::IsDepthStencil(format) ? ImageUsageFlagsBits::Depth_Stencil_Attachment :
							ImageUsageFlagsBits::Color_Attachment) |
						   ImageUsageFlagsBits::Input_Attachment,
						   0,
						   0, format, ImageType::Image_2D, ImageUsageType::Transient_Render_Target, nullptr);
		}

		static GPUImageDesc SwapchainImage(u32 width, u32 height, PixelFormat format, void* image)
		{
			return Image2D(width, height, 1, 1, SampleLevel::None, 1, ImageDomain::Physical, 
						   ImageLayout::Color_Attachment, ImageLayout::Undefined, ImageUsageFlagsBits::Color_Attachment, 0, 0,
						format, ImageType::Image_2D, ImageUsageType::Swapchain_Image, image) ;
		}

		bool IsTransient() const { return Domain == ImageDomain::Transient; }

		u32 ImageTypeToInt() { return (u32)Type + 1; }

		bool IsEmpty() const
		{
			return Width == 0 &&
				Height == 0;
		}
	};
}