#include "ispch.h"
#include "DescriptorSetVulkan.h"
#include "GPUDeviceVulkan.h"
#include "GPUAdapterVulkan.h"
#include "VulkanInitializers.h"
//#include "GPUContextVulkan.h"
//#include "CmdBufferVulkan.h"
#include "Engine/Threading/Threading.h"
#include "VulkanUtils.h"
#include "Engine/Utils/Crc.h"
#include "Engine/Core/Application.h"

void DescriptorSetLayoutInfoVulkan::CacheTypesUsageID()
{
    static CriticalSection locker;
    static U32 uniqueID = 1;
    static std::unordered_map<U32, U32> typesUsageHashMap;

    const U32 typesUsageHash = Crc::MemCrc32(m_layoutTypes, sizeof(m_layoutTypes));
    ScopeLock lock(locker);
    U32 id;
    auto it = std::find(typesUsageHashMap.begin(), typesUsageHashMap.end(), typesUsageHash);
    if (it == typesUsageHashMap.end())
    {
        id = uniqueID++;
        typesUsageHashMap[typesUsageHash] = id;
    }
    m_typesUsageID = id;
}

void DescriptorSetLayoutInfoVulkan::AddDescriptor(I32 descriptorSetIndex, const VkDescriptorSetLayoutBinding& descriptor)
{
    m_layoutTypes[descriptor.descriptorType]++;

    if (descriptorSetIndex >= m_setLayouts.size())
    {
        m_setLayouts.resize(descriptorSetIndex + 1);
    }

    SetLayout& descSetLayout = m_setLayouts[descriptorSetIndex];
    descSetLayout.LayoutBindings.push_back(descriptor);

    m_hash = Crc::MemCrc32(&descriptor, sizeof(descriptor), m_hash);
}

void DescriptorSetLayoutInfoVulkan::AddBindingsForStage(VkShaderStageFlagBits stageFlags, DescriptorSet::Stage descSet, const SpirvShaderDescriptorInfo* descriptorInfo)
{
    const I32 descriptorSetIndex = (I32)descSet;

    VkDescriptorSetLayoutBinding binding;
    binding.descriptorCount = 1;
    binding.stageFlags = stageFlags;
    binding.pImmutableSamplers = nullptr;

    for (U32 descriptorIndex = 0; descriptorIndex < descriptorInfo->DescriptorTypesCount; descriptorIndex++)
    {
        auto& descriptor = descriptorInfo->DescriptorTypes[descriptorIndex];
        binding.binding = descriptorIndex;
        binding.descriptorType = descriptor.DescriptorType;
        AddDescriptor(descriptorSetIndex, binding);
    }
}

DescriptorSetLayoutVulkan::DescriptorSetLayoutVulkan(GPUDeviceVulkan* device)
    : m_device(device)
{
}

DescriptorSetLayoutVulkan::~DescriptorSetLayoutVulkan()
{
    for (VkDescriptorSetLayout& handle : m_handles)
    {
        m_device->DeferredDeletionQueue.EnqueueResource(DeferredDeletionQueueVulkan::Type::DescriptorSetLayout, handle);
    }
}

void DescriptorSetLayoutVulkan::Compile()
{
    ASSERT(m_handles.empty());

    // Validate device limits for the engine
    const VkPhysicalDeviceLimits& limits = m_device->PhysicalDeviceLimits;
    ASSERT(m_layoutTypes[VK_DESCRIPTOR_TYPE_SAMPLER] + m_layoutTypes[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER] < limits.maxDescriptorSetSamplers);
    ASSERT(m_layoutTypes[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER] + m_layoutTypes[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC] < limits.maxDescriptorSetUniformBuffers);
    ASSERT(m_layoutTypes[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC] < limits.maxDescriptorSetUniformBuffersDynamic);
    ASSERT(m_layoutTypes[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER] + m_layoutTypes[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC] < limits.maxDescriptorSetStorageBuffers);
    ASSERT(m_layoutTypes[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC] < limits.maxDescriptorSetStorageBuffersDynamic);
    ASSERT(m_layoutTypes[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER] + m_layoutTypes[VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE] + m_layoutTypes[VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER] < limits.maxDescriptorSetSampledImages);
    ASSERT(m_layoutTypes[VK_DESCRIPTOR_TYPE_STORAGE_IMAGE] + m_layoutTypes[VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER] < limits.maxDescriptorSetStorageImages);

    m_handles.resize(m_setLayouts.size());
    for (I32 i = 0; i < m_setLayouts.size(); i++)
    {
        auto& layout = m_setLayouts[i];
        VkDescriptorSetLayoutCreateInfo layoutInfo = vks::initializers::descriptorSetLayoutCreateInfo(layout.LayoutBindings);
        ThrowIfFailed(vkCreateDescriptorSetLayout(m_device->Device, &layoutInfo, nullptr, &m_handles[i]));
    }

    if (m_typesUsageID == ~0)
    {
        CacheTypesUsageID();
    }

    m_allocateInfo = vks::initializers::descriptorSetAllocateInfo();
    m_allocateInfo.descriptorSetCount = m_handles.size();
    m_allocateInfo.pSetLayouts = m_handles.data();
}

