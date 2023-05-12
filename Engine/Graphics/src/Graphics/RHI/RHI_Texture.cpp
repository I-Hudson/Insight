#include "Graphics/RHI/RHI_Texture.h"

#include "Graphics/RHI/Vulkan/RHI_Texture_Vulkan.h"
#include "Graphics/RHI/DX12/RHI_Texture_DX12.h"

#include "Core/Profiler.h"

#include <filesystem>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

#define RHI_TEXTURE_DEFER_ENABLED
#pragma optimize("", off)
namespace Insight
{
	namespace Graphics
	{
		RHI_Texture::~RHI_Texture()
		{
			Release();
		}

		RHI_Texture* RHI_Texture::New()
		{
#if defined(IS_VULKAN_ENABLED)
			if (RenderContext::Instance().GetGraphicsAPI() == GraphicsAPI::Vulkan) { return ::New<RHI::Vulkan::RHI_Texture_Vulkan, Insight::Core::MemoryAllocCategory::Graphics>(); }
#endif
#if defined(IS_DX12_ENABLED)
			if (RenderContext::Instance().GetGraphicsAPI() == GraphicsAPI::DX12) { return ::New<RHI::DX12::RHI_Texture_DX12, Insight::Core::MemoryAllocCategory::Graphics>(); }
#endif	
			return nullptr;
		}

		void RHI_Texture::LoadFromData(Byte* data, u32 width, u32 height, u32 depth, u32 channels)
		{
			const u64 size_in_bytes = width * height * depth * channels;
			if (data == nullptr || size_in_bytes == 0)
			{
				return;
			}

			RHI_TextureInfo createInfo = { };
			createInfo.TextureType = TextureType::Tex2D;
			createInfo.Width = width;
			createInfo.Height = height;
			createInfo.Depth = depth;
			createInfo.Format = PixelFormat::R8G8B8A8_UNorm;
			createInfo.ImageUsage = ImageUsageFlagsBits::Sampled | ImageUsageFlagsBits::TransferDst;

#ifdef RHI_TEXTURE_DEFER_ENABLED
			std::vector<Byte> imageData;
			imageData.resize(size_in_bytes);
			Platform::MemCopy(imageData.data(), data, size_in_bytes);
			m_deferedRequest = RenderContext::Instance().GetDeferredManager().Push([this, createInfo, size_in_bytes, imageData](RHI_CommandList* cmdList)
				{
					Create(&RenderContext::Instance(), createInfo);
					m_uploadStatus = DeviceUploadStatus::Uploading;

					m_uploadRequest = QueueUpload((void*)imageData.data(), (int)size_in_bytes).Get();
					m_uploadRequest->OnUploadCompleted.Bind<&RHI_Texture::OnUploadComplete>(this);
				});
#else
			Create(&RenderContext::Instance(), createInfo);
			Upload(data, static_cast<int>(size_in_bytes));
#endif
		}

		RPtr<RHI_UploadQueueRequest> RHI_Texture::QueueUpload(void* data, int sizeInBytes)
		{
			return RenderContext::Instance().GetUploadQueue().UploadTexture(data, sizeInBytes, this);
		}

		void RHI_Texture::Release()
		{
#ifdef RHI_TEXTURE_DEFER_ENABLED
			RenderContext::Instance().GetDeferredManager().Remove(m_deferedRequest);
#endif
			RenderContext::Instance().GetUploadQueue().RemoveRequest(m_uploadRequest);
		}

		void RHI_Texture::OnUploadComplete(RHI_UploadQueueRequest* request)
		{
			request->OnUploadCompleted.Unbind<&RHI_Texture::OnUploadComplete>(this);
			m_uploadStatus = DeviceUploadStatus::Completed;
			OnUploadCompleted(this);
			m_uploadRequest = nullptr;
		}
	}
}