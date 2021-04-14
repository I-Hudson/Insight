#include "ispch.h"
#include "Engine/Graphics/GPUDescriptorSet.h"
#include "Engine/Module/GraphicsModule.h"
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
		switch (Module::GraphicsModule::Instance()->GetAPI())
		{
			case GraphicsRendererAPI::Vulkan: return ::New<GraphicsAPI::Vulkan::GPUDescriptorSetVulkan>();
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
		switch (Module::GraphicsModule::Instance()->GetAPI())
		{
			case GraphicsRendererAPI::Vulkan: return ::New<GraphicsAPI::Vulkan::GPUDescriptorPoolVulkan>();
		}
		ASSERT(false && "[DescriptorPool::New] Unsupported API.");
		return nullptr;
	}

	GPUDescriptorBuilder* GPUDescriptorBuilder::New()
	{
		switch (Module::GraphicsModule::Instance()->GetAPI())
		{
			case GraphicsRendererAPI::Vulkan: return ::New<GraphicsAPI::Vulkan::GPUDescriptorBuilderVulkan>();
		}
		ASSERT(false && "[DescriptorPool::New] Unsupported API.");
		return nullptr;
	}
}