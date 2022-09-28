#pragma once

#include "Resource/Resource.h"
#include "Graphics/PixelFormat.h"

namespace Insight
{
	namespace Runtime
	{
		class Texture : public IResource
		{
		public:

			u32 GetWidth() const;
			u32 GetHeight() const;
			u32 GetDepth() const;
			PixelFormat GetFormat() const;

		private:
			/// @brief Handle loading the resource from disk.
			/// @param file_path 
			virtual void Load(std::string file_path) override;
			/// @brief Handle unloading the resource from memory.
			virtual void UnLoad() override;

		private:
			u32 m_width = 0;
			u32 m_height = 0;
			u32 m_depth = 0;
			PixelFormat m_format;
		};
	}
}