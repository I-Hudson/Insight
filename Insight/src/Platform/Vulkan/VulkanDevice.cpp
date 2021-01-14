#include "ispch.h"
#include "VulkanDevice.h"

#include "Insight/Module/WindowModule.h"
#include "Insight/Config/Config.h"


namespace vks
{
	/**
	* Default constructor
	*
	* @param m_physicalDevice Physical device that is to be used
	*/
	VulkanDevice::VulkanDevice(VkPhysicalDevice m_physicalDevice)
	{
		assert(m_physicalDevice);
		this->m_physicalDevice = m_physicalDevice;

		// Store Properties m_features, limits and m_properties of the physical device for later use
		// Device m_properties also contain limits and sparse m_properties
		vkGetPhysicalDeviceProperties(m_physicalDevice, &m_properties);
		// Features should be checked by the examples before using them
		vkGetPhysicalDeviceFeatures(m_physicalDevice, &m_features);
		// Memory m_properties are used regularly for creating all kinds of buffers
		vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &m_memoryProperties);
		// Queue family m_properties, used for setting up requested queues upon device creation
		uint32_t queueFamilyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);
		assert(queueFamilyCount > 0);
		m_queueFamilyProperties.resize(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, m_queueFamilyProperties.data());

		// Get list of supported extensions
		uint32_t extCount = 0;
		vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extCount, nullptr);
		if (extCount > 0)
		{
			std::vector<VkExtensionProperties> extensions(extCount);
			if (vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extCount, &extensions.front()) == VK_SUCCESS)
			{
				for (auto ext : extensions)
				{
					m_supportedExtensions.push_back(ext.extensionName);
				}
			}
		}
	}

	/**
	* Default destructor
	*
	* @note Frees the logical device
	*/
	VulkanDevice::~VulkanDevice()
	{
		if (m_commandPool)
		{
			vkDestroyCommandPool(m_logicalDevice, m_commandPool, nullptr);
		}
		if (m_pipelineCache)
		{
			vkDestroyPipelineCache(m_logicalDevice, m_pipelineCache, nullptr);
		}
		if (m_descriptorPool != VK_NULL_HANDLE)
		{
			vkDestroyDescriptorPool(m_logicalDevice, m_descriptorPool, nullptr);
		}
		vkDestroyRenderPass(m_logicalDevice, m_renderPass, nullptr);
		if (m_logicalDevice)
		{
			vkDestroyDevice(m_logicalDevice, nullptr);
		}
	}

	/**
	* Get the index of a memory type that has all the requested property bits set
	*
	* @param typeBits Bit mask with bits set for each memory type supported by the resource to request for (from VkMemoryRequirements)
	* @param m_properties Bit mask of m_properties for the memory type to request
	* @param (Optional) memTypeFound Pointer to a bool that is set to true if a matching memory type has been found
	*
	* @return Index of the requested memory type
	*
	* @throw Throws an exception if memTypeFound is null and no memory type could be found that supports the requested m_properties
	*/
	uint32_t VulkanDevice::GetMemoryType(uint32_t typeBits, VkMemoryPropertyFlags m_properties, VkBool32* memTypeFound) const
	{
		for (uint32_t i = 0; i < m_memoryProperties.memoryTypeCount; i++)
		{
			if ((typeBits & 1) == 1)
			{
				if ((m_memoryProperties.memoryTypes[i].propertyFlags & m_properties) == m_properties)
				{
					if (memTypeFound)
					{
						*memTypeFound = true;
					}
					return i;
				}
			}
			typeBits >>= 1;
		}

		if (memTypeFound)
		{
			*memTypeFound = false;
			return 0;
		}
		else
		{
			throw std::runtime_error("Could not find a matching memory type");
		}
	}

	uint32_t& VulkanDevice::GetQueueFamilyIndex(VkQueueFlagBits queueFlags)
	{
		switch (queueFlags)
		{
			case VK_QUEUE_GRAPHICS_BIT: return m_queueFamilyIndices.graphics;
			case VK_QUEUE_COMPUTE_BIT:  return m_queueFamilyIndices.graphics;
			case VK_QUEUE_TRANSFER_BIT:  return m_queueFamilyIndices.transfer;
		}
		return m_queueFamilyIndices.graphics;
	}

	/**
	* Get the index of a queue family that supports the requested queue flags
	*
	* @param queueFlags Queue flags to find a queue family index for
	*
	* @return Index of the queue family index that matches the flags
	*
	* @throw Throws an exception if no queue family index could be found that supports the requested flags
	*/
	uint32_t VulkanDevice::QueryQueueFamilyIndex(VkQueueFlagBits queueFlags) const
	{
		// Dedicated queue for compute
		// Try to find a queue family index that supports compute but not graphics
		if (queueFlags & VK_QUEUE_COMPUTE_BIT)
		{
			for (uint32_t i = 0; i < static_cast<uint32_t>(m_queueFamilyProperties.size()); i++)
			{
				if ((m_queueFamilyProperties[i].queueFlags & queueFlags) && ((m_queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
				{
					return i;
				}
			}
		}

		// Dedicated queue for transfer
		// Try to find a queue family index that supports transfer but not graphics and compute
		if (queueFlags & VK_QUEUE_TRANSFER_BIT)
		{
			for (uint32_t i = 0; i < static_cast<uint32_t>(m_queueFamilyProperties.size()); i++)
			{
				if ((m_queueFamilyProperties[i].queueFlags & queueFlags) && ((m_queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) && ((m_queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0))
				{
					return i;
				}
			}
		}

		// For other queue types or if no separate compute queue is present, return the first one to support the requested flags
		for (uint32_t i = 0; i < static_cast<uint32_t>(m_queueFamilyProperties.size()); i++)
		{
			if (m_queueFamilyProperties[i].queueFlags & queueFlags)
			{
				return i;
			}
		}

		throw std::runtime_error("Could not find a matching queue family index");
	}

	/**
	* Create the logical device based on the assigned physical device, also gets default queue family indices
	*
	* @param m_enabledFeatures Can be used to enable certain m_features upon device creation
	* @param pNextChain Optional chain of pointer to extension structures
	* @param useSwapChain Set to false for headless rendering to omit the swapchain device extensions
	* @param requestedQueueTypes Bit flags specifying the queue types to be requested from the device
	*
	* @return VkResult of the device creation call
	*/
	VkResult VulkanDevice::CreateLogicalDevice(VkPhysicalDeviceFeatures m_enabledFeatures, std::vector<const char*> enabledExtensions, void* pNextChain, bool useSwapChain, VkQueueFlags requestedQueueTypes)
	{
		// Desired queues need to be requested upon logical device creation
		// Due to differing queue family configurations of Vulkan implementations this can be a bit tricky, especially if the application
		// requests different queue types

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};

		// Get queue family indices for the requested queue family types
		// Note that the indices may overlap depending on the implementation

		const float defaultQueuePriority(0.0f);

		// Graphics queue
		if (requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT)
		{
			m_queueFamilyIndices.graphics = QueryQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
			VkDeviceQueueCreateInfo queueInfo{};
			queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.queueFamilyIndex = m_queueFamilyIndices.graphics;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &defaultQueuePriority;
			queueCreateInfos.push_back(queueInfo);
		}
		else
		{
			m_queueFamilyIndices.graphics = VK_NULL_HANDLE;
		}

		// Dedicated compute queue
		if (requestedQueueTypes & VK_QUEUE_COMPUTE_BIT)
		{
			m_queueFamilyIndices.compute = QueryQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT);
			if (m_queueFamilyIndices.compute != m_queueFamilyIndices.graphics)
			{
				// If compute family index differs, we need an additional queue create info for the compute queue
				VkDeviceQueueCreateInfo queueInfo{};
				queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueInfo.queueFamilyIndex = m_queueFamilyIndices.compute;
				queueInfo.queueCount = 1;
				queueInfo.pQueuePriorities = &defaultQueuePriority;
				queueCreateInfos.push_back(queueInfo);
			}
		}
		else
		{
			// Else we use the same queue
			m_queueFamilyIndices.compute = m_queueFamilyIndices.graphics;
		}

		// Dedicated transfer queue
		if (requestedQueueTypes & VK_QUEUE_TRANSFER_BIT)
		{
			m_queueFamilyIndices.transfer = QueryQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT);
			if ((m_queueFamilyIndices.transfer != m_queueFamilyIndices.graphics) && (m_queueFamilyIndices.transfer != m_queueFamilyIndices.compute))
			{
				// If compute family index differs, we need an additional queue create info for the compute queue
				VkDeviceQueueCreateInfo queueInfo{};
				queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueInfo.queueFamilyIndex = m_queueFamilyIndices.transfer;
				queueInfo.queueCount = 1;
				queueInfo.pQueuePriorities = &defaultQueuePriority;
				queueCreateInfos.push_back(queueInfo);
			}
		}
		else
		{
			// Else we use the same queue
			m_queueFamilyIndices.transfer = m_queueFamilyIndices.graphics;
		}

		// Create the logical device representation
		std::vector<const char*> deviceExtensions(enabledExtensions);
		if (useSwapChain)
		{
			// If the device will be used for presenting to a display via a swapchain we need to request the swapchain extension
			deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		}

		VkDeviceCreateInfo deviceCreateInfo = {};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());;
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
		deviceCreateInfo.pEnabledFeatures = &m_enabledFeatures;

		// If a pNext(Chain) has been passed, we need to add it to the device creation info
		VkPhysicalDeviceFeatures2 physicalDeviceFeatures2{};
		if (pNextChain)
		{
			physicalDeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
			physicalDeviceFeatures2.features = m_enabledFeatures;
			physicalDeviceFeatures2.pNext = pNextChain;
			deviceCreateInfo.pEnabledFeatures = nullptr;
			deviceCreateInfo.pNext = &physicalDeviceFeatures2;
		}

		// Enable the debug marker extension if it is present (likely meaning a debugging tool is present)
		if (ExtensionSupported(VK_EXT_DEBUG_MARKER_EXTENSION_NAME))
		{
			deviceExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
			m_enableDebugMarkers = true;
		}

		if (deviceExtensions.size() > 0)
		{
			for (const char* enabledExtension : deviceExtensions)
			{
				if (!ExtensionSupported(enabledExtension))
				{
					std::cerr << "Enabled device extension \"" << enabledExtension << "\" is not present at device level\n";
				}
			}

			deviceCreateInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
			deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
		}

		this->m_enabledFeatures = m_enabledFeatures;

		VkResult result = vkCreateDevice(m_physicalDevice, &deviceCreateInfo, nullptr, &m_logicalDevice);
		if (result != VK_SUCCESS)
		{
			return result;
		}

		vkGetDeviceQueue(m_logicalDevice, m_queueFamilyIndices.graphics, 0, &m_queueFamily.graphics);
		vkGetDeviceQueue(m_logicalDevice, m_queueFamilyIndices.compute, 0, &m_queueFamily.compute);
		vkGetDeviceQueue(m_logicalDevice, m_queueFamilyIndices.transfer, 0, &m_queueFamily.transfer);

		// Create a default command pool for graphics command buffers
		m_commandPool = CreateCommandPool(m_queueFamilyIndices.graphics);


		ThrowIfFailed(vkCreatePipelineCache(m_logicalDevice, &vks::initializers::pipelineCacheCreateInfo(), nullptr, &m_pipelineCache));
		CreateDescriptorPool();

		return result;
	}

	/**
	* Create a buffer on the device
	*
	* @param usageFlags Usage flag bit mask for the buffer (i.e. index, vertex, uniform buffer)
	* @param memoryPropertyFlags Memory m_properties for this buffer (i.e. device local, host visible, coherent)
	* @param size Size of the buffer in byes
	* @param buffer Pointer to the buffer handle acquired by the function
	* @param memory Pointer to the memory handle acquired by the function
	* @param data Pointer to the data that should be copied to the buffer after creation (optional, if not set, no data is copied over)
	*
	* @return VK_SUCCESS if buffer handle and memory have been created and (optionally passed) data has been copied
	*/
	VkResult VulkanDevice::CreateBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory* memory, void* data)
	{
		MutexLockGuard l(m_mutex);

		// Create the buffer handle
		VkBufferCreateInfo bufferCreateInfo = vks::initializers::bufferCreateInfo(usageFlags, size);
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		ThrowIfFailed(vkCreateBuffer(m_logicalDevice, &bufferCreateInfo, nullptr, buffer));

		// Create the memory backing up the buffer handle
		VkMemoryRequirements memReqs;
		VkMemoryAllocateInfo memAlloc = vks::initializers::memoryAllocateInfo();
		vkGetBufferMemoryRequirements(m_logicalDevice, *buffer, &memReqs);
		memAlloc.allocationSize = memReqs.size;
		// Find a memory type index that fits the m_properties of the buffer
		memAlloc.memoryTypeIndex = GetMemoryType(memReqs.memoryTypeBits, memoryPropertyFlags);
		// If the buffer has VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT set we also need to enable the appropriate flag during allocation
		VkMemoryAllocateFlagsInfoKHR allocFlagsInfo{};
		if (usageFlags & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
		{
			allocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR;
			allocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
			memAlloc.pNext = &allocFlagsInfo;
		}
		ThrowIfFailed(vkAllocateMemory(m_logicalDevice, &memAlloc, nullptr, memory));

		// If a pointer to the buffer data has been passed, map the buffer and copy over the data
		if (data != nullptr)
		{
			void* mapped;
			ThrowIfFailed(vkMapMemory(m_logicalDevice, *memory, 0, size, 0, &mapped));
			memcpy(mapped, data, size);
			// If host coherency hasn't been requested, do a manual flush to make writes visible
			if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
			{
				VkMappedMemoryRange mappedRange = vks::initializers::mappedMemoryRange();
				mappedRange.memory = *memory;
				mappedRange.offset = 0;
				mappedRange.size = size;
				vkFlushMappedMemoryRanges(m_logicalDevice, 1, &mappedRange);
			}
			vkUnmapMemory(m_logicalDevice, *memory);
		}

		// Attach the memory to the buffer object
		ThrowIfFailed(vkBindBufferMemory(m_logicalDevice, *buffer, *memory, 0));

		return VK_SUCCESS;
	}

	/**
	* Create a buffer on the device
	*
	* @param usageFlags Usage flag bit mask for the buffer (i.e. index, vertex, uniform buffer)
	* @param memoryPropertyFlags Memory m_properties for this buffer (i.e. device local, host visible, coherent)
	* @param buffer Pointer to a vk::Vulkan buffer object
	* @param size Size of the buffer in bytes
	* @param data Pointer to the data that should be copied to the buffer after creation (optional, if not set, no data is copied over)
	*
	* @return VK_SUCCESS if buffer handle and memory have been created and (optionally passed) data has been copied
	*/
	VkResult VulkanDevice::CreateBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, vks::VulkanBuffer* buffer, void* data)
	{
		MutexLockGuard lock(m_mutex);

		buffer->device = m_logicalDevice;

		// Create the buffer handle
		VkBufferCreateInfo bufferCreateInfo = vks::initializers::bufferCreateInfo(usageFlags, size);
		ThrowIfFailed(vkCreateBuffer(m_logicalDevice, &bufferCreateInfo, nullptr, &buffer->buffer));

		// Create the memory backing up the buffer handle
		VkMemoryRequirements memReqs;
		VkMemoryAllocateInfo memAlloc = vks::initializers::memoryAllocateInfo();
		vkGetBufferMemoryRequirements(m_logicalDevice, buffer->buffer, &memReqs);
		memAlloc.allocationSize = memReqs.size;
		// Find a memory type index that fits the m_properties of the buffer
		memAlloc.memoryTypeIndex = GetMemoryType(memReqs.memoryTypeBits, memoryPropertyFlags);
		// If the buffer has VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT set we also need to enable the appropriate flag during allocation
		VkMemoryAllocateFlagsInfoKHR allocFlagsInfo{};
		if (usageFlags & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
		{
			allocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR;
			allocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
			memAlloc.pNext = &allocFlagsInfo;
		}
		ThrowIfFailed(vkAllocateMemory(m_logicalDevice, &memAlloc, nullptr, &buffer->memory));

		buffer->alignment = memReqs.alignment;
		buffer->size = size;
		buffer->usageFlags = usageFlags;
		buffer->memoryPropertyFlags = memoryPropertyFlags;

		// If a pointer to the buffer data has been passed, map the buffer and copy over the data
		if (data != nullptr)
		{
			ThrowIfFailed(buffer->Map());
			memcpy(buffer->mapped, data, size);
			if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
				buffer->Flush();

			buffer->Unmap();
		}

		// Initialize a default descriptor that covers the whole buffer size
		buffer->SetupDescriptor();

		// Attach the memory to the buffer object
		return buffer->Bind();;
	}

	void VulkanDevice::CreateBufferGPU(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, vks::VulkanBuffer* buffer, void* data)
	{
		vks::VulkanBuffer stagingBuffer;
		CreateBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
					 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					 size,
					 &stagingBuffer,
					 data);

		CreateBuffer(usageFlags,
					 memoryPropertyFlags,
					 size,
					 buffer);


		VkCommandBuffer copyCmd = vks::VulkanDevice::Instance()->CreateSingleUseBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
		VkBufferCopy copyRegion = {};
		copyRegion.size = size;

		MutexUnqiueLock lock(m_mutex);
		vkCmdCopyBuffer(copyCmd, stagingBuffer.buffer, buffer->buffer, 1, &copyRegion);
		lock.unlock();
		FlushCommandBuffer(copyCmd, m_queueFamily.transfer, true);

		lock.lock();
		stagingBuffer.Destroy();
		lock.unlock();
	}

	/**
	* Copy buffer data from src to dst using VkCmdCopyBuffer
	*
	* @param src Pointer to the source buffer to copy from
	* @param dst Pointer to the destination buffer to copy to
	* @param queue Pointer
	* @param copyRegion (Optional) Pointer to a copy region, if NULL, the whole buffer is copied
	*
	* @note Source and destination pointers must have the appropriate transfer usage flags set (TRANSFER_SRC / TRANSFER_DST)
	*/
	void VulkanDevice::CopyBuffer(vks::VulkanBuffer* src, vks::VulkanBuffer* dst, VkQueue queue, VkBufferCopy* copyRegion)
	{
		assert(dst->size <= src->size);
		assert(src->buffer);
		VkCommandBuffer copyCmd = CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
		VkBufferCopy bufferCopy{};
		if (copyRegion == nullptr)
		{
			bufferCopy.size = src->size;
		}
		else
		{
			bufferCopy = *copyRegion;
		}

		MutexUnqiueLock lock(m_mutex);
		vkCmdCopyBuffer(copyCmd, src->buffer, dst->buffer, 1, &bufferCopy);
		lock.unlock();
		FlushCommandBuffer(copyCmd, queue);
	}

	/**
	* Create a command pool for allocation command buffers from
	*
	* @param queueFamilyIndex Family index of the queue to create the command pool for
	* @param createFlags (Optional) Command pool creation flags (Defaults to VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)
	*
	* @note Command buffers allocated from the created pool can only be submitted to a queue with the same family index
	*
	* @return A handle to the created command buffer
	*/
	VkCommandPool VulkanDevice::CreateCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags)
	{
		MutexLockGuard lock(m_mutex);

		VkCommandPoolCreateInfo cmdPoolInfo = {};
		cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolInfo.queueFamilyIndex = queueFamilyIndex;
		cmdPoolInfo.flags = createFlags;
		VkCommandPool cmdPool;
		ThrowIfFailed(vkCreateCommandPool(m_logicalDevice, &cmdPoolInfo, nullptr, &cmdPool));
		return cmdPool;
	}

	/**
	* Allocate a command buffer from the command pool
	*
	* @param level Level of the new command buffer (primary or secondary)
	* @param pool Command pool from which the command buffer will be allocated
	* @param (Optional) begin If true, recording on the new command buffer will be started (vkBeginCommandBuffer) (Defaults to false)
	*
	* @return A handle to the allocated command buffer
	*/
	VkCommandBuffer VulkanDevice::CreateCommandBuffer(VkCommandBufferLevel level, VkCommandPool pool, bool begin, VkCommandBufferUsageFlags usage)
	{
		MutexLockGuard lock(m_mutex);

		VkCommandBufferAllocateInfo cmdBufAllocateInfo = vks::initializers::commandBufferAllocateInfo(pool, level, 1);
		VkCommandBuffer cmdBuffer;
		ThrowIfFailed(vkAllocateCommandBuffers(m_logicalDevice, &cmdBufAllocateInfo, &cmdBuffer));
		// If requested, also start recording for the new command buffer
		if (begin)
		{
			VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();
			cmdBufInfo.flags = usage;
			ThrowIfFailed(vkBeginCommandBuffer(cmdBuffer, &cmdBufInfo));
		}
		return cmdBuffer;
	}

	VkCommandBuffer VulkanDevice::CreateCommandBuffer(VkCommandBufferLevel level, bool begin)
	{
		return CreateCommandBuffer(level, m_commandPool, begin);
	}

	VkCommandBuffer VulkanDevice::CreateSingleUseBuffer(VkCommandBufferLevel level, bool begin)
	{
		return CreateCommandBuffer(level, m_commandPool, begin, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	}

	/**
	* Finish command buffer recording and submit it to a queue
	*
	* @param commandBuffer Command buffer to flush
	* @param queue Queue to submit the command buffer to
	* @param pool Command pool on which the command buffer has been created
	* @param free (Optional) Free the command buffer once it has been submitted (Defaults to true)
	*
	* @note The queue that the command buffer is submitted to must be from the same family index as the pool it was allocated from
	* @note Uses a fence to ensure command buffer has finished executing
	*/
	void VulkanDevice::FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool pool, bool free)
	{
		MutexLockGuard lock(m_mutex);

		if (commandBuffer == VK_NULL_HANDLE)
		{
			return;
		}

		ThrowIfFailed(vkEndCommandBuffer(commandBuffer));

		VkSubmitInfo submitInfo = vks::initializers::submitInfo();
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		// Create fence to ensure that the command buffer has finished executing
		VkFenceCreateInfo fenceInfo = vks::initializers::fenceCreateInfo(VK_FLAGS_NONE);
		VkFence fence;
		ThrowIfFailed(vkCreateFence(m_logicalDevice, &fenceInfo, nullptr, &fence));
		// Submit to the queue
		ThrowIfFailed(vkQueueSubmit(queue, 1, &submitInfo, fence));
		// Wait for the fence to signal that command buffer has finished executing
		ThrowIfFailed(vkWaitForFences(m_logicalDevice, 1, &fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT));
		vkDestroyFence(m_logicalDevice, fence, nullptr);
		if (free)
		{
			vkFreeCommandBuffers(m_logicalDevice, pool, 1, &commandBuffer);
		}
	}

	void VulkanDevice::FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free)
	{
		return FlushCommandBuffer(commandBuffer, queue, m_commandPool, true);
	}

	/**
	* Check if an extension is supported by the (physical device)
	*
	* @param extension Name of the extension to check
	*
	* @return True if the extension is supported (present in the list read at device creation time)
	*/
	bool VulkanDevice::ExtensionSupported(std::string extension)
	{
		return (std::find(m_supportedExtensions.begin(), m_supportedExtensions.end(), extension) != m_supportedExtensions.end());
	}

	/**
	* Select the best-fit depth format for this device from a list of possible depth (and stencil) formats
	*
	* @param checkSamplingSupport Check if the format can be sampled from (e.g. for shader reads)
	*
	* @return The depth format that best fits for the current device
	*
	* @throw Throws an exception if no depth format fits the requirements
	*/
	VkFormat VulkanDevice::GetSupportedDepthFormat(bool checkSamplingSupport)
	{
		// All depth formats may be optional, so we need to find a suitable depth format to use
		std::vector<VkFormat> depthFormats = { VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D16_UNORM };
		for (auto& format : depthFormats)
		{
			VkFormatProperties formatProperties;
			vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &formatProperties);
			// Format must support depth stencil attachment for optimal tiling
			if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
			{
				if (checkSamplingSupport)
				{
					if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT))
					{
						continue;
					}
				}
				return format;
			}
		}
		throw std::runtime_error("Could not find a matching depth format");
	}

	VkQueue VulkanDevice::GetQueue(VkQueueFlags queue)
	{
		switch (queue)
		{
			case VK_QUEUE_GRAPHICS_BIT: return m_queueFamily.graphics;
			case VK_QUEUE_COMPUTE_BIT: return m_queueFamily.compute;
			case VK_QUEUE_TRANSFER_BIT: return m_queueFamily.transfer;
		}
		return m_queueFamily.graphics;
	}
	
	/*
	* Check if we have any commands which need the GPU to be idle.
	* If we do then wait and execute the commands.
	* TODO: This might be able to change to wait for the command buffer the 
	*/
	void VulkanDevice::CheckIdleQueue()
	{
		if (m_waitIdleCommands.size() > 0)
		{
			WaitForIdle();
			for (auto& func : m_waitIdleCommands)
			{
				func();
			}

			m_waitIdleCommands.clear();
		}
	}

	/*
	* Queue a new GPU command which needs the device to be idle.
	* TODO: Look at doing in a more effective way. Try and reduce this as much as possible.
	*/
	void VulkanDevice::QueueIdleCommand(const std::function<void()>& funcPtr)
	{
		if (m_enableWaitCommands)
		{
			IS_PROFILE_CATEGORY("QueueIdleCommand", Insight::Category::Type::GPU_CRITICAL);
			m_waitIdleCommands.push_back(funcPtr);
		}
		else
		{
			funcPtr();
		}
	}

	void VulkanDevice::EnableIdleCommands()
	{
		if (!m_enableWaitCommands)
		{
			m_enableWaitCommands = true;
		}
	}

	VkResult VulkanDevice::WaitForIdle()
	{
		MutexLockGuard lock(m_mutex);

		return vkDeviceWaitIdle(m_logicalDevice);
	}

	void VulkanDevice::CreateRenderpass()
	{

	}

	void VulkanDevice::CreateDescriptorPool()
	{
		MutexLockGuard lock(m_mutex);

		VkDescriptorPoolSize pool_sizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};

		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1000 * ARRAY_SIZEOF(pool_sizes);
		pool_info.poolSizeCount = (uint32_t)ARRAY_SIZEOF(pool_sizes);
		pool_info.pPoolSizes = pool_sizes;
		ThrowIfFailed(vkCreateDescriptorPool(m_logicalDevice, &pool_info, nullptr, &m_descriptorPool));
	}
}