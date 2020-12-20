#pragma once
#include "Insight/Core/Core.h"
#include "Insight/Core/Object.h"

namespace Insight
{
	namespace Render
	{
		class IS_API Texture : public Object
		{
		public:
			Texture();
			Texture(std::string const& filePath);
			virtual ~Texture();

			bool Load(std::string const & filePath, bool const & overwrite = false);
			void Release();

			U32 const& GetId() { return m_textureId; }
			U32 const& GetDataSize() const { return m_dataSize; }
			int const& GetWidth() const { return m_texWidth; }
			int const& GetHeight() const { return m_texHeight; }
			int const& GetDepth() const { return m_texDepth; }
			int const& GetChannels() const { return m_texChannels; }
			std::string const& GetFileName() const { return m_fileName; }

		private:
			virtual bool LoadTextureAPI(void* data, U32 const & dataSize) = 0;
			virtual void ReleaseTextureAPI() = 0;

		protected:
			U32 m_textureId;

			U32 m_dataSize;
			std::string m_fileName;

			int m_texWidth;
			int m_texHeight;
			int m_texDepth;
			int m_texChannels;

#ifdef IS_DEBUG
			void* m_debugPixels;
#endif

			bool m_textureInit;
		};

	}
}