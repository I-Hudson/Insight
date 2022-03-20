#include "Graphics/GPU/GPUImage.h"
#include "Graphics/GPU/RHI/Vulkan/GPUImage_Vulkan.h"
#include "Core/Logger.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace Insight
{
	namespace Graphics
	{
		//void GPUImage::LoadFromFile(std::string filename, PixelFormat format)
		//{
		//	stbi_uc* stbiData = stbi_load(filename.c_str(), &m_width, &m_height, &m_channels, STBI_rgb);
		//	if (!stbiData)
		//	{
		//		return;
		//	}
		//	m_loadedFromFile = true;
		//	const u64 dataSize = m_width * m_height * m_channels;
		//	std::vector<Byte> data;
		//	data.resize(dataSize);
		//	memcpy_s(data.data(), dataSize, stbiData, dataSize);
		//	stbi_image_free(stbiData);

		//	GPUImageCreateInfo info{};
		//	info.Data = data;
		//	info.Format = format;
		//	LoadFromData(info);
		//}
		GPUImage* GPUImage::New()
		{
			if (GraphicsManager::IsVulkan()) { return new RHI::Vulkan::GPUImage_Vulkan(); }
			//else if (GraphicsManager::IsDX12()) { return new RHI::DX12::GPUImage_DX12(); }
			return nullptr;
		}


		GPUImageManager::GPUImageManager()
		{
		}

		GPUImageManager::~GPUImageManager()
		{
			Destroy();
		}

		GPUImage* GPUImageManager::CreateOrGetImage(std::string key)
		{
			std::map<const std::string, GPUImage*>::iterator itr = m_keyToImageLookup.find(key);
			if (itr != m_keyToImageLookup.end())
			{
				return itr->second;
			}

			GPUImage* newImage = GPUImage::New();
			m_images.push_back(newImage);
			m_keyToImageLookup[key] = newImage;
			return newImage;
		}

		void GPUImageManager::DestroyImage(std::string key)
		{
			std::map<const std::string, GPUImage*>::iterator itr = m_keyToImageLookup.find(key);
			if (itr == m_keyToImageLookup.end())
			{
				IS_CORE_WARN("[GPUImageManager::DestroyImage] Image with key {} does not exists.");
				return;
			}
			DestroyImage(itr->second);
			m_keyToImageLookup.erase(itr);
		}

		void GPUImageManager::DestroyImage(GPUImage* image)
		{
			std::list<GPUImage*>::iterator itr = std::find(m_images.begin(), m_images.end(), image);
			if (itr == m_images.end())
			{
				IS_CORE_WARN("[GPUImageManager::DestroyImage] Image does not exists.");
				return;
			}

			GPUImage*& imageRef = *itr;
			imageRef->Destroy();
			delete imageRef;
			m_images.erase(itr);
		}

		void GPUImageManager::Destroy()
		{
			if (m_images.size() > 0)
			{
				IS_CORE_WARN("[GPUImageManager::Destroy] Outstanding images have not been destroy explicitly.");
			}

			for (std::list<GPUImage*>::iterator::value_type& ptr : m_images)
			{
				ptr->Destroy();
				delete ptr;
				ptr = nullptr;
			}
			m_images.clear();
			m_keyToImageLookup.clear();
		}
	}
}