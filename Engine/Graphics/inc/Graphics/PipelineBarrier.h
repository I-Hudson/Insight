#pragma once

#include "Graphics/Enums.h"

#include <vector>

namespace Insight
{
	namespace Graphics
	{
		class RHI_Buffer;
		class RHI_Texture;

		struct BufferBarrier
		{
			AccessFlags SrcAccessFlags;
			AccessFlags DstAccessFlags;
			RHI_Buffer* Buffer;
			u32 Offset;
			u32 Size;
		};

		struct ImageSubresourceRange
		{
			ImageAspectFlags AspectMask;
			u32 BaseMipLevel;
			u32 LevelCount;
			u32 BaseArrayLayer;
			u32 LayerCount;

			static ImageSubresourceRange SingleMipAndLayer(ImageAspectFlags aspectMask)
			{
				return ImageSubresourceRange
				{
					aspectMask,
					0,
					1,
					0,
					1
				};
			}
		};

		struct ImageBarrier
		{
			AccessFlags SrcAccessFlags;
			AccessFlags DstAccessFlags;
			ImageLayout OldLayout;
			ImageLayout NewLayout;
			RHI_Texture* Image;
			ImageSubresourceRange SubresourceRange;

			bool IsValid() const
			{
				return NewLayout != ImageLayout::Undefined;
			}
			RGTextureHandle TextureHandle;
		};

		struct PipelineBarrier
		{
			PipelineStageFlags SrcStage;
			PipelineStageFlags DstStage;

			std::vector<BufferBarrier> BufferBarriers;
			std::vector<ImageBarrier> ImageBarriers;
		};
	}
}