#pragma once

#include "Graphics/Enums.h"
#include "Graphics/PixelFormat.h"
#include "Graphics/RHI/RHI_Resource.h"
#include <vector>

namespace Insight
{
	namespace Graphics
	{
		class RenderContext;

		struct RHI_TextureCreateInfo
		{
			TextureType TextureType = TextureType::Unknown;
			int Width = -1;
			int Height = -1;
			int Depth = -1;
			PixelFormat Format = PixelFormat::Unknown;
			ImageUsageFlags ImageUsage = 0;

			u32 Mip_Count = 1;
			u32 Layer_Count = 1;

			static RHI_TextureCreateInfo Tex2D(int width, int height, PixelFormat format, ImageUsageFlags usage)
			{
				RHI_TextureCreateInfo info = { };
				info.TextureType = TextureType::Tex2D;
				info.Width = width;
				info.Height = height;
				info.Depth = 1;
				info.Format = format;
				info.ImageUsage = usage;
				return info;
			}
			static RHI_TextureCreateInfo Tex2DArray(int width, int height, PixelFormat format, ImageUsageFlags usage, u32 layer_count)
			{
				RHI_TextureCreateInfo info = { };
				info.TextureType = TextureType::Tex2DArray;
				info.Width = width;
				info.Height = height;
				info.Depth = 1;
				info.Format = format;
				info.ImageUsage = usage;
				info.Layer_Count = layer_count;
				return info;
			}
		};

		class RHI_Texture : public RHI_Resource
		{
		public:
			static RHI_Texture* New();

			void LoadFromFile(std::string filePath);
			void LoadFromData(Byte* data, u32 width, u32 height, u32 depth, u32 channels);

			RHI_TextureCreateInfo GetInfo() const { return m_info; }
			int GetWidth()					const { return m_info.Width; }
			int GetHeight()					const { return m_info.Height; }
			int GetChannels()				const { return 4; }
			TextureType GetType()			const { return m_info.TextureType; }
			PixelFormat GetFormat()			const { return m_info.Format; }

			virtual void Create(RenderContext* context, RHI_TextureCreateInfo createInfo) = 0;

			virtual void Upload(void* data, int sizeInBytes) = 0;
			virtual std::vector<Byte> Download(void* data, int sizeInBytes) = 0;

		protected:
			RHI_TextureCreateInfo m_info = { };
		};
	}
}