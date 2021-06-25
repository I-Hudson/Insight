
#include "Engine/GraphicsAPI/Vulkan/GPUDescriptorAllocatorVulkan.h"
#include "Engine/GraphicsAPI/Vulkan/GPUBufferVulkan.h"
#include "Engine/GraphicsAPI/Vulkan/VulkanUtils.h"
#include "Engine/GraphicsAPI/Vulkan/GPUImageVulkan.h"

namespace Insight::GraphicsAPI::Vulkan
{
#define CacheDescriptors 1
	/// <summary>
	/// GPUDescriptorAllocator
	/// </summary>
	GPUDescriptorAllocatorVulkan::GPUDescriptorAllocatorVulkan()
		: m_currentPool(nullptr)
	{
	}

	GPUDescriptorAllocatorVulkan::~GPUDescriptorAllocatorVulkan()
	{
		for (auto* pool : m_freePools)
		{
			pool->ResetPool(Graphics::GPUDescriptorPoolResetFlag::None);
			::Delete(pool);
		}
		for (auto* pool : m_usedPools)
		{
			pool->ResetPool(Graphics::GPUDescriptorPoolResetFlag::None);
			::Delete(pool);
		}
		for (auto* set : m_allocatedSets)
		{
			::Delete(set);
		}
		for (auto& kvp : m_freeSets)
		{
			for (auto* set : kvp.second)
			{
				::Delete(set);
			}
		}
	}

	void GPUDescriptorAllocatorVulkan::Init()
	{ }

	bool GPUDescriptorAllocatorVulkan::Allocate(Graphics::GPUDescriptorSet*& set, VkDescriptorSetLayout layout)
	{
		if (m_currentPool == nullptr)
		{
			m_currentPool = GrabPool();
			m_usedPools.push_back(m_currentPool);
		}

		Graphics::GPUDescriptorSetDesc setDesc(m_currentPool, layout);

#if CacheDescriptors
		// Check if set has already been allocated
		Utils::Hasher hash;
		hash.Hash(layout);
		auto itr = m_freeSets.find(hash.GetHash());
		if (itr != m_freeSets.end())
		{
			if (itr->second.size() > 0)
			{
				set = itr->second.at(0);
				itr->second.erase(itr->second.begin());

				m_allocatedSets.push_back(set);
				return true;
			}
		}
#endif
		if (!set)
		{
			set = Graphics::GPUDescriptorSet::New();
			m_allocatedSets.push_back(set);
		}

		bool reallocate = false;
		{
			IS_PROFILE_SCOPE("Descriptor set init");
			switch (set->Init(setDesc))
			{
				case GPUResults::Error_Fragmented_Pool:
				case GPUResults::Error_Out_Of_Pool_Memory:
					reallocate = true;
					break;
			}

			if (reallocate)
			{
				m_currentPool = GrabPool();
				m_usedPools.push_back(m_currentPool);

				setDesc.Pool = m_currentPool;
				ASSERT(set->Init(setDesc) == GPUResults::Success && "[GPUDescriptorAllocatorVulkan::Allocate] Set was not created.");
			}
		}
		return true;
	}

	void GPUDescriptorAllocatorVulkan::ResetPools()
	{
		for (auto* pool : m_usedPools)
		{
#if !CacheDescriptors
			pool->ResetPool(Graphics::GPUDescriptorPoolResetFlag::None);
#endif
			m_freePools.push_back(pool);
		}
		m_usedPools.clear();
		m_currentPool = nullptr;

		for (auto* set : m_allocatedSets)
		{
#if CacheDescriptors
			Utils::Hasher hash;
			hash.Hash(set->GetDesc().Layout);
			m_freeSets[hash.GetHash()].push_back(set);
#else
			::Delete(set);
#endif
		}
		m_allocatedSets.clear();
	}

	Graphics::GPUDescriptorPool* GPUDescriptorAllocatorVulkan::GrabPool()
	{
		if (m_freePools.size() > 0)
		{
			auto* pool = m_freePools.back();
			m_freePools.pop_back();
			return pool;
		}
		else
		{
			return CreatePool();
		}
	}

	Graphics::GPUDescriptorPool* GPUDescriptorAllocatorVulkan::CreatePool()
	{
		Graphics::GPUDescriptorPool* pool = Graphics::GPUDescriptorPool::New();
		pool->Init();
		return pool;
	}

	GPUDescriptorLayoutCacheVulkan::~GPUDescriptorLayoutCacheVulkan()
	{
		Cleanup();
	}

	/// <summary>
	/// GPUDescriptorLayoutCacheVulkan
	/// </summary>
	void GPUDescriptorLayoutCacheVulkan::Init()
	{
	}

	void GPUDescriptorLayoutCacheVulkan::Cleanup()
	{
		VkDevice device = static_cast<GPUDeviceVulkan*>(Graphics::GPUDevice::Instance())->Device;
		for (auto& pair : m_layoutCache)
		{
			vkDestroyDescriptorSetLayout(device, pair.second, nullptr);
		}
	}

