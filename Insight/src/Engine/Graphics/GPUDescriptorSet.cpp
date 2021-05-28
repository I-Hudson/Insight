#include "ispch.h"
#include "Engine/Graphics/GPUDescriptorSet.h"
#include "Engine/Graphics/Graphics.h"
#include "Engine/GraphicsAPI/Vulkan/GPUDescriptorSetVulkan.h"
#include "Engine/GraphicsAPI/Vulkan/GPUDescriptorAllocatorVulkan.h"

namespace Insight::Graphics
{
	/// <summary>
	/// DescriptorSet
	/// </summary>
	/// <param name="pool"></param>
	GPUDescriptorSet::GPUDescriptorSet()
		: m_desc(GPUDescriptorSetDesc(nullptr, nullptr))
	{ }

	GPUDescriptorSet::~GPUDescriptorSet()
	{ }

	GPUDescriptorSet* GPUDescriptorSet::New()
	{
		if (::Graphics::IsVulkan())
		{
			return ::New<GraphicsAPI::Vulkan::GPUDescriptorSetVulkan>();
		}
		ASSERT(false && "[DescriptorSet::New] Unsupported API.");
		return nullptr;
	}


	/// <summary>
	/// DescriptorPool
	/// </summary>
	GPUDescriptorPool::GPUDescriptorPool()
	{ }

	GPUDescriptorPool::~GPUDescriptorPool()
	{ }

	GPUDescriptorPool* GPUDescriptorPool::New()
	{
		if (::Graphics::IsVulkan())
		{
			return ::New<GraphicsAPI::Vulkan::GPUDescriptorPoolVulkan>();
		}
		ASSERT(false && "[DescriptorPool::New] Unsupported API.");
		return nullptr;
	}

	GPUDescriptorBuilder* GPUDescriptorBuilder::New()
	{
		if (::Graphics::IsVulkan())
		{
			return ::New<GraphicsAPI::Vulkan::GPUDescriptorBuilderVulkan>();
		}
		ASSERT(false && "[DescriptorPool::New] Unsupported API.");
		return nullptr;
	}

	GPUDescriptorAllocator* GPUDescriptorAllocator::New()
	{
		if (::Graphics::IsVulkan())
		{
			return ::New<GraphicsAPI::Vulkan::GPUDescriptorAllocatorVulkan>();
		}
		ASSERT(false && "[DescriptorPool::New] Unsupported API.");
		return nullptr;
	}

	GPUDescriptorLayoutCache* GPUDescriptorLayoutCache::New()
	{
		if (::Graphics::IsVulkan())
		{
			return ::New<GraphicsAPI::Vulkan::GPUDescriptorLayoutCacheVulkan>();
		}
		ASSERT(false && "[DescriptorPool::New] Unsupported API.");
		return nullptr;
	}
}