DescriptorPoolVulkan::DescriptorPoolVulkan(GPUDeviceVulkan* device, const DescriptorSetLayoutVulkan& layout)
    : m_device(device)
    , m_handle(VK_NULL_HANDLE)
    , m_descriptorSetsMax(0)
    , m_allocatedDescriptorSetsCount(0)
    , m_allocatedDescriptorSetsCountMax(0)
    , m_layout(layout)
{
    //FixedAllocation<VULKAN_DESCRIPTOR_TYPE_END + 1>
    std::vector<VkDescriptorPoolSize> types;
    types.reserve(VULKAN_DESCRIPTOR_TYPE_END + 1);

    // The maximum amount of descriptor sets layout allocations to hold
    const U32 MaxSetsAllocations = 256;
    m_descriptorSetsMax = MaxSetsAllocations * (VULKAN_HASH_POOLS_WITH_TYPES_USAGE_ID ? 1 : m_layout.GetLayouts().size());
    for (U32 typeIndex = VULKAN_DESCRIPTOR_TYPE_BEGIN; typeIndex <= VULKAN_DESCRIPTOR_TYPE_END; typeIndex++)
    {
        const VkDescriptorType descriptorType = (VkDescriptorType)typeIndex;
        const U32 typesUsed = m_layout.GetTypesUsed(descriptorType);
        if (typesUsed > 0)
        {
            types.push_back(VkDescriptorPoolSize{});
            VkDescriptorPoolSize& type = types.back();
            Platform::MemClear(&type, sizeof(type));
            type.type = descriptorType;
            type.descriptorCount = typesUsed * MaxSetsAllocations;
        }
    }

    VkDescriptorPoolCreateInfo createInfo = vks::initializers::descriptorPoolCreateInfo(types, m_descriptorSetsMax);
    ThrowIfFailed(vkCreateDescriptorPool(m_device->Device, &createInfo, nullptr, &m_handle));
}

DescriptorPoolVulkan::~DescriptorPoolVulkan()
{
    if (m_handle != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorPool(m_device->Device, m_handle, nullptr);
    }
}

void DescriptorPoolVulkan::Track(const DescriptorSetLayoutVulkan& layout)
{
#if !IS_RELEASE
    for (U32 typeIndex = VULKAN_DESCRIPTOR_TYPE_BEGIN; typeIndex <= VULKAN_DESCRIPTOR_TYPE_END; typeIndex++)
    {
        ASSERT(m_layout.GetTypesUsed((VkDescriptorType)typeIndex) == layout.GetTypesUsed((VkDescriptorType)typeIndex));
    }
#endif
    m_allocatedDescriptorSetsCount += layout.GetLayouts().size();
    m_allocatedDescriptorSetsCountMax = std::max(m_allocatedDescriptorSetsCount, m_allocatedDescriptorSetsCountMax);
}

void DescriptorPoolVulkan::TrackRemoveUsage(const DescriptorSetLayoutVulkan& layout)
{
    // Check and increment our current type usage
    for (U32 typeIndex = VULKAN_DESCRIPTOR_TYPE_BEGIN; typeIndex <= VULKAN_DESCRIPTOR_TYPE_END; typeIndex++)
    {
        ASSERT(m_layout.GetTypesUsed((VkDescriptorType)typeIndex) == layout.GetTypesUsed((VkDescriptorType)typeIndex));
    }

    m_allocatedDescriptorSetsCount -= layout.GetLayouts().size();
}

