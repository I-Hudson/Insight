#pragma once
#include "Insight/Core/Core.h"
#include "Insight/Core/Object.h"

namespace Insight
{
	namespace FileSystem
	{
		class FileSystemManager;
	}

	namespace Render
	{
		class IS_API TextureGPUData
		{
		public:
			virtual ~TextureGPUData() { };
			virtual void Init(void* textureData, const U32& textureDataSize, const U32& width, const U32& height, const U32& channels) = 0;

			U32 GetMipMapCount(const U32& width, const U32& height, const U32& channels);
			U64 GetMipMapOffset(const U32& width, const U32& height, const U32& channels, const U16& mipMaps);
			U64 GetImageBufferSize(const U32& width, const U32& height, const U32& channels, const U16& mipMaps);

			static SharedPtr<TextureGPUData> Create();
		};

		class IS_API Texture : public Object
		{
		public:
			Texture();
			Texture(std::string const& filePath);
			virtual ~Texture() override;
			virtual bool IsValid() { return m_gpuData.get() != nullptr; }

			U32 const& GetId() { return m_textureId; }
			U32 const& GetDataSize() const { return m_dataSize; }
			int const& GetWidth() const { return m_texWidth; }
			int const& GetHeight() const { return m_texHeight; }
			int const& GetDepth() const { return m_texDepth; }
			int const& GetChannels() const { return m_texChannels; }
			std::string const& GetFileName() const { return m_fileName; }
			std::string const& GetFilePath() const { return m_filePath; }

			const SharedPtr<TextureGPUData> GetGPUTextureData() { return m_gpuData; }

		protected:
			U32 m_textureId;

			U32 m_dataSize;
			std::string m_fileName;
			std::string m_filePath;

			int m_texWidth;
			int m_texHeight;
			int m_texDepth;
			int m_texChannels;
			SharedPtr<TextureGPUData> m_gpuData;
		};
	}
}