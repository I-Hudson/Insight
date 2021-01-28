#pragma once

#include "Engine/Platform/Platform.h"
#include "Engine/Platform/CriticalSection.h"
#include "Types.h"
#include "VulkanHeaders.h"
#include "VulkanPlatform.h"
#include "Engine/Graphics/Shaders/Config.h"
#include "Config.h"

#define VULKAN_DESCRIPTOR_TYPE_BEGIN VK_DESCRIPTOR_TYPE_SAMPLER
#define VULKAN_DESCRIPTOR_TYPE_END VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT

class GPUDeviceVulkan;
class CmdBufferVulkan;
class GPUContextVulkan;

namespace DescriptorSet
{
    enum Stage
    {
        // Vertex shader stage
        Vertex = 0,

        // Pixel shader stage
        Pixel = 1,

        // Geometry shader stage
        Geometry = 2,

        // Hull shader stage
        Hull = 3,

        // Domain shader stage
        Domain = 4,

        // Graphics pipeline stages count
        GraphicsStagesCount = 5,

        // Compute pipeline slot
        Compute = 0,

        // The maximum amount of slots for all stages
        Max = 5,
    };

    INLINE Stage GetSetForFrequency(ShaderStage stage)
    {
        switch (stage)
        {
        case ShaderStage::Vertex:
            return Vertex;
        case ShaderStage::Hull:
            return Hull;
        case ShaderStage::Domain:
            return Domain;
        case ShaderStage::Pixel:
            return Pixel;
        case ShaderStage::Geometry:
            return Geometry;
        case ShaderStage::Compute:
            return Compute;
        default:
            return Max;
        }
    }

    INLINE ShaderStage GetFrequencyForGfxSet(Stage stage)
    {
        switch (stage)
        {
        case Vertex:
            return ShaderStage::Vertex;
        case Hull:
            return ShaderStage::Hull;
        case Domain:
            return ShaderStage::Domain;
        case Pixel:
            return ShaderStage::Pixel;
        case Geometry:
            return ShaderStage::Geometry;
        default:
            return static_cast<ShaderStage>(ShaderStage_Count);
        }
    }

    template<typename T>
    INLINE bool CopyAndReturnNotEqual(T& a, T b)
    {
        const bool result = a != b;
        a = b;
        return result;
    }
};

/// <summary>
/// Define a single descriptor set within a shader.
/// </summary>
class DescriptorSetLayoutInfoVulkan
{
public:
    struct SetLayout
    {
        std::vector<VkDescriptorSetLayoutBinding> LayoutBindings;
    };

    DescriptorSetLayoutInfoVulkan()
    {
        Platform::MemClear(m_layoutTypes, sizeof(m_layoutTypes));
    }

    INLINE U32 GetTypesUsed(VkDescriptorType type) const
    {
        return m_layoutTypes[type];
    }

    const std::vector<SetLayout>& GetLayouts() const
    {
        return m_setLayouts;
    }

    INLINE const U32* GetLayoutTypes() const
    {
        return m_layoutTypes;
    }

    INLINE U32 GetTypesUsageID() const
    {
        return m_typesUsageID;
    }

    void AddBindingsForStage(VkShaderStageFlagBits stageFlags, DescriptorSet::Stage descSet, const SpirvShaderDescriptorInfo* descriptorInfo);

    void CopyFrom(const DescriptorSetLayoutInfoVulkan& info)
    {
        Platform::MemCopy(m_layoutTypes, info.m_layoutTypes, sizeof(m_layoutTypes));
        m_hash = info.m_hash;
        m_typesUsageID = info.m_typesUsageID;
        m_setLayouts = info.m_setLayouts;
    }

    INLINE bool operator ==(const DescriptorSetLayoutInfoVulkan& other) const
    {
        if (other.m_setLayouts.size() != m_setLayouts.size())
            return false;
        if (other.m_typesUsageID != m_typesUsageID)
            return false;

        for (I32 index = 0; index < other.m_setLayouts.size(); ++index)
        {
            const I32 bindingsCount = m_setLayouts[index].LayoutBindings.size();
            if (other.m_setLayouts[index].LayoutBindings.size() != bindingsCount)
                return false;

            if (bindingsCount != 0 && Platform::MemCompare(other.m_setLayouts[index].LayoutBindings.data(), m_setLayouts[index].LayoutBindings.data(), bindingsCount * sizeof(VkDescriptorSetLayoutBinding)))
                return false;
        }

        return true;
    }