void DescriptorPoolVulkan::Reset()
{
    if (m_handle != VK_NULL_HANDLE)
    {
        ThrowIfFailed(vkResetDescriptorPool(m_device->Device, m_handle, 0));
    }
    m_allocatedDescriptorSetsCount = 0;
}

bool DescriptorPoolVulkan::AllocateDescriptorSets(const VkDescriptorSetAllocateInfo& descriptorSetAllocateInfo, VkDescriptorSet* result)
{
    VkDescriptorSetAllocateInfo allocateInfo = descriptorSetAllocateInfo;
    allocateInfo.descriptorPool = m_handle;
    return vkAllocateDescriptorSets(m_device->Device, &allocateInfo, result) == VK_SUCCESS;
}

TypedDescriptorPoolSetVulkan::~TypedDescriptorPoolSetVulkan()
{
    for (auto pool = _poolListHead; pool;)
    {
        const auto next = pool->Next;
        Delete(pool->Element);
        Delete(pool);
        pool = next;
    }
}

bool TypedDescriptorPoolSetVulkan::AllocateDescriptorSets(const DescriptorSetLayoutVulkan& layout, VkDescriptorSet* outSets)
{
    const auto& layoutHandles = layout.GetHandles();
    if (!layoutHandles.empty())
    {
        auto* pool = _poolListCurrent->Element;
        while (!pool->AllocateDescriptorSets(layout.GetAllocateInfo(), outSets))
        {
            pool = GetFreePool(true);
        }
        return true;
    }
    return true;
}

DescriptorPoolVulkan* TypedDescriptorPoolSetVulkan::GetFreePool(bool forceNewPool)
{
    if (!forceNewPool)
    {
        return _poolListCurrent->Element;
    }
    if (_poolListCurrent->Next)
    {
        _poolListCurrent = _poolListCurrent->Next;
        return _poolListCurrent->Element;
    }
    return PushNewPool();
}

DescriptorPoolVulkan* TypedDescriptorPoolSetVulkan::PushNewPool()
{
    auto* newPool = ::New<DescriptorPoolVulkan>(m_device, m_layout);
    if (_poolListCurrent)
    {
        _poolListCurrent->Next = New<PoolList>(newPool);
        _poolListCurrent = _poolListCurrent->Next;
    }
    else
    {
        _poolListCurrent = _poolListHead = New<PoolList>(newPool);
    }
    return newPool;
}

void TypedDescriptorPoolSetVulkan::Reset()
{
    for (PoolList* pool = _poolListHead; pool; pool = pool->Next)
    {
        pool->Element->Reset();
    }
    _poolListCurrent = _poolListHead;
}

DescriptorPoolSetContainerVulkan::DescriptorPoolSetContainerVulkan(GPUDeviceVulkan* device)
    : m_device(device)
    , m_lastFrameUsed(Application::FrameCount)
    , m_used(true)
{
}

DescriptorPoolSetContainerVulkan::~DescriptorPoolSetContainerVulkan()
{
    for (auto& ptr : m_typedDescriptorPools)
    {
        ::Delete(ptr.second);
    }
    m_typedDescriptorPools.clear();
}

TypedDescriptorPoolSetVulkan* DescriptorPoolSetContainerVulkan::AcquireTypedPoolSet(const DescriptorSetLayoutVulkan& layout)
{
    const U32 hash = VULKAN_HASH_POOLS_WITH_TYPES_USAGE_ID ? layout.GetTypesUsageID() : GetHash(layout);
    TypedDescriptorPoolSetVulkan* typedPool;
    auto it = std::find(m_typedDescriptorPools.begin(), m_typedDescriptorPools.end(), hash);
    if (it == m_typedDescriptorPools.end())
    {
        typedPool = New<TypedDescriptorPoolSetVulkan>(m_device, this, layout);
        m_typedDescriptorPools[hash] = typedPool;
    }
    return typedPool;
}

void DescriptorPoolSetContainerVulkan::Reset()
{
    for (auto i = m_typedDescriptorPools.begin(); i != m_typedDescriptorPools.end(); ++i)
    {
        TypedDescriptorPoolSetVulkan* typedPool = i->second;
        typedPool->Reset();
    }
}

void DescriptorPoolSetContainerVulkan::SetUsed(bool used)
{
    m_used = used;
    m_lastFrameUsed = used ? Application::FrameCount : m_lastFrameUsed;
}

