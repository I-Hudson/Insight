#include "ispch.h"
#include "Texture.h"

#include "Module/GraphicsModule.h"
#include "FileSystem/FileSystem.h"
#include "Platform/Vulkan/VulkanTexture.h"
#include "stb_image.h"

namespace Insight
{
	namespace Render
	{
		U32 TextureGPUData::GetMipMapCount(const U32& width, const U32& height, const U32& channels)
		{
			const U32 smallestMipMapSize = 32;
			U32 i = 0;
			for (;;)
			{
				U32 w = width >> i;
				U32 h = height >> i;

				if (w < smallestMipMapSize || h < smallestMipMapSize)
				{
					break;
				}
				++i;
			}
			return i;
		}

		U64 TextureGPUData::GetMipMapOffset(const U32& width, const U32& height, const U32& channels, const U16& mipMaps)
		{
			U64 dataSize = 0;
			//width* height* channels;
			for (size_t i = 0; i < mipMaps; ++i)
			{
				U32 w = width  >> i;
				U32 h = height >> i;

				dataSize += w * h * channels;
			}
			return dataSize;
		}

		U64 TextureGPUData::GetImageBufferSize(const U32& width, const U32& height, const U32& channels, const U16& mipMaps)
		{
			U64 dataSize = 0;
			//width* height* channels;
			for (size_t i = 0; i <= mipMaps; ++i)
			{
				U32 w = width >> i;
				U32 h = height >> i;
				dataSize += w * h * channels;
			}
			return dataSize;
		}

		SharedPtr<TextureGPUData> TextureGPUData::Create()
		{
			switch (Module::GraphicsModule::Instance()->GetAPI())
			{
				case GraphicsAPI::Vulkan: return CreateSharedPtr<vks::VulkanTextureGPUData>();
			//case GraphicsAPI::OpenGL: return CreateSharedPtr<OpenGLTextureGPUData>();
			}
		}

		Texture::Texture()
			: m_textureId(0)
			, m_dataSize(0)
			, m_fileName("")
			, m_texWidth(0)
			, m_texHeight(0)
			, m_texDepth(0)
			, m_texChannels(0)
		{ }

		Texture::Texture(std::string const& filePath)
			: m_textureId(0)
			, m_dataSize(0)
			, m_fileName("")
			, m_filePath(filePath)
			, m_texWidth(0)
			, m_texHeight(0)
			, m_texDepth(0)
			, m_texChannels(0)
		{
			if (!std::filesystem::exists(std::filesystem::path(filePath)))
			{
				IS_CORE_ERROR("[Texture::Texture] File was not found. '{0}'", filePath);
			}
			else
			{
				int x, y, c;
				void* data = stbi_load(filePath.c_str(), &x, &y, &c, STBI_rgb_alpha);
				if (data)
				{
					m_dataSize = x * y * 4;
					m_gpuData = TextureGPUData::Create();
					m_gpuData->Init(data, m_dataSize, x, y, 4);
				}
				else
				{
					IS_CORE_ERROR("[Texture::Texture] Texture was unable to load.");
				}
				stbi_image_free(data);
			}
		}

		Texture::~Texture()
		{

		}
	}
}