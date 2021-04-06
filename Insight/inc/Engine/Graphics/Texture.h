#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Core/DerivedFrom.h"
#include "GPUResource.h"
#include "PixelFormat.h"

namespace FileSystem
{
	class FileSystemManager;
}

namespace Render
{
	struct IS_API TextureDescription
	{
		U32 TextureId;
		std::string FileName;
		std::string FilePath;
		std::string Extension;

		void* Data;
		U32 TexWidth;
		U32 TexHeight;
		U32 TexDepth;

		U32 TexChannels;
		U32 MipLevels;
		U32 SizeBytes;

		PixelFormat Format;
		bool HasBeenDestroyed;

		TextureDescription()
			: TextureId(-1), FileName(""), FilePath("")
			, Data(nullptr), TexWidth(0), TexHeight(0), TexDepth(0)
			, TexChannels(0), SizeBytes(0), MipLevels(0)
			, Format(PixelFormat::Unknown), HasBeenDestroyed(false)
		{ }
	};

	class IS_API Texture : public Object
	{
	public:
		Texture();
		virtual ~Texture();

		void Init(const std::string& filePath);
		void Init(const TextureDescription& desc);
		virtual bool IsValid() = 0;

		U32 const& GetId() { return m_desc.TextureId; }
		std::string const& GetFileName() const { return m_desc.FileName; }
		std::string const& GetFilePath() const { return m_desc.FilePath; }

		int const& GetWidth() const { return m_desc.TexWidth; }
		int const& GetHeight() const { return m_desc.TexHeight; }
		int const& GetDepth() const { return m_desc.TexDepth; }
		int const& GetChannels() const { return m_desc.TexChannels; }
		U32 const& GetSizeBytes() const { return m_desc.SizeBytes; }
		PixelFormat const& GetFormat() const { return m_desc.Format; }

		TextureDescription const& GetDescription() const { return m_desc; }

		static Texture* New();

	protected:
		U32 GetMipMapCount(const U32& width, const U32& height, const U32& channels);
		U64 GetMipMapOffset(const U32& width, const U32& height, const U32& channels, const U16& mipMaps);
		U64 GetImageBufferSize(const U32& width, const U32& height, const U32& channels, const U16& mipMaps);

		virtual void CreateGPUResource() = 0;

	protected:
		TextureDescription m_desc;
	};
}