	VkDescriptorSetLayout GPUDescriptorLayoutCacheVulkan::CreateDescriptorLayout(VkDescriptorSetLayoutCreateInfo* info)
	{
		DescriptorLayoutInfo layoutInfo;
		layoutInfo.Bindings.reserve(info->bindingCount);
		layoutInfo.BindingsFlags.reserve(info->bindingCount);
		bool isSorted = true;
		int lastBinding = -1;

		//copy from the direct info struct into our own one
		for (u32 i = 0; i < info->bindingCount; ++i)
		{
			layoutInfo.Bindings.push_back(info->pBindings[i]);
			layoutInfo.BindingsFlags.push_back(VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);

			//check that the bindings are in strict increasing order
			if ((i32)info->pBindings[i].binding > lastBinding)
			{
				lastBinding = info->pBindings[i].binding;
			}
			else
			{
				isSorted = false;
			}
		}

		//sort the bindings if they arent in order
		if (!isSorted) 
		{
			std::sort(layoutInfo.Bindings.begin(), layoutInfo.Bindings.end(), [](VkDescriptorSetLayoutBinding& a, VkDescriptorSetLayoutBinding& b)
			{
				return a.binding < b.binding;
			});
		}

		VkDevice device = static_cast<GPUDeviceVulkan*>(Graphics::GPUDevice::Instance())->Device;
		//try to grab from cache
		auto it = m_layoutCache.find(layoutInfo);
		if (it != m_layoutCache.end())
		{
			return (*it).second;
		}
		else 
		{

			if (GPUDeviceVulkan::Instance()->HasExtension(Graphics::GPUDeviceExtension::Bindless_Descriptor))
			{
				VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlags = { };
				bindingFlags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
				bindingFlags.bindingCount = static_cast<u32>(layoutInfo.BindingsFlags.size());
				bindingFlags.pBindingFlags = layoutInfo.BindingsFlags.data();
				info->pNext = &bindingFlags;
			}

			//create a new one (not found)
			VkDescriptorSetLayout layout;
			vkCreateDescriptorSetLayout(device, info, nullptr, &layout);

			//add to cache
			m_layoutCache[layoutInfo] = layout;
			return layout;
		}
	}

	/// <summary>
	/// GPUDescriptorLayoutCacheVulkan
	/// </summary>
	/// <param name="other"></param>
	/// <returns></returns>
	bool GPUDescriptorLayoutCacheVulkan::DescriptorLayoutInfo::operator==(const DescriptorLayoutInfo& other) const
	{
		if (other.Bindings.size() != Bindings.size())
		{
			return false;
		}
		else 
		{
			//compare each of the bindings is the same. Bindings are sorted so they will match
			for (int i = 0; i < Bindings.size(); ++i)
			{
				if (other.Bindings[i].binding != Bindings[i].binding)
				{
					return false;
				}
				if (other.Bindings[i].descriptorType != Bindings[i].descriptorType)
				{
					return false;
				}
				if (other.Bindings[i].descriptorCount != Bindings[i].descriptorCount)
				{
					return false;
				}
				if (other.Bindings[i].stageFlags != Bindings[i].stageFlags)
				{
					return false;
				}
			}
			return true;
		}
	}

	size_t GPUDescriptorLayoutCacheVulkan::DescriptorLayoutInfo::hash() const
	{
		using std::size_t;
		using std::hash;

		size_t result = hash<size_t>()(Bindings.size());

		for (const VkDescriptorSetLayoutBinding& b : Bindings)
		{
			//pack the binding data into a single int64. Not fully correct but it's ok
			size_t binding_hash = b.binding | b.descriptorType << 8 | b.descriptorCount << 16 | b.stageFlags << 24;

			//shuffle the packed binding data and xor it with the main hash
			result ^= hash<size_t>()(binding_hash);
		}

		return result;
	}

	/// <summary>
	/// GPUDescriptorBuilderVulkan
	/// </summary>
	/// <param name="layoutCache"></param>
	/// <param name="allocator"></param>
	/// <returns></returns>
	Graphics::GPUDescriptorBuilder* GPUDescriptorBuilderVulkan::Begin(Graphics::GPUDescriptorLayoutCache* layoutCache, Graphics::GPUDescriptorAllocator* allocator)
	{
		m_layoutCache = static_cast<GPUDescriptorLayoutCacheVulkan*>(layoutCache);
		m_allocator = static_cast<GPUDescriptorAllocatorVulkan*>(allocator);
		return this;
	}

