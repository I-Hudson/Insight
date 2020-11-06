#include "ispch.h"
#include "Texture.h"

#include "stb_image.h"

namespace Insight
{
	namespace Render
	{
		Texture::Texture()
			: m_textureId(0)
			, m_dataSize(0)
			, m_fileName("")
			, m_texWidth(0)
			, m_texHeight(0)
			, m_texDepth(0)
			, m_texChannels(0)
			, m_textureInit(false)
		{
		}

		Texture::Texture(std::string const& filePath)
			: m_textureId(0)
			, m_dataSize(0)
			, m_fileName("")
			, m_texWidth(0)
			, m_texHeight(0)
			, m_texDepth(0)
			, m_texChannels(0)
			, m_textureInit(false)
		{
			Load(filePath);
		}

		Texture::~Texture()
		{
		}

		bool Texture::Load(std::string const& filePath, bool const& overwrite)
		{
			if (m_textureInit && !overwrite)
			{
				IS_CORE_ERROR("Texture id: {0}, file name: {1}, has allready been set and 'overwrite' is false.", m_textureId, m_fileName);
				return false;
			}

			void* pixels = stbi_load(filePath.c_str(), &m_texWidth, &m_texHeight, &m_texChannels, STBI_rgb_alpha);
			if (pixels == nullptr)
			{
				IS_CORE_ERROR("Texture failed to load. File Path: {0}.", filePath);
				return false;
			}
			m_dataSize = m_texWidth * m_texHeight * m_texChannels;

			LoadTextureAPI(pixels, m_dataSize);

#ifdef IS_DEBUG
			m_debugPixels = pixels;
#else
			stbi_image_free(pixels);
#endif
			return false;
		}

		void Texture::Release()
		{
			ReleaseTextureAPI();
#ifdef IS_DEBUG
			stbi_image_free(m_debugPixels);
#endif
		}
	}
}