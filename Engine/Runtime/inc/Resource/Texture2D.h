#pragma once

#include "Resource/Texture.h"

namespace Insight
{
	namespace Runtime
	{
		class IS_RUNTIME Texture2D : public Texture
		{
			REGISTER_RESOURCE(Texture2D);
		public:

		private:
			/// @brief Handle loading the resource from disk.
			/// @param file_path 
			virtual void Load();

			/// @brief Handle loading a resource from memory. This is called for a resource which is a "sub resource" of another one
			/// and exists inside another resource's disk file.
			/// @param file_path 
			virtual void LoadFromMemory(const void* data, u64 size_in_bytes);

			/// @brief Handle unloading the resource from memory.
			virtual void UnLoad();

			/// @brief Save resrouce to disk.
			virtual void Save(const std::string& file_path);

		private:
			void OnRHITextureUploadCompleted(Graphics::RHI_Texture* rhiTexture);
		};
	}
}