    friend INLINE U32 GetHash(const DescriptorSetLayoutInfoVulkan& key)
    {
        return key.m_hash;
    }

protected:

    U32 m_layoutTypes[VULKAN_DESCRIPTOR_TYPE_END];
    std::vector<SetLayout> m_setLayouts;
    U32 m_hash = 0;
    U32 m_typesUsageID = ~0;

    void CacheTypesUsageID();
    void AddDescriptor(I32 descriptorSetIndex, const VkDescriptorSetLayoutBinding& descriptor);

};

/// <summary>
/// 
/// </summary>
class DescriptorSetLayoutVulkan : public DescriptorSetLayoutInfoVulkan
{
public:

    using DescriptorSetLayoutHandlesArray = std::vector<VkDescriptorSetLayout>;

    DescriptorSetLayoutVulkan(GPUDeviceVulkan* device);
    ~DescriptorSetLayoutVulkan();


    INLINE const DescriptorSetLayoutHandlesArray& GetHandles() const
    {
        return m_handles;
    }

    INLINE const VkDescriptorSetAllocateInfo& GetAllocateInfo() const
    {
        return m_allocateInfo;
    }

    friend INLINE U32 GetHash(const DescriptorSetLayoutVulkan& key)
    {
        return key.m_hash;
    }

    void Compile();

private:
    GPUDeviceVulkan* m_device;
    DescriptorSetLayoutHandlesArray m_handles;
    VkDescriptorSetAllocateInfo m_allocateInfo;
};

class DescriptorPoolVulkan
{
public:
    DescriptorPoolVulkan(GPUDeviceVulkan* device, const DescriptorSetLayoutVulkan& layout);
    ~DescriptorPoolVulkan();

    INLINE VkDescriptorPool GetHandle() const
    {
        return m_handle;
    }

    INLINE bool IsEmpty() const
    {
        return m_allocatedDescriptorSetsCount == 0;
    }

    INLINE bool CanAllocate(const DescriptorSetLayoutVulkan& layout) const
    {
        return m_descriptorSetsMax > m_allocatedDescriptorSetsCount + layout.GetLayouts().size();
    }

    INLINE U32 GetAllocatedDescriptorSetsCount() const
    {
        return m_allocatedDescriptorSetsCount;
    }

    void Track(const DescriptorSetLayoutVulkan& layout);
    void TrackRemoveUsage(const DescriptorSetLayoutVulkan& layout);
    void Reset();
    bool AllocateDescriptorSets(const VkDescriptorSetAllocateInfo& descriptorSetAllocateInfo, VkDescriptorSet* result);

private:
    GPUDeviceVulkan* m_device;
    VkDescriptorPool m_handle;

    U32 m_descriptorSetsMax;
    U32 m_allocatedDescriptorSetsCount;
    U32 m_allocatedDescriptorSetsCountMax;

    const DescriptorSetLayoutVulkan& m_layout;
};

class DescriptorPoolSetContainerVulkan;

class TypedDescriptorPoolSetVulkan
{
public:
    TypedDescriptorPoolSetVulkan(GPUDeviceVulkan* device, const DescriptorPoolSetContainerVulkan* owner, const DescriptorSetLayoutVulkan& layout)
        : m_device(device)
        , m_owner(owner)
        , m_layout(layout)
    {
        PushNewPool();
    };

    ~TypedDescriptorPoolSetVulkan();

    bool AllocateDescriptorSets(const DescriptorSetLayoutVulkan& layout, VkDescriptorSet* outSets);

    const DescriptorPoolSetContainerVulkan* GetOwner() const
    {
        return m_owner;
    }

private:
    DescriptorPoolVulkan* GetFreePool(bool forceNewPool = false);
    DescriptorPoolVulkan* PushNewPool();
    void Reset();

    GPUDeviceVulkan* m_device;
    const DescriptorPoolSetContainerVulkan* m_owner;
    const DescriptorSetLayoutVulkan& m_layout;

    class PoolList
    {
    public:

        DescriptorPoolVulkan* Element;
        PoolList* Next;

        PoolList(DescriptorPoolVulkan* element, PoolList* next = nullptr)
        {
            Element = element;
            Next = next;
        }
    };

    PoolList* _poolListHead = nullptr;
    PoolList* _poolListCurrent = nullptr;

