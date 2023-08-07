#include "Resource/Texture2D.h"
#include "Graphics/RenderContext.h"

#include "Resource/Loaders/ResourceLoaderRegister.h"
#include "Resource/Loaders/TextureLoader.h"

#include "FileSystem/FileSystem.h"
#include "Core/Profiler.h"

#include <fstream>

namespace Insight
{
	namespace Runtime
	{
		Texture2D::Texture2D(std::string_view filePath)
			: Texture(filePath)
		{
		}

		Texture2D::~Texture2D()
		{
		}

		void Texture2D::UpdateRHITexture(void* textureData, u64 textureSize)
		{
			IS_PROFILE_FUNCTION();

			if (m_rhi_texture)
			{
				Renderer::FreeTexture(m_rhi_texture);
			}

			ASSERT(textureSize == GetWidth() * GetHeight() * GetDepth() * 4);
			m_rhi_texture = Renderer::CreateTexture();
			m_rhi_texture->SetName(std::string(FileSystem::GetFileName(m_file_path)));
			m_rhi_texture->OnUploadCompleted.Bind<&Texture2D::OnRHITextureUploadCompleted>(this);
			m_rhi_texture->LoadFromData((Byte*)textureData, GetWidth(), GetHeight(), GetDepth(), 4);
		}

		std::vector<Byte> Texture2D::GetPixels() const
		{
			if (IsEngineFormat())
			{
				Serialisation::BinarySerialiser serialiser(true);
				std::vector<Byte> fileData = FileSystem::ReadFromFile(m_file_path, FileType::Binary);
				if (!serialiser.Deserialise(fileData))
				{
					return {};
				}

				Serialisation::SerialiserObject<Texture> textureSerialsier;
				std::vector<Byte> pixelData = textureSerialsier.GetData<std::vector<Byte>>(&serialiser, *(Texture*)this, "m_rawDataPtr");
				return pixelData;
			}
			else
			{
				const IResourceLoader* resourceLoader = ResourceLoaderRegister::GetLoaderFromResource(this);
				if (!resourceLoader)
				{
					return {};
				}
				const TextureLoader* textureLoader = static_cast<const TextureLoader*>(resourceLoader);
				LoadPixelData pixelsData = textureLoader->LoadPixels(this->GetFilePath(), m_metaData.DiskFormat);
				return pixelsData.Data;
			}
		}

		void Texture2D::UnLoad()
		{
			Renderer::FreeTexture(m_rhi_texture);
			m_rhi_texture = nullptr;

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
			m_metaData.PixelFormat = rhiTexture->GetFormat();
		}
	}
}