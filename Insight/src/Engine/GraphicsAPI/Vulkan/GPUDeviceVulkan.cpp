#include "ispch.h"
#include "GPUDeviceVulkan.h"
#include "VulkanInitializers.h"
#include "VulkanUtils.h"
#include "Engine/Config/Config.h"
#include "GLFW/glfw3.h"
#include "VulkanPlatform.h"
#include "Engine/Threading/Threading.h"

#include "GPUAdapterVulkan.h"
#include "QueueVulkan.h"
#include "CmdBufferVulkan.h"
#include "UniformBufferUploaderVulkan.h"

#include "Engine/Graphics/GPUBufferDescription.h"
#include "Engine/Graphics/Textures/GPUTextureDescription.h"

#include "Engine/Core/Maths/Color32.h"
#include "Engine/Core/Application.h"
#include "Engine/Core/Collections/HashFunctions.h"

GPUDeviceVulkan::OptionalVulkanDeviceExtensions GPUDeviceVulkan::OptionalDeviceExtensions;

GPUDevice* GPUDeviceVulkan::New()
{
	GPUDevice* device = ::New<GPUDeviceVulkan>();
	if (!device->Init())
	{
		::Delete(device);
		IS_FATEL("[GPUDeviceVulkan::New] Device could not be created.");
		return nullptr;
	}
	return device;
}

GPUDeviceVulkan::GPUDeviceVulkan()
	: GPUDevice(RendererType::Vulkan, ShaderProfile::Vulkan_SM5)
	, DeferredDeletionQueue(this)
{
}

GPUDeviceVulkan::~GPUDeviceVulkan()
{
}

GPUContext* GPUDeviceVulkan::GetMainContext()
{
	return nullptr;
}

GPUAdapter* GPUDeviceVulkan::GetAdapter()
{
	return m_adapter;
}

static I32 GetMaxSampleCount(VkSampleCountFlags counts)
{
	if (counts & VK_SAMPLE_COUNT_64_BIT)
	{
		return VK_SAMPLE_COUNT_64_BIT;
	}
	if (counts & VK_SAMPLE_COUNT_32_BIT)
	{
		return VK_SAMPLE_COUNT_32_BIT;
	}
	if (counts & VK_SAMPLE_COUNT_16_BIT)
	{
		return VK_SAMPLE_COUNT_16_BIT;
	}
	if (counts & VK_SAMPLE_COUNT_8_BIT)
	{
		return VK_SAMPLE_COUNT_8_BIT;
	}
	if (counts & VK_SAMPLE_COUNT_4_BIT)
	{
		return VK_SAMPLE_COUNT_4_BIT;
	}
	if (counts & VK_SAMPLE_COUNT_2_BIT)
	{
		return VK_SAMPLE_COUNT_2_BIT;
	}
	return VK_SAMPLE_COUNT_1_BIT;
}

