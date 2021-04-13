#include "ispch.h"
#include "Engine/GraphicsAPI/Vulkan/GPUDescriptorSetVulkan.h"
#include "Engine/GraphicsAPI/Vulkan/VulkanUtils.h"
#include "Engine/GraphicsAPI/Vulkan/VulkanInitializers.h"

namespace Insight::GraphicsAPI::Vulkan
{
	GPUDescriptorSetVulkan::GPUDescriptorSetVulkan()
	{
	}

	GPUDescriptorSetVulkan::~GPUDescriptorSetVulkan()
	{
	}

	void GPUDescriptorSetVulkan::Init(Graphics::GPUDescriptorSetDesc& desc)
	{
		ReleaseGPU();

		m_desc = desc;
		GPUDescriptorPoolVulkan* pool = static_cast<GPUDescriptorPoolVulkan*>(m_desc.Pool);
		VkDescriptorSetAllocateInfo allocInfo = vks::initializers::descriptorSetAllocateInfo(pool->m_pool, nullptr, 1);
		vkAllocateDescriptorSets(m_device->Device, &allocInfo, &m_set);
	}

	void GPUDescriptorSetVulkan::BindTexture(Graphics::GPUImage* image, u32 slot)
	{
	}

	void GPUDescriptorSetVulkan::BindUniformBuffer(Graphics::GPUBuffer* buffer, u32 slot)
	{
	}

	void GPUDescriptorSetVulkan::BindDynamicUniformBuffer(Graphics::GPUBuffer* buffer, u32 slot)
	{
	}

	void GPUDescriptorSetVulkan::SetName(const std::string& name)
	{
		m_name = name;
		if (GPUDebugMarkerVulkan::IsInitialised())
		{
			GPUDebugMarkerVulkan::Instance()->SetObjectName(m_name, Graphics::Debug::DebugObject::Descriptor_Set, (u64)m_set);
		}
	}

	void GPUDescriptorSetVulkan::OnReleaseGPU()
	{
		GPUDescriptorPoolVulkan* pool = static_cast<GPUDescriptorPoolVulkan*>(m_desc.Pool);
		vkFreeDescriptorSets(m_device->Device, pool->m_pool, 1, &m_set);
	}

	/// <summary>
	/// GPUDescriptorPoolVulkan
	/// </summary>
	GPUDescriptorPoolVulkan::GPUDescriptorPoolVulkan()
	{ }

	GPUDescriptorPoolVulkan::~GPUDescriptorPoolVulkan()
	{ }

	void GPUDescriptorPoolVulkan::Init()
	{
		ReleaseGPU();

		std::vector<VkDescriptorPoolSize> poolSizes = {};
		VkDescriptorPoolSize pool = {};
		pool.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		pool.descriptorCount = Graphics::DESCRIPTOR_POOL_UNIFORM_BUFFER_COUNT * Graphics::DESCRIPTOR_POOL_SIZE;
		poolSizes.push_back(pool);

		pool.type = VK_DESCRIPTOR_TYPE_SAMPLER;
		pool.descriptorCount = Graphics::DESCRIPTOR_POOL_SAMPLER_BUFFER_COUNT * Graphics::DESCRIPTOR_POOL_SIZE;
		poolSizes.push_back(pool);

		VkDescriptorPoolCreateInfo poolInfo = vks::initializers::descriptorPoolCreateInfo(poolSizes, 256);
		ThrowIfFailed(vkCreateDescriptorPool(m_device->Device, &poolInfo, nullptr, &m_pool));
		m_memoryUsage = 1;
	}

	Graphics::GPUDescriptorSet* GPUDescriptorPoolVulkan::AllocaSet(Graphics::GPUDescriptorSetDesc& desc)
	{
		Graphics::GPUDescriptorSet* set = Graphics::GPUDescriptorSet::New();
		desc.Pool = this;
		set->Init(desc);
		m_sets.push_back(set);
		return set;
	}

	void GPUDescriptorPoolVulkan::ResetPool(Graphics::GPUDescriptorPoolResetFlag resetFlags)
	{
		ThrowIfFailed(vkResetDescriptorPool(m_device->Device, m_pool, ToVulkanDescriptorResetFlags(resetFlags)));
	}

	void GPUDescriptorPoolVulkan::SetName(const std::string& name)
	{
		m_name = name;
		if (GPUDebugMarkerVulkan::IsInitialised())
		{
			GPUDebugMarkerVulkan::Instance()->SetObjectName(m_name, Graphics::Debug::DebugObject::Descriptor_Pool, (u64)m_pool);
		}
	}

	void GPUDescriptorPoolVulkan::OnReleaseGPU()
	{
		for (auto* set : m_sets)
		{
			::Delete(set);
		}
		vkDestroyDescriptorPool(m_device->Device, m_pool, nullptr);
	}
}