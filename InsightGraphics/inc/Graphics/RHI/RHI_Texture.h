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
		};

		class RHI_Texture : public RHI_Resource
		{
		public:
			static RHI_Texture* New();

			void LoadFromFile(std::string filePath);

			int GetWidth() const			{ return m_info.Width; }
			int GetHeight() const			{ return m_info.Height; }
			int GetChannels() const			{ return 4; }
			TextureType GetType() const		{ return m_info.TextureType; }
			PixelFormat GetFormat() const	{ return m_info.Format; }

			virtual void Create(RenderContext* context, RHI_TextureCreateInfo createInfo) = 0;

			virtual void Upload(void* data, int sizeInBytes) = 0;
			virtual std::vector<Byte> Download(void* data, int sizeInBytes) = 0;

		protected:
			RHI_TextureCreateInfo m_info = { };
		};
	}
}