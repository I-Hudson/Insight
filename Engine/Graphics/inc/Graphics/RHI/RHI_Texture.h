#pragma once

#include "Graphics/Enums.h"
#include "Graphics/PixelFormat.h"
#include "Graphics/RHI/RHI_Resource.h"
#include "Graphics/RHI/RHI_UploadQueue.h"
#include "Graphics/GPUDeferedManager.h"

#include "Core/Delegate.h"

#include "Maths/Vector4.h"

#include <vector>

namespace Insight
{
	namespace Graphics
	{
		class RenderContext;
		class RHI_CommandList;
		struct RHI_UploadQueueRequest;

		struct RHI_TextureInfo
		{
			TextureType TextureType = TextureType::Unknown;
			int Width = -1;
			int Height = -1;
			int Depth = -1;
			PixelFormat Format = PixelFormat::Unknown;
			ImageUsageFlags ImageUsage = 0;

			u32 Mip_Count = 1;
			u32 Layer_Count = 1;

			Maths::Vector4 ClearColour = Maths::Vector4(0, 0, 0, 1);

			ImageLayout Layout = ImageLayout::Undefined;
			DeviceUploadStatus InitalStatus = DeviceUploadStatus::NotUploaded;

			static RHI_TextureInfo Tex2D(int width, int height, PixelFormat format, ImageUsageFlags usage)
			{
				RHI_TextureInfo info = { };
				info.TextureType = TextureType::Tex2D;
				info.Width = width;
				info.Height = height;
				info.Depth = 1;
				info.Format = format;
				info.ImageUsage = usage;
				return info;
			}
			static RHI_TextureInfo Tex2DArray(int width, int height, PixelFormat format, ImageUsageFlags usage, u32 layer_count)
			{
				RHI_TextureInfo info = { };
				info.TextureType = TextureType::Tex2DArray;
				info.Width = width;
				info.Height = height;
				info.Depth = 1;
				info.Format = format;
				info.ImageUsage = usage;
				info.Layer_Count = layer_count;
				return info;
			}
			static RHI_TextureInfo Cubemap(const u32 width, const u32 height, const PixelFormat format, const ImageUsageFlags usage)
			{
				RHI_TextureInfo info = { };
				info.TextureType = TextureType::TexCube;
				info.Width = width;
				info.Height = height;
				info.Depth = 1;
				info.Format = format;
				info.ImageUsage = usage;
				info.Layer_Count = 6;
				return info;
			}
		};

		class IS_GRAPHICS RHI_Texture : public RHI_Resource
		{
		public:
			virtual ~RHI_Texture() override;

			static RHI_Texture* New();

			void LoadFromData(Byte* data, u32 width, u32 height, u32 depth, u32 channels, const u64 textureSize = 0);

			RHI_TextureInfo  GetInfo					(u32 mip = 0)	const { if (mip < m_infos.size()) { return m_infos.at(mip); }				return {}; }
			int				 GetWidth					(u32 mip = 0)	const { if (mip < m_infos.size()) { return m_infos.at(mip).Width; }			return -1; }
			int				 GetHeight                  (u32 mip = 0)	const { if (mip < m_infos.size()) { return m_infos.at(mip).Height; }		return -1; }
			int				 GetChannels                (u32 mip = 0)	const { return 4; }
			TextureType		 GetType					(u32 mip = 0)	const { if (mip < m_infos.size()) { return m_infos.at(mip).TextureType; }	return TextureType::Unknown; }
			PixelFormat		 GetFormat				    (u32 mip = 0)	const { if (mip < m_infos.size()) { return m_infos.at(mip).Format; }		return PixelFormat::Unknown; }
			ImageLayout		 GetLayout				    (u32 mip = 0)	const { if (mip < m_infos.size()) { return m_infos.at(mip).Layout; }		return ImageLayout::Undefined; }
			Maths::Vector4	 GetClearColour				()				const { if (m_infos.size() > 0)   { return m_infos.at(0).ClearColour; }		return Maths::Vector4(0, 0, 0, 0); }
			bool			 HasAplha					()				const { return m_hasAlpha; }

			void			SetLayout(ImageLayout newLayout, u32 mip = 0) { if (mip < m_infos.size()) { m_infos.at(mip).Layout = newLayout; } }

			virtual void Create(RenderContext* context, RHI_TextureInfo createInfo) = 0;
			//TODO Mid: Look into a system to batch upload textures. Maybe submit a batch upload struct with a list of textures and data.
			virtual void Upload(void* data, int sizeInBytes) = 0;
			/// <summary>
			/// Add the upload to the RHI_UploadQueue. This will upload just before all rendering. This doesn't garuntee that the upload will have completed.
			/// </summary>
			/// <param name="data"></param>
			/// <param name="sizeInBytes"></param>
			RPtr<RHI_UploadQueueRequest> QueueUpload(void* data, int sizeInBytes);
			virtual std::vector<Byte> Download(void* data, int sizeInBytes) = 0;

			Core::Delegate<RHI_Texture*> OnUploadCompleted;

			// RHI_Resource - BEGIN
			virtual void Release() override;
			// RHI_Resource - End

		private:
			void OnUploadComplete(RHI_UploadQueueRequest* request);

		protected:
			/// @brief Define the info for all mips of the image.
			std::vector<RHI_TextureInfo> m_infos = { };
			GPUDeferedRequest m_deferedRequest;
			RHI_UploadQueueRequest* m_uploadRequest = nullptr;
			friend class RHI_CommandList;
		
		public:
			bool m_hasAlpha = false;
			PixelFormat m_pixelFormat;
		};
	}
}