	Graphics::GPUDescriptorBuilder* GPUDescriptorBuilderVulkan::BindBuffer(u32 binding, Graphics::GPUBuffer* buffer, DescriptorType type, ShaderStage stage, bool write)
	{
		//create the descriptor binding for the layout
		VkDescriptorSetLayoutBinding newBinding{};

		newBinding.descriptorCount = 1;
		newBinding.descriptorType = ToVulkanDescriptorType(type);
		newBinding.pImmutableSamplers = nullptr;
		newBinding.stageFlags = ToVulkanShaderStageFlags(stage);
		newBinding.binding = binding;

		m_bindings.push_back(newBinding);

		if (write)
		{
			//create the descriptor write
			VkWriteDescriptorSet newWrite{};
			newWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			newWrite.pNext = nullptr;

			newWrite.descriptorCount = 1;
			newWrite.descriptorType = ToVulkanDescriptorType(type);
			if (buffer)
			{
				newWrite.pBufferInfo = static_cast<GPUBufferVulkan*>(buffer)->GetBufferInfo();
			}
			newWrite.dstBinding = binding;
			m_writes.push_back(newWrite);
		}
		return this;
	}

	Graphics::GPUDescriptorBuilder* GPUDescriptorBuilderVulkan::BindImage(u32 binding, Graphics::GPUImage* image, DescriptorType type, ShaderStage stage, bool write)
	{
		//create the descriptor binding for the layout
		VkDescriptorSetLayoutBinding newBinding{};

		newBinding.descriptorCount = 1;
		newBinding.descriptorType = ToVulkanDescriptorType(type);
		newBinding.pImmutableSamplers = nullptr;
		newBinding.stageFlags = ToVulkanShaderStageFlags(stage);
		newBinding.binding = binding;

		m_bindings.push_back(newBinding);

		if (write)
		{
			//create the descriptor write
			VkWriteDescriptorSet newWrite{};
			newWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			newWrite.pNext = nullptr;

			newWrite.descriptorCount = 1;
			newWrite.descriptorType = ToVulkanDescriptorType(type);
			newWrite.pImageInfo = static_cast<GPUImageVulkan*>(image)->GetDescriptorImageInfo();
			newWrite.dstBinding = binding;
			m_writes.push_back(newWrite);
		}
		return this;
	}

	Graphics::GPUDescriptorBuilder* GPUDescriptorBuilderVulkan::BindImageArray(u32 binding, std::vector<Graphics::GPUImage*>& images, DescriptorType type, ShaderStage stage, bool write)
	{
		//create the descriptor binding for the layout
		VkDescriptorSetLayoutBinding newBinding{};

		newBinding.descriptorCount = (u32)images.size();
		newBinding.descriptorType = ToVulkanDescriptorType(type);
		newBinding.pImmutableSamplers = nullptr;
		newBinding.stageFlags = ToVulkanShaderStageFlags(stage);
		newBinding.binding = binding;
		m_bindings.push_back(newBinding);

		m_tempTextureArrays.push_back(DiscriptorTextureArray());
		DiscriptorTextureArray& array = m_tempTextureArrays.back();
		for (auto& image : images)
		{
			array.ImageInfos.push_back(*static_cast<GPUImageVulkan*>(image)->GetDescriptorImageInfo());
		}

		if (write)
		{
			//create the descriptor write
			VkWriteDescriptorSet newWrite{};
			newWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			newWrite.pNext = nullptr;
			newWrite.descriptorCount = (u32)m_tempTextureArrays.back().ImageInfos.size();
			newWrite.descriptorType = ToVulkanDescriptorType(type);
			newWrite.pImageInfo = m_tempTextureArrays.back().ImageInfos.data();
			newWrite.dstBinding = binding;
			m_writes.push_back(newWrite);
		}
		return this;
	}

	bool GPUDescriptorBuilderVulkan::Build(Graphics::GPUDescriptorSet*& set)
	{
		IS_PROFILE_FUNCTION();
		//build layout first
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.pNext = nullptr;

		layoutInfo.pBindings = m_bindings.data();
		layoutInfo.bindingCount = static_cast<u32>(m_bindings.size());

		VkDescriptorSetLayout layout = nullptr;
		{
			IS_PROFILE_SCOPE("CreateDescriptorLayout");
			layout = m_layoutCache->CreateDescriptorLayout(&layoutInfo);
		}
		//allocate descriptor
		{
			IS_PROFILE_SCOPE("Allocate");
			bool success = m_allocator->Allocate(set, layout);
			if (!success) { return false; };
		}

		//write descriptor
		for (VkWriteDescriptorSet& w : m_writes) 
		{
			w.dstSet = static_cast<GPUDescriptorSetVulkan*>(set)->m_set;
		}

		VkDevice device = static_cast<GPUDeviceVulkan*>(Graphics::GPUDevice::Instance())->Device;
		vkUpdateDescriptorSets(device, static_cast<u32>(m_writes.size()), m_writes.data(), 0, nullptr);
		m_writes.clear();
		m_bindings.clear();
		m_tempTextureArrays.clear();

		return true;
	}
}
