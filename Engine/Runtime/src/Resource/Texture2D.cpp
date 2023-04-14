#include "Resource/Texture2D.h"
#include "Graphics/RenderContext.h"

#include "FileSystem/FileSystem.h"
#include "Core/Profiler.h"

#include <stb_image.h>
#include <stb_image_write.h>

#define QOI_IMPLEMENTATION
#include <qoi.h>

#include <fstream>

namespace Insight
{
	namespace Runtime
	{
		Texture2D::Texture2D(std::string_view filePath)
			: Texture(filePath)
		{
		}

		void Texture2D::Load()
		{
			IS_PROFILE_FUNCTION();

			if (m_rawDataPtr == nullptr && m_dataSize == 0)
			{
				std::string_view filePath = m_file_path;
				if (filePath.empty() || !FileSystem::FileSystem::Exists(filePath))
				{
					m_resource_state = EResoruceStates::Not_Found;
					return;
				}

				std::ifstream fileStream(filePath.data(), std::ios::in | std::ios::binary);
				if (!fileStream.is_open())
				{
					m_resource_state = EResoruceStates::Failed_To_Load;
					return;
				}

				const u64 fileSize = FileSystem::FileSystem::GetFileSize(filePath);
				std::vector<Byte> fileData;
				fileData.resize(fileSize);

				fileStream.read((char*)fileData.data(), fileSize);
				fileStream.close();

				m_dataSize = fileSize;
				m_rawDataPtr = static_cast<Byte*>(NewBytes(m_dataSize, Core::MemoryAllocCategory::Resources));
				Platform::MemCopy(m_rawDataPtr, fileData.data(), m_dataSize);
			}

			LoadFromMemory(m_rawDataPtr, m_dataSize);
		}

		void Texture2D::LoadFromMemory(Byte* data, const u64 dataSize)
		{
			ASSERT(!m_rhi_texture && data && dataSize > 0);
			IS_PROFILE_FUNCTION();

			m_rhi_texture = Renderer::CreateTexture();
			m_rhi_texture->SetName(std::string(FileSystem::FileSystem::GetFileName(m_file_path)));
			m_rhi_texture->OnUploadCompleted.Bind<&Texture2D::OnRHITextureUploadCompleted>(this);

			int width, height, channels;
			Byte* textureData = nullptr;
			if (m_diskFormat == TextureDiskFormat::QOI)
			{
				IS_PROFILE_SCOPE("qoi_decode");
				qoi_desc qoiDesc;
				textureData = static_cast<Byte*>(qoi_decode(m_rawDataPtr, static_cast<int>(m_dataSize), &qoiDesc, 4));
				width = qoiDesc.width;
				height = qoiDesc.height;
				channels = qoiDesc.channels;
			}
			else
			{
				IS_PROFILE_SCOPE("stbi_load_from_memory");
				textureData = stbi_load_from_memory(data, static_cast<int>(dataSize), &width, &height, &channels, STBI_rgb_alpha);
				channels = STBI_rgb_alpha;
			}

			if (!textureData)
			{
				m_resource_state = EResoruceStates::Failed_To_Load;
				return;
			}

			m_width = width;
			m_height = height;
			m_depth = 1;

			m_rhi_texture->LoadFromData(textureData, GetWidth(), GetHeight(), GetDepth(), STBI_rgb_alpha);

			if (m_diskFormat == TextureDiskFormat::QOI)
			{
				QOI_FREE(textureData);
			}
			else
			{
				stbi_image_free(textureData);
			}

			/// Delete the texture data from memory.
			/// The texture is now loaded so it's not needed.
			/// TODO: Look into this as if you edit the texture at runtime 
			/// do I really want to have to download from the GPU. But if kept in
			/// RAM what is the memory cost.
			//m_dataSize = 0;
			//DeleteBytes(m_rawDataPtr);

			m_resource_state = EResoruceStates::Loaded;
		}

		void Texture2D::UnLoad()
		{
			Renderer::FreeTexture(m_rhi_texture);
			m_rhi_texture = nullptr;
			
			m_dataSize = 0;
			DeleteBytes(m_rawDataPtr);

			m_resource_state = EResoruceStates::Unloaded;
		}

		void Texture2D::Save(const std::string& file_path)
		{
			FAIL_ASSERT();
		}

		void Texture2D::OnRHITextureUploadCompleted(Graphics::RHI_Texture* rhiTexture)
		{
			ASSERT(rhiTexture == m_rhi_texture);

			rhiTexture->OnUploadCompleted.Unbind<&Texture2D::OnRHITextureUploadCompleted>(this);
			m_width = rhiTexture->GetWidth(0);
			m_height = rhiTexture->GetHeight(0);
			m_depth = rhiTexture->GetInfo(0).Depth;
			m_format = rhiTexture->GetFormat();
		}
	}
}