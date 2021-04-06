#include "ispch.h"
#include "Engine/Graphics/Texture.h"

#include "Engine/Platform/Platform.h"
#include "Engine/Module/GraphicsModule.h"
#include "Engine/FileSystem/FileSystem.h"
#include "Platform/Vulkan/VulkanTexture.h"
#include "stb_image.h"

namespace Render
{
	Texture::Texture()
		: m_desc(TextureDescription())
	{
		SetType<Texture>();
	}

	Texture::~Texture()
	{
		ASSERT(m_desc.HasBeenDestroyed);
	}

	void Texture::Init(const std::string& filePath)
	{
		m_desc.FilePath = filePath;
		Init(m_desc);
	}

	void Texture::Init(const TextureDescription& desc)
	{
		std::string path = FileSystem::FileSystemManager::Instance()->FormatFilePathStringToUNIX(desc.FilePath);

		if (!std::filesystem::exists(std::filesystem::path(path)))
		{
			IS_CORE_ERROR("[Texture::Texture] File was not found. '{0}'.", path);
			path = "./data/shaders/undefinedTexture.png";
		}

		m_desc.FilePath = path;
		m_desc.FileName = path.substr(path.find_last_of('/'), path.find_last_of('.'));
		m_desc.Extension = path.substr(path.find_last_of('.'));

		int x, y, c;
		void* data = stbi_load(path.c_str(), &x, &y, &c, STBI_rgb_alpha);
		if (data)
		{
			m_desc.TexWidth = x;
			m_desc.TexHeight = y;
			m_desc.TexChannels = c;
			m_desc.TexDepth = 1;

			m_desc.SizeBytes = x * y * 4;
			m_desc.MipLevels = 1;//GetMipMapCount(x, y, c);
			m_desc.Data = data;
			CreateGPUResource();
		}
		else
		{
			IS_CORE_ERROR("[Texture::Texture] Texture was unable to load.");
		}
		m_desc.Data = nullptr;
		stbi_image_free(data);
	}

	Texture* Texture::New()
	{
		switch (Module::GraphicsModule::Instance()->GetAPI())
		{
			case GraphicsRendererAPI::Vulkan: return ::New<vks::VulkanTexture>();
		}
		ASSERT(false);
	}

	U32 Texture::GetMipMapCount(const U32& width, const U32& height, const U32& channels)
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

	U64 Texture::GetMipMapOffset(const U32& width, const U32& height, const U32& channels, const U16& mipMaps)
	{
		U64 dataSize = 0;
		//width* height* channels;
		for (size_t i = 0; i < mipMaps; ++i)
		{
			U32 w = width >> i;
			U32 h = height >> i;

			dataSize += w * h * channels;
		}
		return dataSize;
	}

	U64 Texture::GetImageBufferSize(const U32& width, const U32& height, const U32& channels, const U16& mipMaps)
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
}