bool GPUDeviceVulkan::Init()
{
	// Get all the extensions we need/want for vulkan.
	std::vector<const char*> instanceExtensions, layerExtentions;
	GetInstanceExtensions(instanceExtensions, layerExtentions);

	// Create a vulkan instance.
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Debug";
	appInfo.pEngineName = "Insight";
	appInfo.apiVersion = VK_MAKE_VERSION(INSIGHT_MAJOR, INSIGHT_MINOR, INSIGHT_PATCH);

	VkInstanceCreateInfo instanceCreateInfo{};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pNext = NULL;
	instanceCreateInfo.pApplicationInfo = &appInfo;
	instanceCreateInfo.enabledExtensionCount = static_cast<U32>(instanceExtensions.size());
	instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
	instanceCreateInfo.ppEnabledLayerNames = layerExtentions.data();
	instanceCreateInfo.enabledLayerCount = 1;
	ThrowIfFailed(vkCreateInstance(&instanceCreateInfo, nullptr, &m_instance));

	//#ifdef IS_DEBUG
		// Setup debugging reports. 
	if ((bool)CONFIG_VAL(Config::GraphicsConfig.Validation))
	{
		// The report flags determine what type of messages for the layers will be displayed
		// For validating (debugging) an application the error and warning bits should sufficeVK_EXT_debug_report
		VkDebugReportFlagsEXT debugReportFlags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
		// Additional flags include performance info, loader and layer debug messages, etc.
		Debug::SetupDebugging(m_instance, debugReportFlags, VK_NULL_HANDLE);
	}
	//#endif

		// Get physical device
	U32 gpuCount = 0;
	ThrowIfFailed(vkEnumeratePhysicalDevices(m_instance, &gpuCount, nullptr));
	ASSERT(gpuCount > 0);
	std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
	ThrowIfFailed(vkEnumeratePhysicalDevices(m_instance, &gpuCount, physicalDevices.data()));
	std::vector<GPUAdapterVulkan> adapters;
	U32 gpuIndex = 0;
	for (auto& gpu : physicalDevices)
	{
		GPUAdapterVulkan adapter(gpu);
		adapters.push_back(adapter);
		IS_CORE_INFO("Device: {0}, '{1}' 	API: {2}.{3}.{4} 	Driver: {5}.{6}.{7}", gpuIndex++, adapter.Description,
			VK_VERSION_MAJOR(adapter.GpuProps.apiVersion),
			VK_VERSION_MINOR(adapter.GpuProps.apiVersion),
			VK_VERSION_PATCH(adapter.GpuProps.apiVersion),
			VK_VERSION_MAJOR(adapter.GpuProps.driverVersion),
			VK_VERSION_MINOR(adapter.GpuProps.driverVersion),
			VK_VERSION_PATCH(adapter.GpuProps.driverVersion));
		IS_CORE_INFO("	VendorId: 0x{0}, Type: {1}, Max Descriptor Sets Bound: {2}, Timestamps: {3}", adapter.GpuProps.vendorID, PhysicalDeviceTypeString(adapter.GpuProps.deviceType), adapter.GpuProps.limits.maxBoundDescriptorSets, !!adapter.GpuProps.limits.timestampComputeAndGraphics);
	}

	// New adapter.
	U32 selectedGpu = 0;
	for (auto& gpu : adapters)
	{
		if (gpu.IsNVIDIA() || gpu.IsAMD())
		{
			break;
		}
		++selectedGpu;
	}
	m_adapter = ::New<GPUAdapterVulkan>(adapters[selectedGpu]);
	ASSERT(m_adapter);

	m_totalGraphicsMemory = 0;
	m_state = DeviceState::Created;

	const auto gpu = m_adapter->Gpu;

	//Query queues
	// Get queue family indices for the requested queue family types
	// Note that the indices may overlap depending on the implementation
	const float defaultQueuePriority(0.0f);
	U32 queueCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueCount, nullptr);
	ASSERT(queueCount >= 1);
	m_queueFamilyProps.resize(queueCount);
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueCount, m_queueFamilyProps.data());

	// Query the device features.
	vkGetPhysicalDeviceFeatures(gpu, &m_physicalDeviceFeatures);

	// Setup device extensions and layers.
	std::vector<const char*> deviceExtensions;
	std::vector<const char*> validationLayers;
	GetDeviceExtensionsAndLayers(gpu, deviceExtensions, validationLayers);

	VkDeviceCreateInfo deviceInfo{};
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.enabledExtensionCount = static_cast<U32>(deviceExtensions.size());
	deviceInfo.ppEnabledExtensionNames = deviceExtensions.data();
	deviceInfo.enabledLayerCount = static_cast<U32>(validationLayers.size());
	deviceInfo.ppEnabledLayerNames = deviceInfo.enabledLayerCount > 0 ? validationLayers.data() : nullptr;

	// Setup queues info
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
	I32 graphicsQueueIndex = -1;
	I32 computeQueueIndex = -1;
	I32 transferQueueIndex = -1;
	IS_INFO("Found {0} queue families:", m_queueFamilyProps.size());
	U32 numPriorities = 0;
	for (I32 i = 0; i < m_queueFamilyProps.size(); ++i)
	{
		const VkQueueFamilyProperties& curProps = m_queueFamilyProps[i];

		bool isValidQueue = false;
		if ((curProps.queueFlags & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT)
		{
			if (graphicsQueueIndex == -1)
			{
				graphicsQueueIndex = i;
				isValidQueue = true;
			}
			else
			{
				// TODO: Support for multi-queue and choose the best queue
			}
		}

		if ((curProps.queueFlags & VK_QUEUE_COMPUTE_BIT) == VK_QUEUE_COMPUTE_BIT)
		{
			if (computeQueueIndex == -1 && graphicsQueueIndex != i)
			{
				computeQueueIndex = i;
				isValidQueue = true;
			}
		}

		if ((curProps.queueFlags & VK_QUEUE_TRANSFER_BIT) == VK_QUEUE_TRANSFER_BIT)
		{
			// Favor a non-gfx transfer queue
			if (transferQueueIndex == -1 && (curProps.queueFlags & VK_QUEUE_GRAPHICS_BIT) != VK_QUEUE_GRAPHICS_BIT && (curProps.queueFlags & VK_QUEUE_COMPUTE_BIT) != VK_QUEUE_COMPUTE_BIT)
			{
				transferQueueIndex = i;
				isValidQueue = true;
			}
		}

		std::string queueTypeInfo;
		if ((curProps.queueFlags & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT)
			queueTypeInfo += " graphics";
		if ((curProps.queueFlags & VK_QUEUE_COMPUTE_BIT) == VK_QUEUE_COMPUTE_BIT)
			queueTypeInfo += " compute";
		if ((curProps.queueFlags & VK_QUEUE_TRANSFER_BIT) == VK_QUEUE_TRANSFER_BIT)
			queueTypeInfo += " transfer";
		if ((curProps.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) == VK_QUEUE_SPARSE_BINDING_BIT)
			queueTypeInfo += " sparse";

		if (!isValidQueue)
		{
			IS_INFO("Skipping unnecessary queue family {0}: {1} queues{2}", i, curProps.queueCount, queueTypeInfo);
			continue;
		}

		const I32 queueIndex = (I32)queueCreateInfos.size();
		queueCreateInfos.push_back(VkDeviceQueueCreateInfo{});
		VkDeviceQueueCreateInfo& curQueue = queueCreateInfos[queueIndex];
		curQueue.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		curQueue.queueFamilyIndex = i;
		curQueue.queueCount = curProps.queueCount;
		numPriorities += curProps.queueCount;
		IS_INFO("- queue family {0}: {1} queues{2}", i, curProps.queueCount, queueTypeInfo);
	}
	std::vector<float> queuePriorities;
	queuePriorities.resize(numPriorities);
	float* currentPriority = queuePriorities.data();
	for (I32 index = 0; index < queueCreateInfos.size(); ++index)
	{
		VkDeviceQueueCreateInfo& queue = queueCreateInfos[index];
		queue.pQueuePriorities = currentPriority;
		const VkQueueFamilyProperties& properties = m_queueFamilyProps[queue.queueFamilyIndex];
		for (I32 queueIndex = 0; queueIndex < (I32)properties.queueCount; queueIndex++)
		{
			*currentPriority++ = 1.0f;
		}
	}
	deviceInfo.queueCreateInfoCount = static_cast<U32>(queueCreateInfos.size());
	deviceInfo.pQueueCreateInfos = queueCreateInfos.data();

	VkPhysicalDeviceFeatures enabledFeatures;
	VulkanPlatform::EnabledPhysicalDeviceFeatures(m_physicalDeviceFeatures, enabledFeatures);
	deviceInfo.pEnabledFeatures = &enabledFeatures;
	ThrowIfFailed(vkCreateDevice(gpu, &deviceInfo, nullptr, &Device));
	ASSERT(Device != VK_NULL_HANDLE);

	// Create queues
	if (graphicsQueueIndex == -1)
	{
		IS_ERROR("Missing Vulkan graphics queue.");
		return false;
	}
	GraphicsQueue = ::New<QueueVulkan>(this, graphicsQueueIndex);
	ComputeQueue = computeQueueIndex != -1 ? ::New<QueueVulkan>(this, computeQueueIndex) : GraphicsQueue;
	TransferQueue = transferQueueIndex != -1 ? ::New<QueueVulkan>(this, transferQueueIndex) : GraphicsQueue;

	// Init device limits 
	PhysicalDeviceLimits = m_adapter->GpuProps.limits;
	MSAALevel maxMssa = MSAALevel::None;
	if (m_physicalDeviceFeatures.sampleRateShading)
	{
		const I32 framebufferColorSampleCount = GetMaxSampleCount(PhysicalDeviceLimits.framebufferColorSampleCounts);
		const I32 framebufferDepthSampleCount = GetMaxSampleCount(PhysicalDeviceLimits.framebufferDepthSampleCounts);
		maxMssa = (MSAALevel)std::clamp(std::min<I32>(framebufferColorSampleCount, framebufferDepthSampleCount), 1, 8);
	}

	auto& limits = m_gpuLimits;
	limits.HasCompute = GetShaderProfile() == ShaderProfile::Vulkan_SM5 && PhysicalDeviceLimits.maxComputeWorkGroupCount[0] >= GPU_MAX_CS_DISPATCH_THREAD_GROUPS && PhysicalDeviceLimits.maxComputeWorkGroupCount[1] >= GPU_MAX_CS_DISPATCH_THREAD_GROUPS;
	//limits.HasTessellation = !!m_physicalDeviceFeatures.tessellationShader && PhysicalDeviceLimits.maxBoundDescriptorSets > (uint32_t)DescriptorSet::Domain;
	limits.HasGeometryShaders = false; // TODO: add geometry shaders support for Vulkan
	limits.HasInstancing = true;
	limits.HasVolumeTextureRendering = true;
	limits.HasDrawIndirect = false; // TODO: add Draw Indirect support for Vulkan
	limits.HasAppendConsumeBuffers = false; // TODO: add Append Consume buffers support for Vulkan
	limits.HasSeparateRenderTargetBlendState = true;
	limits.HasDepthAsSRV = true;
	limits.HasReadOnlyDepth = true;
	limits.HasMultisampleDepthAsSRV = !!m_physicalDeviceFeatures.sampleRateShading;
	limits.MaximumMipLevelsCount = std::min(static_cast<I32>(log2(PhysicalDeviceLimits.maxImageDimension2D)), GPU_MAX_TEXTURE_MIP_LEVELS);
	limits.MaximumTexture1DSize = PhysicalDeviceLimits.maxImageDimension1D;
	limits.MaximumTexture1DArraySize = PhysicalDeviceLimits.maxImageArrayLayers;
	limits.MaximumTexture2DSize = PhysicalDeviceLimits.maxImageDimension2D;
	limits.MaximumTexture2DArraySize = PhysicalDeviceLimits.maxImageArrayLayers;
	limits.MaximumTexture3DSize = PhysicalDeviceLimits.maxImageDimension3D;
	limits.MaximumTextureCubeSize = PhysicalDeviceLimits.maxImageDimensionCube;

	for (I32 i = 0; i < static_cast<I32>(PixelFormat::MAX); i++)
	{
		const auto format = static_cast<PixelFormat>(i);
		const auto vkFormat = ToVulkanFormat(format);

		MSAALevel msaa = MSAALevel::None;
		FormatSupport support = FormatSupport::None;

		if (vkFormat != VK_FORMAT_UNDEFINED)
		{
			VkFormatProperties properties;
			Platform::MemClear(&properties, sizeof(properties));
			vkGetPhysicalDeviceFormatProperties(gpu, vkFormat, &properties);

			// Query image format features support flags
#define CHECK_IMAGE_FORMAT(bit, feature) if (((properties.linearTilingFeatures & bit) == bit) || ((properties.optimalTilingFeatures & bit) == bit)) support |= feature
			if (properties.linearTilingFeatures != 0 || properties.optimalTilingFeatures != 0)
				support |= FormatSupport::Texture1D | FormatSupport::Texture2D | FormatSupport::Texture3D | FormatSupport::TextureCube;
			CHECK_IMAGE_FORMAT(VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT, FormatSupport::ShaderLoad);
			//VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT,
			//VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT
			CHECK_IMAGE_FORMAT(VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT, FormatSupport::RenderTarget);
			CHECK_IMAGE_FORMAT(VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT, FormatSupport::Blendable);
			CHECK_IMAGE_FORMAT(VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, FormatSupport::DepthStencil);
			//VK_FORMAT_FEATURE_BLIT_SRC_BIT
			//VK_FORMAT_FEATURE_BLIT_DST_BIT
			CHECK_IMAGE_FORMAT(VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT, FormatSupport::ShaderSample | FormatSupport::ShaderSampleComparison);
#undef CHECK_IMAGE_FORMAT

			// Query buffer format features support flags
#define CHECK_BUFFER_FORMAT(bit, feature) if ((properties.bufferFeatures & bit) == bit) support |= feature
			if (properties.bufferFeatures != 0)
				support |= FormatSupport::Buffer;
			//VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT
			//VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT
			//VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT
			CHECK_BUFFER_FORMAT(VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT, FormatSupport::InputAssemblyVertexBuffer);
#undef CHECK_BUFFER_FORMAT

			// Unused bits
			//VK_FORMAT_FEATURE_TRANSFER_SRC_BIT
			//VK_FORMAT_FEATURE_TRANSFER_DST_BIT
			//VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT
			//VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT
			//VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT
			//VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_BIT
			//VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT
			//VK_FORMAT_FEATURE_DISJOINT_BIT
			//VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT
			//VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT_IMG
			//VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_MINMAX_BIT_EXT

			// Multi-sampling support
			if (support & FormatSupport::Texture2D)
				msaa = maxMssa;
		}

		m_featuresPerFormat[i] = FormatFeatures(format, msaa, support);
	}


	// Setup memory limit and print memory info
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(gpu, &memoryProperties);
	IS_INFO("Max memory allocations: {0}", m_adapter->GpuProps.limits.maxMemoryAllocationCount);
	IS_INFO("Found {0} device memory heaps:", memoryProperties.memoryHeapCount);
	for (U32 i = 0; i < memoryProperties.memoryHeapCount; ++i)
	{
		const VkMemoryHeap& heap = memoryProperties.memoryHeaps[i];
		bool isGPUHeap = (heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) == VK_MEMORY_HEAP_DEVICE_LOCAL_BIT;
		IS_INFO("-  memory heap {0}: flags 0x{1:x}, size {2} MB (GPU: {3})", i, heap.flags, (U32)(heap.size / 1024 / 1024), isGPUHeap);
		if (isGPUHeap)
			m_totalGraphicsMemory += heap.size;
	}
	IS_INFO("Found {0} device memory types:", memoryProperties.memoryTypeCount);
	for (U32 i = 0; i < memoryProperties.memoryTypeCount; ++i)
	{
		const VkMemoryType& type = memoryProperties.memoryTypes[i];
		std::string flagsInfo;
		if ((type.propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			flagsInfo += "local, ";
		if ((type.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
			flagsInfo += "host visible, ";
		if ((type.propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
			flagsInfo += "host coherent, ";
		if ((type.propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) == VK_MEMORY_PROPERTY_HOST_CACHED_BIT)
			flagsInfo += "host cached, ";
		if ((type.propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) == VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT)
			flagsInfo += "lazy, ";
		if (!flagsInfo.empty())
			flagsInfo = ", properties: " + flagsInfo.substr(0, flagsInfo.size() - 2);
		IS_INFO("-  memory type {0}: flags 0x{1:x}, heap {2}{3}", i, type.propertyFlags, type.heapIndex, flagsInfo);
	}

	// Initialise vulkan memory allocator
	VmaAllocatorCreateInfo allocatorInfo{};
	allocatorInfo.instance = m_instance;
	allocatorInfo.physicalDevice = gpu;
	allocatorInfo.device = Device;

#ifdef IS_DEBUG
	VmaRecordSettings vmaRecordSettings{};
	vmaRecordSettings.pFilePath = "vma.csv";
	allocatorInfo.pRecordSettings = &vmaRecordSettings;
#endif

	ThrowIfFailed(vmaCreateAllocator(&allocatorInfo, &VmaAllocator));

	//Prepare other things for the engine to use in relation to vulkan.
	FenceManager.Init(this);
	UniformBufferUploader = ::New<UniformBufferUploaderVulkan>(this);
	//DescriptorPoolsManager = New<DescriptorPoolsManagerVulkan>(this);
	//MainContext = New<GPUContextVulkan>(this, GraphicsQueue);

	return true;
}

bool GPUDeviceVulkan::LoadContent()
{
	return false;
}

void GPUDeviceVulkan::Dispose()
{
	GPUDeviceLock lock(this);

	if (m_state == DeviceState::Disposed)
	{
		return;
	}

	m_state = DeviceState::Disposing;

	WaitForGPU();

	Resources.OnDeviceDestroy();

	SAFE_DELETE(UniformBufferUploader);
	SAFE_DELETE(GraphicsQueue);
	SAFE_DELETE(ComputeQueue);
	SAFE_DELETE(TransferQueue);
	FenceManager.Dispose();
	DeferredDeletionQueue.ReleaseResources(true);

	vmaDestroyAllocator(VmaAllocator);
	VmaAllocator = VK_NULL_HANDLE;

	SAFE_DELETE(m_adapter);

#ifdef IS_DEBUG
	if ((bool)CONFIG_VAL(Config::GraphicsConfig.Validation))
	{
		Debug::FreeDebugCallback(m_instance);
	}
#endif

	vkDestroyDevice(Device, nullptr);
	vkDestroyInstance(m_instance, nullptr);

	GPUDevice::Dispose();
	m_state = DeviceState::Disposed;
}

void GPUDeviceVulkan::WaitForGPU()
{
}

GPUBuffer* GPUDeviceVulkan::NewBuffer(const std::string& name)
{
	return nullptr;
}

GPUTexture* GPUDeviceVulkan::NewTexture(const std::string& name)
{
	return nullptr;
}

SemaphoreVulkan::SemaphoreVulkan(GPUDeviceVulkan* device)
	: _device(device)
{
	// Create semaphore
	VkSemaphoreCreateInfo info = vks::initializers::semaphoreCreateInfo();
	ThrowIfFailed(vkCreateSemaphore(device->Device, &info, nullptr, &_semaphoreHandle));
}

SemaphoreVulkan::~SemaphoreVulkan()
{
	ASSERT(_semaphoreHandle != VK_NULL_HANDLE);
	_device->DeferredDeletionQueue.EnqueueResource(DeferredDeletionQueueVulkan::Semaphore, _semaphoreHandle);
	_semaphoreHandle = VK_NULL_HANDLE;
}

FenceVulkan::FenceVulkan(GPUDeviceVulkan* device, FenceManagerVulkan* owner, bool createSignaled)
	: m_signaled(createSignaled)
	, m_owner(owner)
{
	VkFenceCreateInfo info = vks::initializers::fenceCreateInfo();
	info.flags = createSignaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;
	ThrowIfFailed(vkCreateFence(device->Device, &info, nullptr, &m_handle));

}

FenceVulkan::~FenceVulkan()
{
	ASSERT(m_handle == VK_NULL_HANDLE);
}

FenceManagerVulkan::~FenceManagerVulkan()
{
	ASSERT(m_usedFences.empty())
}

void FenceManagerVulkan::Dispose()
{
	ScopeLock lock(m_device->m_fenceLock);

	ASSERT(m_usedFences.empty());
	for (FenceVulkan* fence : m_freeFences)
	{
		DestroyFence(fence);
	}
	m_freeFences.clear();
}

FenceVulkan* FenceManagerVulkan::AllocateFence(bool createSignaled)
{
	ScopeLock lock(m_device->m_fenceLock);

	FenceVulkan* fence;
	if (!m_freeFences.empty())
	{
		fence = m_freeFences.back();
		m_freeFences.pop_back();
		m_usedFences.push_back(fence);

		if (createSignaled)
		{
			fence->m_signaled = true;
		}

		return fence;
	}

	fence = New<FenceVulkan>(m_device, this, createSignaled);
	m_usedFences.push_back(fence);
	return fence;
}

bool FenceManagerVulkan::WaitForFence(FenceVulkan* fence, U64 timeInNanoseconds)
{
	//ASSERT(VectorContains(m_usedFences, fence));
	ASSERT(!fence->m_signaled);

	VkResult result =  vkWaitForFences(m_device->Device, 1, &fence->m_handle, true, timeInNanoseconds);
	ThrowIfFailed(result);
	if (result == VK_SUCCESS)
	{
		fence->m_signaled = true;
		return true;
	}

	return false;
}

void FenceManagerVulkan::ResetFence(FenceVulkan* fence)
{
	if (fence->m_signaled)
	{
		ThrowIfFailed(vkResetFences(m_device->Device, 1, &fence->m_handle));
		fence->m_signaled = false;
	}
}

void FenceManagerVulkan::ReleaseFence(FenceVulkan*& fence)
{
	ScopeLock lock(m_device->m_fenceLock);

	ResetFence(fence);
	m_usedFences.erase(m_usedFences.begin() + VectorFindIndex(m_usedFences, fence));
	m_freeFences.push_back(fence);
	fence = nullptr;
}

void FenceManagerVulkan::WaitAndReleaseFence(FenceVulkan*& fence, U64 timeInNanoseconds)
{
	ScopeLock lock(m_device->m_fenceLock);

	if (!fence->IsSignaled())
	{
		WaitForFence(fence, timeInNanoseconds);
	}

	ResetFence(fence);
	m_usedFences.erase(m_usedFences.begin() + VectorFindIndex(m_usedFences, fence));
	m_freeFences.push_back(fence);
	fence = nullptr;
}

bool FenceManagerVulkan::CheckFenceState(FenceVulkan* fence)
{
	ASSERT(VectorContains(m_usedFences, fence));
	ASSERT(!fence->m_signaled);

	const VkResult result = vkGetFenceStatus(m_device->Device, fence->GetHandle());
	if (result == VK_SUCCESS)
	{
		fence->m_signaled = true;
		return true;
	}

	return false;
}

void FenceManagerVulkan::DestroyFence(FenceVulkan* fence)
{
	vkDestroyFence(m_device->Device, fence->GetHandle(), nullptr);
	fence->m_handle = VK_NULL_HANDLE;
	::Delete(fence);
}

DeferredDeletionQueueVulkan::DeferredDeletionQueueVulkan(GPUDeviceVulkan* device)
	: _device(device)
{
}

DeferredDeletionQueueVulkan::~DeferredDeletionQueueVulkan()
{
	ASSERT(_entries.empty());
}

auto DeferredDeletionQueueVulkan::ReleaseResources(bool deleteImmediately) -> void
{
	ScopeLock lock(&_locker);
	const U64 checkFrame = Application::FrameCount - VULKAN_RESOURCE_DELETE_SAFE_FRAMES_COUNT;
	for (I32 i = 0; i < _entries.size(); i++)
	{
		Entry* e = &_entries[i];

		if (deleteImmediately || (checkFrame > e->FrameNumber && (e->CmdBuffer == nullptr || e->FenceCounter < e->CmdBuffer->GetFenceSignaledCounter()))
			)
		{
			if (e->AllocationHandle == VK_NULL_HANDLE)
			{
				switch (e->StructureType)
				{
#define SWITCH_CASE(type) case Type::type: vkDestroy##type(_device->Device, (Vk##type)e->Handle, nullptr); break
					SWITCH_CASE(RenderPass);
					SWITCH_CASE(Buffer);
					SWITCH_CASE(BufferView);
					SWITCH_CASE(Image);
					SWITCH_CASE(ImageView);
					SWITCH_CASE(Pipeline);
					SWITCH_CASE(PipelineLayout);
					SWITCH_CASE(Framebuffer);
					SWITCH_CASE(DescriptorSetLayout);
					SWITCH_CASE(Sampler);
					SWITCH_CASE(Semaphore);
					SWITCH_CASE(ShaderModule);
					SWITCH_CASE(Event);
					SWITCH_CASE(QueryPool);
#undef SWITCH_CASE
				default:
#if !IS_RELEASE
					ASSERT(false);
#endif
					break;
				}
			}
			else
			{
				if (e->StructureType == Image)
				{
					vmaDestroyImage(_device->VmaAllocator, (VkImage)e->Handle, e->AllocationHandle);
				}
				else if (e->StructureType == Buffer)
				{
					vmaDestroyBuffer(_device->VmaAllocator, (VkBuffer)e->Handle, e->AllocationHandle);
				}
				else
				{
					ASSERT(false);
				}
			}

			_entries.erase(_entries.begin() + (i--));

			if (_entries.empty())
				break;
		}
	}
}

void DeferredDeletionQueueVulkan::EnqueueGenericResource(Type type, U64 handle, VmaAllocation allocation)
{
	ASSERT(handle != 0);
	const auto queue = _device->GraphicsQueue;

	Entry entry;
	queue->GetLastSubmittedInfo(entry.CmdBuffer, entry.FenceCounter);
	entry.Handle = handle;
	entry.AllocationHandle = allocation;
	entry.StructureType = type;
	entry.FrameNumber = Application::FrameCount;

	ScopeLock lock(_locker);

#if IS_DEBUG
	auto it= std::find_if(_entries.begin(), _entries.end(), [handle](const Entry& e)
		{
			return e.Handle == handle;
		});
	ASSERT(it == _entries.end());
#endif
	_entries.push_back(entry);
}

U32 GetHash(const RenderTargetLayoutVulkan& key)
{
	U32 hash = (I32)key.MSAA * 11;
	CombineHash(hash, (U32)key.ReadDepth);
	CombineHash(hash, (U32)key.WriteDepth);
	CombineHash(hash, (U32)key.DepthFormat * 93473262);
	CombineHash(hash, key.RTsCount * 136);
	for (I32 i = 0; i < ARRAY_COUNT(key.RTVsFormats); i++)
	{
		CombineHash(hash, (U32)key.RTVsFormats[i]);
	}
	return hash;
}

U32 GetHash(const FramebufferVulkan::Key& key)
{
	U32 hash = (I32)(I64)key.RenderPass;
	CombineHash(hash, (U32)key.AttachmentCount * 136);
	for (I32 i = 0; i < ARRAY_COUNT(key.Attachments); i++)
	{
		CombineHash(hash, (U32)(I64)key.Attachments[i]);
	}
	return hash;
}

FramebufferVulkan::FramebufferVulkan(GPUDeviceVulkan* device, Key& key, VkExtent3D& extent, U32 layers)
	: _device(device)
	, _handle(VK_NULL_HANDLE)
	, Extent(extent)
	, Layers(layers)
{
	Platform::MemCopy(Attachments, key.Attachments, sizeof(Attachments));

	VkFramebufferCreateInfo createInfo = vks::initializers::framebufferCreateInfo();
	createInfo.renderPass = key.RenderPass->GetHandle();
	createInfo.attachmentCount = key.AttachmentCount;
	createInfo.pAttachments = key.Attachments;
	createInfo.width = extent.width;
	createInfo.height = extent.height;
	createInfo.layers = layers;
	ThrowIfFailed(vkCreateFramebuffer(device->Device, &createInfo, nullptr, &_handle));
}

FramebufferVulkan::~FramebufferVulkan()
{
	_device->DeferredDeletionQueue.EnqueueResource(DeferredDeletionQueueVulkan::Type::Framebuffer, _handle);
}

bool FramebufferVulkan::HasReference(VkImageView imageView) const
{
	for (I32 i = 0; i < ARRAY_COUNT(Attachments); ++i)
	{
		if (Attachments[i] == imageView)
			return true;
	}
	return false;
}

RenderPassVulkan::RenderPassVulkan(GPUDeviceVulkan* device, const RenderTargetLayoutVulkan& layout)
	: m_device(device)
	, m_handle(VK_NULL_HANDLE)
	, Layout(layout)
{
	const I32 colorAttachmentsCount = layout.RTsCount;
	const bool hasDepthStencilAttachment = layout.DepthFormat != PixelFormat::Unknown;
	const I32 attachmentsCount = colorAttachmentsCount + (hasDepthStencilAttachment ? 1 : 0);

	VkAttachmentReference colorReferences[GPU_MAX_RT_BINDED];
	VkAttachmentReference depthStencilReference;
	VkAttachmentDescription attachments[GPU_MAX_RT_BINDED + 1];

	VkSubpassDescription subpassDesc;
	Platform::MemClear(&subpassDesc, sizeof(subpassDesc));
	subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDesc.colorAttachmentCount = colorAttachmentsCount;
	subpassDesc.pColorAttachments = colorReferences;
	subpassDesc.pResolveAttachments = nullptr;
	for (I32 i = 0; i < colorAttachmentsCount; i++)
	{
		VkAttachmentDescription& attachment = attachments[i];
		attachment.flags = 0;
		attachment.format = ToVulkanFormat(layout.RTVsFormats[i]);
		attachment.samples = (VkSampleCountFlagBits)layout.MSAA;
		//attachment.loadOp = currentBlendDesc->BlendEnable ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_DONT_CARE; // TODO: Only if any destination blend?
		attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD; // TODO: only load when using blend mode
		attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		VkAttachmentReference& reference = colorReferences[i];
		reference.attachment = i;
		reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}
	if (hasDepthStencilAttachment)
	{
		VkImageLayout depthStencilLayout;
		if (layout.ReadDepth && !layout.WriteDepth)
			depthStencilLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
		else
			depthStencilLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		// Use last slot for depth stencil attachment
		VkAttachmentDescription& depthAttachment = attachments[colorAttachmentsCount];
		depthAttachment.flags = 0;
		depthAttachment.format = ToVulkanFormat(layout.DepthFormat);
		depthAttachment.samples = (VkSampleCountFlagBits)layout.MSAA;
		// TODO: fix those operations for load and store
		depthAttachment.loadOp = layout.ReadDepth || true ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.storeOp = layout.WriteDepth || true ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // TODO: Handle stencil
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = depthStencilLayout;
		depthAttachment.finalLayout = depthStencilLayout;
		depthStencilReference.attachment = colorAttachmentsCount;
		depthStencilReference.layout = depthStencilLayout;
		subpassDesc.pDepthStencilAttachment = &depthStencilReference;
	}

	VkRenderPassCreateInfo createInfo = vks::initializers::renderPassCreateInfo();
	createInfo.attachmentCount = attachmentsCount;
	createInfo.pAttachments = attachments;
	createInfo.subpassCount = 1;
	createInfo.pSubpasses = &subpassDesc;
	ThrowIfFailed(vkCreateRenderPass(device->Device, &createInfo, nullptr, &m_handle));
}

RenderPassVulkan::~RenderPassVulkan()
{
	m_device->DeferredDeletionQueue.EnqueueResource(DeferredDeletionQueueVulkan::Type::RenderPass, m_handle);
}

QueryPoolVulkan::QueryPoolVulkan(GPUDeviceVulkan* device, I32 capacity, VkQueryType type)
	: _device(device)
	, _handle(VK_NULL_HANDLE)
	, _count(0)
	, _capacity(capacity)
	, _type(type)
{
	VkQueryPoolCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
	createInfo.queryType = type;
	createInfo.queryCount = capacity;
	ThrowIfFailed(vkCreateQueryPool(device->Device, &createInfo, nullptr, &_handle));
#if VULKAN_RESET_QUERY_POOLS
	_resetRanges.Add(Range{ 0, static_cast<U32>(capacity) });
	device->QueriesToReset.Add(this);
#endif
}

QueryPoolVulkan::~QueryPoolVulkan()
{
#if VULKAN_RESET_QUERY_POOLS
	_device->QueriesToReset.Remove(this);
#endif
	_device->DeferredDeletionQueue.EnqueueResource(DeferredDeletionQueueVulkan::Type::QueryPool, _handle);
}

#if VULKAN_RESET_QUERY_POOLS
void QueryPoolVulkan::Reset(CmdBufferVulkan* cmdBuffer)
{
	for (auto& range : _resetRanges)
	{
		vkCmdResetQueryPool(cmdBuffer->GetHandle(), _handle, range.Start, range.Count);
	}
	_resetRanges.clear();
}
#endif

BufferedQueryPoolVulkan::BufferedQueryPoolVulkan(GPUDeviceVulkan* device, I32 capacity, VkQueryType type)
	: QueryPoolVulkan(device, capacity, type)
	, _lastBeginIndex(0)
{
	_queryOutput.resize(capacity);
	_usedQueryBits.reserve((capacity + 63) / 64);
	_startedQueryBits.reserve((capacity + 63) / 64);
	_readResultsBits.reserve((capacity + 63) / 64);
}

bool BufferedQueryPoolVulkan::AcquireQuery(U32& resultIndex)
{
	const U64 allUsedMask = (U64)-1;
	for (I32 wordIndex = _lastBeginIndex / 64; wordIndex < _usedQueryBits.size(); wordIndex++)
	{
		U64 beginQueryWord = _usedQueryBits[wordIndex];
		if (beginQueryWord != allUsedMask)
		{
			resultIndex = 0;
			while ((beginQueryWord & 1) == 1)
			{
				resultIndex++;
				beginQueryWord >>= 1;
			}
			resultIndex += wordIndex * 64;
			const U64 bit = (U64)1 << (U64)(resultIndex % 64);
			_usedQueryBits[wordIndex] = _usedQueryBits[wordIndex] | bit;
			_readResultsBits[wordIndex] &= ~bit;
			_lastBeginIndex = resultIndex + 1;
			return true;
		}
	}

	return false;
}

void BufferedQueryPoolVulkan::ReleaseQuery(U32 queryIndex)
{
	const U32 word = queryIndex / 64;
	const U64 bit = (U64)1 << (queryIndex % 64);
	_usedQueryBits[word] = _usedQueryBits[word] & ~bit;
	_readResultsBits[word] = _readResultsBits[word] & ~bit;
	if (queryIndex < (U32)_lastBeginIndex)
	{
		// Use the lowest word available
		const U64 allUsedMask = (U64)-1;
		const I32 lastQueryWord = _lastBeginIndex / 64;
		if (lastQueryWord < _usedQueryBits.size() && _usedQueryBits[lastQueryWord] == allUsedMask)
		{
			_lastBeginIndex = (U32)queryIndex;
		}
	}
}

void BufferedQueryPoolVulkan::MarkQueryAsStarted(U32 queryIndex)
{
	const U32 word = queryIndex / 64;
	const U64 bit = (U64)1 << (queryIndex % 64);
	_startedQueryBits[word] = _startedQueryBits[word] | bit;
}

bool BufferedQueryPoolVulkan::GetResults(GPUContextVulkan* context, U32 index, U64& result)
{
	const U64 bit = (U64)(index % 64);
	const U64 bitMask = (U64)1 << bit;
	const U32 word = index / 64;

	if ((_startedQueryBits[word] & bitMask) == 0)
	{
		// Query never started/ended
		result = 0;
		return true;
	}

	if ((_readResultsBits[word] & bitMask) == 0)
	{
		const VkResult vkResult = vkGetQueryPoolResults(_device->Device, _handle, index, 1, sizeof(U64), &_queryOutput[index], sizeof(U64), VK_QUERY_RESULT_64_BIT);
		if (vkResult == VK_SUCCESS)
		{
			_readResultsBits[word] = _readResultsBits[word] | bitMask;

#if VULKAN_RESET_QUERY_POOLS
			// Add to reset
			if (!_device->QueriesToReset.Contains(this))
				_device->QueriesToReset.Add(this);
			_resetRanges.Add(Range{ index, 1 });
#endif
		}
		else if (vkResult == VK_NOT_READY)
		{
			// No results yet
			result = 0;
			return false;
		}
		else
		{
			IS_CORE_INFO("{0}", VkErrorToString(vkResult));
		}
	}

	result = _queryOutput[index];
	return true;
}

bool BufferedQueryPoolVulkan::HasRoom() const
{
	const U64 allUsedMask = (U64)-1;
	if (_lastBeginIndex < _usedQueryBits.size() * 64)
	{
		ASSERT((_usedQueryBits[_lastBeginIndex / 64] & allUsedMask) != allUsedMask);
		return true;
	}
	return false;
}

HelperResourcesVulkan::HelperResourcesVulkan(GPUDeviceVulkan* device)
	: _device(device)
	, _dummyBuffer(nullptr)
	, _dummyVB(nullptr)
{
	Platform::MemClear(_dummyTextures, sizeof(_dummyTextures));
	Platform::MemClear(_staticSamplers, sizeof(_staticSamplers));
}

void InitSampler(VkSamplerCreateInfo& createInfo, bool supportsMirrorClampToEdge, GPUSamplerFilter filter, GPUSamplerAddressMode addressU, GPUSamplerAddressMode addressV, GPUSamplerAddressMode addressW, GPUSamplerCompareFunction compareFunction)
{
	createInfo.magFilter = ToVulkanMagFilterMode(filter);
	createInfo.minFilter = ToVulkanMinFilterMode(filter);
	createInfo.mipmapMode = ToVulkanMipFilterMode(filter);
	createInfo.addressModeU = ToVulkanWrapMode(addressU, supportsMirrorClampToEdge);
	createInfo.addressModeV = ToVulkanWrapMode(addressV, supportsMirrorClampToEdge);
	createInfo.addressModeW = ToVulkanWrapMode(addressW, supportsMirrorClampToEdge);
	createInfo.compareEnable = compareFunction != GPUSamplerCompareFunction::Never ? VK_TRUE : VK_FALSE;
	createInfo.compareOp = ToVulkanSamplerCompareFunction(compareFunction);
}

VkSampler HelperResourcesVulkan::GetStaticSampler(StaticSamplers type)
{
	if (!_staticSamplers[0])
	{
		const bool supportsMirrorClampToEdge = _device->OptionalDeviceExtensions.HasMirrorClampToEdge;

		VkSamplerCreateInfo createInfo = vks::initializers::samplerCreateInfo();
		createInfo.mipLodBias = 0.0f;
		createInfo.minLod = 0.0f;
		createInfo.maxLod = std::numeric_limits<float>().max();
		createInfo.maxAnisotropy = 1.0f;
		createInfo.anisotropyEnable = VK_FALSE;
		createInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;

		// Linear Clamp
		InitSampler(createInfo, supportsMirrorClampToEdge, GPUSamplerFilter::Trilinear, GPUSamplerAddressMode::Clamp, GPUSamplerAddressMode::Clamp, GPUSamplerAddressMode::Clamp, GPUSamplerCompareFunction::Never);
		ThrowIfFailed(vkCreateSampler(_device->Device, &createInfo, nullptr, &_staticSamplers[0]));

		// Point Clamp
		InitSampler(createInfo, supportsMirrorClampToEdge, GPUSamplerFilter::Point, GPUSamplerAddressMode::Clamp, GPUSamplerAddressMode::Clamp, GPUSamplerAddressMode::Clamp, GPUSamplerCompareFunction::Never);
		ThrowIfFailed(vkCreateSampler(_device->Device, &createInfo, nullptr, &_staticSamplers[1]));

		// Linear Wrap
		InitSampler(createInfo, supportsMirrorClampToEdge, GPUSamplerFilter::Trilinear, GPUSamplerAddressMode::Wrap, GPUSamplerAddressMode::Wrap, GPUSamplerAddressMode::Wrap, GPUSamplerCompareFunction::Never);
		ThrowIfFailed(vkCreateSampler(_device->Device, &createInfo, nullptr, &_staticSamplers[2]));

		// Point Wrap
		InitSampler(createInfo, supportsMirrorClampToEdge, GPUSamplerFilter::Point, GPUSamplerAddressMode::Wrap, GPUSamplerAddressMode::Wrap, GPUSamplerAddressMode::Wrap, GPUSamplerCompareFunction::Never);
		ThrowIfFailed(vkCreateSampler(_device->Device, &createInfo, nullptr, &_staticSamplers[3]));

		// Shadow
		InitSampler(createInfo, supportsMirrorClampToEdge, GPUSamplerFilter::Point, GPUSamplerAddressMode::Clamp, GPUSamplerAddressMode::Clamp, GPUSamplerAddressMode::Clamp, GPUSamplerCompareFunction::Less);
		ThrowIfFailed(vkCreateSampler(_device->Device, &createInfo, nullptr, &_staticSamplers[4]));

		// Shadow PCF
		InitSampler(createInfo, supportsMirrorClampToEdge, GPUSamplerFilter::Trilinear, GPUSamplerAddressMode::Clamp, GPUSamplerAddressMode::Clamp, GPUSamplerAddressMode::Clamp, GPUSamplerCompareFunction::Less);
		ThrowIfFailed(vkCreateSampler(_device->Device, &createInfo, nullptr, &_staticSamplers[5]));
	}

	return _staticSamplers[static_cast<I32>(type)];
}

GPUTextureVulkan* HelperResourcesVulkan::GetDummyTexture(SpirvShaderResourceType type)
{

	I32 index;
	switch (type)
	{
	case SpirvShaderResourceType::Texture1D:
		index = 0;
		break;
	case SpirvShaderResourceType::Texture2D:
		index = 1;
		break;
	case SpirvShaderResourceType::Texture3D:
		index = 2;
		break;
	case SpirvShaderResourceType::TextureCube:
		index = 3;
		break;
	case SpirvShaderResourceType::Texture1DArray:
		index = 4;
		break;
	case SpirvShaderResourceType::Texture2DArray:
		index = 5;
		break;
	default:
		ASSERT(false);
		return nullptr;
	}

	auto texture = _dummyTextures[index];
	if (!texture)
	{
		texture = (GPUTextureVulkan*)_device->NewTexture("DummyTexture");
		GPUTextureDescription desc;
		const PixelFormat format = PixelFormat::R8G8B8A8_UNorm;
		const GPUTextureFlags flags = GPUTextureFlags::ShaderResource | GPUTextureFlags::UnorderedAccess;
		switch (type)
		{
		case SpirvShaderResourceType::Texture1D:
			desc = GPUTextureDescription::New1D(1, 1, format, flags, 1);
			break;
		case SpirvShaderResourceType::Texture2D:
			desc = GPUTextureDescription::New2D(1, 1, format, flags);
			break;
		case SpirvShaderResourceType::Texture3D:
			desc = GPUTextureDescription::New3D(1, 1, 1, format, flags);
			break;
		case SpirvShaderResourceType::TextureCube:
			desc = GPUTextureDescription::NewCube(1, format, flags);
			break;
		case SpirvShaderResourceType::Texture1DArray:
			desc = GPUTextureDescription::New1D(1, 1, format, flags, 4);
			break;
		case SpirvShaderResourceType::Texture2DArray:
			desc = GPUTextureDescription::New2D(1, 1, format, flags, 4);
			break;
		default:;
		}
		//texture->Init(desc);
		//ASSERT(texture->View(0));
		_dummyTextures[index] = texture;
	}

	return texture;
}

GPUBufferVulkan* HelperResourcesVulkan::GetDummyBuffer()
{
	if (!_dummyBuffer)
	{
		_dummyBuffer = (GPUBufferVulkan*)_device->NewBuffer("DummyBuffer");
		//_dummyBuffer->Init(GPUBufferDescription::Buffer(sizeof(I32), GPUBufferFlags::ShaderResource | GPUBufferFlags::UnorderedAccess, PixelFormat::R32_SInt));
	}

	return _dummyBuffer;
}

GPUBufferVulkan* HelperResourcesVulkan::GetDummyVertexBuffer()
{
	if (!_dummyVB)
	{
		_dummyVB = (GPUBufferVulkan*)_device->NewBuffer("DummyVertexBuffer");
		//_dummyVB->Init(GPUBufferDescription::Vertex(sizeof(Color32), 1, &Color32::Transparent));
	}

	return _dummyVB;
}

void HelperResourcesVulkan::Dispose()
{
	for (I32 i = 0; i < ARRAY_COUNT(_dummyTextures); i++)
	{
		SAFE_DELETE(_dummyTextures[i]);
	}

	SAFE_DELETE(_dummyBuffer);
	SAFE_DELETE(_dummyVB);

	for (I32 i = 0; i < ARRAY_COUNT(_staticSamplers); ++i)
	{
		auto& sampler = _staticSamplers[i];
		if (sampler != VK_NULL_HANDLE)
		{
			_device->DeferredDeletionQueue.EnqueueResource(DeferredDeletionQueueVulkan::Type::Sampler, sampler);
			sampler = VK_NULL_HANDLE;
		}
	}
}

StagingManagerVulkan::StagingManagerVulkan(GPUDeviceVulkan* device)
	: _device(device)
{
}

GPUBuffer* StagingManagerVulkan::AcquireBuffer(U32 size, GPUResourceUsage usage)
{
	// Try reuse free buffer
	{
		ScopeLock lock(_locker);

		for (I32 i = 0; i < _freeBuffers.size(); i++)
		{
			auto& freeBuffer = _freeBuffers[i];
			//if (freeBuffer.Buffer->GetSize() == size && freeBuffer.Buffer->GetDescription().Usage == usage)
			{
				const auto buffer = freeBuffer.Buffer;
				_freeBuffers.erase(_freeBuffers.begin() + i);
				return buffer;
			}
		}
	}

	// Allocate new buffer
	auto buffer = _device->NewBuffer("Pooled Staging");
	//if (buffer->Init(GPUBufferDescription::Buffer(size, GPUBufferFlags::None, PixelFormat::Unknown, nullptr, 0, usage)))
	{
		IS_CORE_INFO("Failed to create pooled staging buffer.");
		return nullptr;
	}

	// Cache buffer
	{
		ScopeLock lock(_locker);

		_allBuffers.push_back(buffer);
#if !IS_RELEASE
		_allBuffersAllocSize += size;
		_allBuffersTotalSize += size;
		_allBuffersPeekSize = std::max(_allBuffersTotalSize, _allBuffersPeekSize);
#endif
	}

	return buffer;
}

void StagingManagerVulkan::ReleaseBuffer(CmdBufferVulkan* cmdBuffer, GPUBuffer*& buffer)
{
	ScopeLock lock(_locker);

	if (cmdBuffer)
	{
		// Return to pending pool (need to wait until command buffer will be executed and buffer will be reusable)
		auto items = FindOrAdd(cmdBuffer);
		//auto item = items->FindOrAdd(cmdBuffer->GetFenceSignaledCounter());
		//item->Resources.Add(buffer);
	}
	else
	{
		// Return to pool
		_freeBuffers.push_back({ buffer, Application::FrameCount });
	}

	// Clear reference
	buffer = nullptr;
}

void StagingManagerVulkan::ProcessPendingFree()
{
	ScopeLock lock(_locker);

	// Find staging buffers that has been processed by the GPU and can be reused
	for (I32 i = _pendingBuffers.size() - 1; i >= 0; i--)
	{
		auto& e = _pendingBuffers[i];

		for (I32 fenceIndex = e.Items.size() - 1; fenceIndex >= 0; fenceIndex--)
		{
			auto& items = e.Items[fenceIndex];
			if (items.FenceCounter < e.CmdBuffer->GetFenceSignaledCounter())
			{
				for (auto buffer : items.Resources)
				{
					// Return to pool
					_freeBuffers.push_back({ buffer, Application::FrameCount });
				}

				e.Items.erase(e.Items.begin() + fenceIndex);
			}
		}

		if (e.Items.empty())
		{
			_pendingBuffers.erase(_pendingBuffers.begin() + i);
		}
	}

	// Free staging buffers that has not been used for a few frames
	const U64 SafeFramesCount = 30;
	for (I32 i = _freeBuffers.size() - 1; i >= 0; i--)
	{
		auto& e = _freeBuffers[i];
		if (e.FrameNumber + SafeFramesCount < Application::FrameCount)
		{
			auto buffer = e.Buffer;

			// Remove buffer from lists
			//_allBuffers.erase(buffer);
			_freeBuffers.erase(_freeBuffers.begin() + i);

#if !BUILD_RELEASE
			// Update stats
			//_allBuffersFreeSize += buffer->GetSize();
			//_allBuffersTotalSize -= buffer->GetSize();
#endif

			// Release memory
			//buffer->ReleaseGPU();
			Delete(buffer);
		}
	}
}

void StagingManagerVulkan::Dispose()
{
	ScopeLock lock(_locker);

#if !IS_RELEASE
	IS_CORE_INFO("Vulakn staging buffers peek memory usage: {0}, allocs: {1}, frees: {2}", BytesToText(_allBuffersPeekSize), BytesToText(_allBuffersAllocSize), BytesToText(_allBuffersFreeSize));
#endif

	// Release buffers and clear memory
	for (auto buffer : _allBuffers)
	{
		//buffer->ReleaseGPU();
		Delete(buffer);
	}
	_allBuffers.resize(0);
	_pendingBuffers.resize(0);
}

StagingManagerVulkan::PendingItemsPerCmdBuffer* StagingManagerVulkan::FindOrAdd(CmdBufferVulkan* cmdBuffer)
{
	for (I32 i = 0; i < _pendingBuffers.size(); i++)
	{
		if (_pendingBuffers[i].CmdBuffer == cmdBuffer)
		{
			return &_pendingBuffers[i];
		}
	}

	_pendingBuffers.push_back(PendingItemsPerCmdBuffer());
	const auto item = &_pendingBuffers.back();
	item->CmdBuffer = cmdBuffer;
	item->Items.clear();
	return item;
}

StagingManagerVulkan::PendingItems* StagingManagerVulkan::PendingItemsPerCmdBuffer::FindOrAdd(U64 fence)
{
	for (I32 i = 0; i < Items.size(); ++i)
	{
		if (Items[i].FenceCounter == fence)
		{
			return &Items[i];
		}
	}

	Items.push_back(StagingManagerVulkan::PendingItems());
	const auto item = &Items.back();
	item->FenceCounter = fence;
	for (auto& ptr : item->Resources)
	{
		::Delete(ptr);
	}
	item->Resources.clear();
	return item;
}
