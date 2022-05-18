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

		class RHI_Texture : public RHI_Resource
		{
		public:
			static RHI_Texture* New();

			void LoadFromFile(std::string filePath);

			int GetWidth() const			{ return m_width; }
			int GetHeight() const			{ return m_height; }
			int GetChannels() const			{ return m_channels; }
			TextureType GetType() const		{ return m_type; }
			PixelFormat GetFormat() const	{ return m_format; }

			virtual void Create(RenderContext* context, TextureType textureType, int width, int height, int channels) = 0;

			virtual void Upload(void* data, int sizeInBytes) = 0;
			virtual std::vector<Byte> Download(void* data, int sizeInBytes) = 0;

		protected:
			int m_width = 0;
			int m_height = 0;
			int m_channels = 0;
			TextureType m_type = TextureType::Unknown;
			PixelFormat m_format = PixelFormat::Unknown;
		};
	}
}