DescriptorPoolsManagerVulkan::DescriptorPoolsManagerVulkan(GPUDeviceVulkan* device)
    : m_device(device)
{
}

DescriptorPoolsManagerVulkan::~DescriptorPoolsManagerVulkan()
{
    for (auto& ptr : m_poolSets)
    {
        ::Delete(ptr);
    }
    m_poolSets.clear();
}

DescriptorPoolSetContainerVulkan& DescriptorPoolsManagerVulkan::AcquirePoolSetContainer()
{
    ScopeLock lock(m_locker);

    for (auto* poolSet : m_poolSets)
    {
        if (poolSet->IsUnused())
        {
            poolSet->SetUsed(true);
            poolSet->Reset();
            return *poolSet;
        }
    }

    const auto poolSet = ::New<DescriptorPoolSetContainerVulkan>(m_device);
    m_poolSets.push_back(poolSet);
    return *poolSet;
}

void DescriptorPoolsManagerVulkan::ReleasePoolSet(DescriptorPoolSetContainerVulkan& poolSet)
{
    poolSet.SetUsed(false);
}

void DescriptorPoolsManagerVulkan::GC()
{
    ScopeLock lock(m_locker);
    for (I32 i = m_poolSets.size() - 1; i >= 0; i--)
    {
        const auto poolSet = m_poolSets[i];
        if (poolSet->IsUnused() && Application::FrameCount - poolSet->GetLastFrameUsed() > VULKAN_RESOURCE_DELETE_SAFE_FRAMES_COUNT)
        {
            m_poolSets.erase(m_poolSets.begin() + i);
            Delete(poolSet);
            break;
        }
    }
}

PipelineLayoutVulkan::PipelineLayoutVulkan(GPUDeviceVulkan* device, const DescriptorSetLayoutInfoVulkan& layout)
    : m_device(device)
    , m_handle(VK_NULL_HANDLE)
    , m_descriptorSetLayout(device)
{
    m_descriptorSetLayout.CopyFrom(layout);
    m_descriptorSetLayout.Compile();

    const auto& layoutHandles = m_descriptorSetLayout.GetHandles();

    VkPipelineLayoutCreateInfo createInfo = vks::initializers::pipelineLayoutCreateInfo();
    createInfo.setLayoutCount = (U32)layoutHandles.size();
    createInfo.pSetLayouts = layoutHandles.data();
    ThrowIfFailed(vkCreatePipelineLayout(m_device->Device, &createInfo, nullptr, &m_handle));
}

PipelineLayoutVulkan::~PipelineLayoutVulkan()
{
    if (m_handle != VK_NULL_HANDLE)
    {
        m_device->DeferredDeletionQueue.EnqueueResource(DeferredDeletionQueueVulkan::Type::PipelineLayout, m_handle);
    }
}

U32 DescriptorSetWriterVulkan::SetupDescriptorWrites(const SpirvShaderDescriptorInfo& info, VkWriteDescriptorSet* writeDescriptors, VkDescriptorImageInfo* imageInfo, VkDescriptorBufferInfo* bufferInfo, U8* bindingToDynamicOffset)
{
    ASSERT(info.DescriptorTypesCount <= 64);
    WriteDescriptors = writeDescriptors;
    WritesCount = info.DescriptorTypesCount;
    BindingToDynamicOffset = bindingToDynamicOffset;

    U32 dynamicOffsetIndex = 0;
    for (U32 i = 0; i < info.DescriptorTypesCount; i++)
    {
        writeDescriptors->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptors->dstBinding = i;
        writeDescriptors->descriptorCount = 1;
        writeDescriptors->descriptorType = info.DescriptorTypes[i].DescriptorType;

        switch (writeDescriptors->descriptorType)
        {
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
            BindingToDynamicOffset[i] = dynamicOffsetIndex;
            dynamicOffsetIndex++;
            writeDescriptors->pBufferInfo = bufferInfo++;
            break;
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            writeDescriptors->pBufferInfo = bufferInfo++;
            break;
        case VK_DESCRIPTOR_TYPE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
            writeDescriptors->pImageInfo = imageInfo++;
            break;
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
            break;
        default:
            break;
        }

        ++writeDescriptors;
    }

    return dynamicOffsetIndex;
}