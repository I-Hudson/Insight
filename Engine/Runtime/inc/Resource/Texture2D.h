#pragma once

#include "Resource/Texture.h"

namespace Insight
{
	namespace Runtime
	{
		class TextureLoader;

		class IS_RUNTIME Texture2D : public Texture
		{
			REGISTER_RESOURCE(Texture2D);
		public:
			Texture2D(std::string_view filePath);
			virtual ~Texture2D() override;

			/// @brief Handle loading a resource from memory. This is called for a resource which is a "sub resource" of another one
			/// and exists inside another resource's disk file.
			/// @param file_path 
			virtual void LoadFromMemory(const void* data, u64 size_in_bytes) override;

		private:
			/// @brief Handle loading the resource from disk.
			/// @param file_path 
			virtual void Load() override;

			/// @brief Handle unloading the resource from memory.
			virtual void UnLoad();

			/// @brief Save resrouce to disk.
			virtual void Save(const std::string& file_path);

		private:
			void OnRHITextureUploadCompleted(Graphics::RHI_Texture* rhiTexture);

			friend TextureLoader;
		};
	}
}