    friend DescriptorPoolSetContainerVulkan;
};

class DescriptorPoolSetContainerVulkan
{
public:

    DescriptorPoolSetContainerVulkan(GPUDeviceVulkan* device);

    ~DescriptorPoolSetContainerVulkan();

    TypedDescriptorPoolSetVulkan* AcquireTypedPoolSet(const DescriptorSetLayoutVulkan& layout);
    void Reset();
    void SetUsed(bool used);

    bool IsUnused() const
    {
        return !m_used;
    }

    U64 GetLastFrameUsed() const
    {
        return m_lastFrameUsed;
    }

private:
    GPUDeviceVulkan* m_device;
    std::unordered_map<U32, TypedDescriptorPoolSetVulkan*> m_typedDescriptorPools;
    U64 m_lastFrameUsed;
    bool m_used;
};

class DescriptorPoolsManagerVulkan
{
public:

    DescriptorPoolsManagerVulkan(GPUDeviceVulkan* device);
    ~DescriptorPoolsManagerVulkan();

    DescriptorPoolSetContainerVulkan& AcquirePoolSetContainer();
    void ReleasePoolSet(DescriptorPoolSetContainerVulkan& poolSet);
    void GC();

private:

    GPUDeviceVulkan* m_device = nullptr;
    CriticalSection m_locker;
    std::vector<DescriptorPoolSetContainerVulkan*> m_poolSets;
};

class PipelineLayoutVulkan
{
public:

    PipelineLayoutVulkan(GPUDeviceVulkan* device, const DescriptorSetLayoutInfoVulkan& layout);
    ~PipelineLayoutVulkan();

    INLINE VkPipelineLayout GetHandle() const
    {
        return m_handle;
    }

    INLINE const DescriptorSetLayoutVulkan& GetDescriptorSetLayout() const
    {
        return m_descriptorSetLayout;
    }

    INLINE bool HasDescriptors() const
    {
        return !m_descriptorSetLayout.GetLayouts().empty();
    }

private:

    GPUDeviceVulkan* m_device;
    VkPipelineLayout m_handle;
    DescriptorSetLayoutVulkan m_descriptorSetLayout;
};

struct DescriptorSetWriteContainerVulkan
{
    std::vector<VkDescriptorImageInfo> DescriptorImageInfo;
    std::vector<VkDescriptorBufferInfo> DescriptorBufferInfo;
    std::vector<VkWriteDescriptorSet> DescriptorWrites;
    std::vector<Byte> BindingToDynamicOffset;

    void Release()
    {
        DescriptorImageInfo.resize(0);
        DescriptorBufferInfo.resize(0);
        DescriptorWrites.resize(0);
        BindingToDynamicOffset.resize(0);
    }
};

class DescriptorSetWriterVulkan
{
public:

    VkWriteDescriptorSet* WriteDescriptors = nullptr;
    Byte* BindingToDynamicOffset = nullptr;
    U32* DynamicOffsets = nullptr;
    U32 WritesCount = 0;

public:

    U32 SetupDescriptorWrites(const SpirvShaderDescriptorInfo& info, VkWriteDescriptorSet* writeDescriptors, VkDescriptorImageInfo* imageInfo, VkDescriptorBufferInfo* bufferInfo, Byte* bindingToDynamicOffset);

