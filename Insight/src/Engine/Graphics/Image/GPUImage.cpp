#include "ispch.h"
#include "Engine/Graphics/Image/GPUImage.h"
#include "Engine/Module/GraphicsModule.h"
#include "Engine/GraphicsAPI/Vulkan/GPUImageVulkan.h"

namespace Insight::Graphics
{

	GPUImageView* GPUImageView::New()
	{
		switch (Module::GraphicsModule::Instance()->GetAPI())
		{
			case GraphicsRendererAPI::Vulkan: return ::New<GraphicsAPI::Vulkan::GPUImageViewVulkan>();
		}

		ASSERT(false && "[GPUImageView::New] API missing.");
		return nullptr;
	}

	GPUImageView::GPUImageView()
	{
	}

	GPUImageView::~GPUImageView()
	{
	}

	void GPUImageView::Init(GPUImage* image)
	{
		ASSERT(image != nullptr && "[GPUImageView::Init] 'image' must be valid.");
		ReleaseGPU();

		m_image = image;
		m_image->m_view = this;
		if (!OnInit())
		{
			IS_WARN("GPUImage::Init] Can not create gpu resource.");
		}
	}


	GPUImage* GPUImage::New()
	{
		switch (Module::GraphicsModule::Instance()->GetAPI())
		{
			case GraphicsRendererAPI::Vulkan: return ::New<GraphicsAPI::Vulkan::GPUImageVulkan>();
		}

		ASSERT(false && "[GPUImage::New] API missing.");
		return nullptr;
	}

	GPUImage::GPUImage()
		: m_desc(GPUImageDesc())
	{ }

	GPUImage::~GPUImage()
	{ }

	void GPUImage::Init(GPUImageDesc desc)
	{
		ReleaseGPU();

		m_desc = desc;
		if (!OnInit())
		{
			IS_WARN("GPUImage::Init] Can not create gpu resource.");
		}
	}

	void GPUImage::SetLayout(const ImageLayout& newLayout)
	{
		m_imageLayout = newLayout;
	}

	/// <summary>
	/// GPUSAMPLER
	/// </summary>
	GPUSampler::GPUSampler()
		: m_desc(GPUSamplerDesc())
	{ }

	GPUSampler::~GPUSampler()
	{ }

	GPUSampler* GPUSampler::New()
	{
		switch (Module::GraphicsModule::Instance()->GetAPI())
		{
			case GraphicsRendererAPI::Vulkan: return ::New<GraphicsAPI::Vulkan::GPUSamplerVulkan>();
		}

		ASSERT(false && "[GPUImage::New] API missing.");
		return nullptr;
	}

	GPUSampler* GPUSampler::TryFromCache(GPUSamplerDesc& desc)
	{
		GPUSampler* samplerPtr = nullptr;
		GPUSamplerCache::Instance()->GetItem(desc.Hash(), samplerPtr);
		return samplerPtr;
	}
}