#pragma once

#include "Resource/Loaders/IResourceLoader.h"
#include "Resource/Texture2D.h"

namespace Insight
{

	namespace Runtime
	{
		struct LoadPixelData
		{
			std::vector<Byte> Data;
			int Width;
			int Height;
			int Depth;
			int Channels;
		};

		constexpr static const char* c_TextureLoaderFileExtensions[] =
		{
			".png"
			, ".jpeg"
			, ".jpg"
			, ".qoi"
		};

		class TextureLoader : public IResourceLoader
		{
		public:
			TextureLoader();
			virtual ~TextureLoader() override;

			virtual bool Load(IResource* resource) const override;

			LoadPixelData LoadPixels(std::string_view filePath, TextureDiskFormat diskFormat) const;
		};
	}
}