    bool WriteUniformBuffer(U32 descriptorIndex, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range) const
    {
        ASSERT(descriptorIndex < WritesCount);
        ASSERT(WriteDescriptors[descriptorIndex].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        VkDescriptorBufferInfo* bufferInfo = const_cast<VkDescriptorBufferInfo*>(WriteDescriptors[descriptorIndex].pBufferInfo);
        ASSERT(bufferInfo);
        bool edited = DescriptorSet::CopyAndReturnNotEqual(bufferInfo->buffer, buffer);
        edited |= DescriptorSet::CopyAndReturnNotEqual(bufferInfo->offset, offset);
        edited |= DescriptorSet::CopyAndReturnNotEqual(bufferInfo->range, range);
        return edited;
    }

    bool WriteDynamicUniformBuffer(U32 descriptorIndex, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range, U32 dynamicOffset) const
    {
        ASSERT(descriptorIndex < WritesCount);
        ASSERT(WriteDescriptors[descriptorIndex].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);
        VkDescriptorBufferInfo* bufferInfo = const_cast<VkDescriptorBufferInfo*>(WriteDescriptors[descriptorIndex].pBufferInfo);
        ASSERT(bufferInfo);
        bool edited = DescriptorSet::CopyAndReturnNotEqual(bufferInfo->buffer, buffer);
        edited |= DescriptorSet::CopyAndReturnNotEqual(bufferInfo->offset, offset);
        edited |= DescriptorSet::CopyAndReturnNotEqual(bufferInfo->range, range);
        const Byte dynamicOffsetIndex = BindingToDynamicOffset[descriptorIndex];
        DynamicOffsets[dynamicOffsetIndex] = dynamicOffset;
        return edited;
    }

    bool WriteSampler(U32 descriptorIndex, VkSampler sampler) const
    {
        ASSERT(descriptorIndex < WritesCount);
        ASSERT(WriteDescriptors[descriptorIndex].descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER || WriteDescriptors[descriptorIndex].descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
        VkDescriptorImageInfo* imageInfo = const_cast<VkDescriptorImageInfo*>(WriteDescriptors[descriptorIndex].pImageInfo);
        ASSERT(imageInfo);
        bool edited = DescriptorSet::CopyAndReturnNotEqual(imageInfo->sampler, sampler);
        return edited;
    }

    bool WriteImage(U32 descriptorIndex, VkImageView imageView, VkImageLayout layout) const
    {
        ASSERT(descriptorIndex < WritesCount);
        ASSERT(WriteDescriptors[descriptorIndex].descriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE || WriteDescriptors[descriptorIndex].descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
        VkDescriptorImageInfo* imageInfo = const_cast<VkDescriptorImageInfo*>(WriteDescriptors[descriptorIndex].pImageInfo);
        ASSERT(imageInfo);
        bool edited = DescriptorSet::CopyAndReturnNotEqual(imageInfo->imageView, imageView);
        edited |= DescriptorSet::CopyAndReturnNotEqual(imageInfo->imageLayout, layout);
        return edited;
    }

    bool WriteStorageImage(U32 descriptorIndex, VkImageView imageView, VkImageLayout layout) const
    {
        ASSERT(descriptorIndex < WritesCount);
        ASSERT(WriteDescriptors[descriptorIndex].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
        VkDescriptorImageInfo* imageInfo = const_cast<VkDescriptorImageInfo*>(WriteDescriptors[descriptorIndex].pImageInfo);
        ASSERT(imageInfo);
        bool edited = DescriptorSet::CopyAndReturnNotEqual(imageInfo->imageView, imageView);
        edited |= DescriptorSet::CopyAndReturnNotEqual(imageInfo->imageLayout, layout);
        return edited;
    }

    bool WriteStorageTexelBuffer(U32 descriptorIndex, const VkBufferView* bufferView) const
    {
        ASSERT(descriptorIndex < WritesCount);
        ASSERT(WriteDescriptors[descriptorIndex].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER);
        WriteDescriptors[descriptorIndex].pTexelBufferView = bufferView;
        return true;
    }

    bool WriteStorageBuffer(U32 descriptorIndex, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range) const
    {
        ASSERT(descriptorIndex < WritesCount);
        ASSERT(WriteDescriptors[descriptorIndex].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER || WriteDescriptors[descriptorIndex].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC);
        VkDescriptorBufferInfo* bufferInfo = const_cast<VkDescriptorBufferInfo*>(WriteDescriptors[descriptorIndex].pBufferInfo);
        ASSERT(bufferInfo);
        bool edited = DescriptorSet::CopyAndReturnNotEqual(bufferInfo->buffer, buffer);
        edited |= DescriptorSet::CopyAndReturnNotEqual(bufferInfo->offset, offset);
        edited |= DescriptorSet::CopyAndReturnNotEqual(bufferInfo->range, range);
        return edited;
    }

    bool WriteUniformTexelBuffer(U32 descriptorIndex, const VkBufferView* view) const
    {
        ASSERT(descriptorIndex < WritesCount);
        ASSERT(WriteDescriptors[descriptorIndex].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER);
        return DescriptorSet::CopyAndReturnNotEqual(WriteDescriptors[descriptorIndex].pTexelBufferView, view);
    }

    void SetDescriptorSet(VkDescriptorSet descriptorSet) const
    {
        for (U32 i = 0; i < WritesCount; i++)
        {
            WriteDescriptors[i].dstSet = descriptorSet;
        }
    }
};
