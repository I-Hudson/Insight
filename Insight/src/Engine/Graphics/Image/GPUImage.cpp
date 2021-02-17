#include "ispch.h"
#include "GPUImage.h"
#include "Engine/Module/GraphicsModule.h"
#include "Engine/GraphicsAPI/Vulkan/GPUImageVulkan.h"

GPUImageView* GPUImageView::New()
{
	switch (Module::GraphicsModule::Instance()->GetAPI())
	{
		case GraphicsAPI::Vulkan: return ::New<GPUImageViewVulkan>();
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
		case GraphicsAPI::Vulkan: return ::New<GPUImageVulkan>();
	}

	ASSERT(false && "[GPUImage::New] API missing.");
	return nullptr;
}

GPUImage::GPUImage()
{
}

GPUImage::~GPUImage()
{
}

void GPUImage::Init(GPUImageDescription desc)
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
