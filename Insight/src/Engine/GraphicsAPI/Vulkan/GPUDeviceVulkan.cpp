#include "ispch.h"
#include "Engine/GraphicsAPI/Vulkan/GPUDeviceVulkan.h"
#include "Engine/GraphicsAPI/Vulkan/GPUSyncVulkan.h"
#include "VulkanInitializers.h"
#include "VulkanDebug.h"
#include "Engine/GraphicsAPI/Vulkan/VulkanUtils.h"
#include "Engine/Config/Config.h"
#include "GLFW/glfw3.h"
#include "Engine/GraphicsAPI/Vulkan/VulkanPlatform.h"
#include "Engine/Threading/Threading.h"

#include "Engine/GraphicsAPI/Vulkan/GPUAdapterVulkan.h"
#include "Engine/GraphicsAPI/Vulkan/GPUCommandBufferVulkan.h"

#include "Engine/Graphics/GPUBufferDesc.h"
#include "Engine/Graphics/Image/GPUImage.h"

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
{
}

GPUDeviceVulkan::~GPUDeviceVulkan()
{
}

VkQueue GPUDeviceVulkan::GetQueue(GPUQueue queue)
{
	switch (queue)
	{
		case GPUQueue::GRAPHICS: return m_graphicsQueue;
		case GPUQueue::COMPUTE: return m_computeQueue;
		case GPUQueue::TRANSFER: return m_transferQueue;
		default: return m_graphicsQueue;
	}
}

u32 GPUDeviceVulkan::GetQueueFamilyIndex(GPUQueue queue)
{
	switch (queue)
	{
		case GPUQueue::GRAPHICS: return m_graphicsQueueFamilyIndex;
		case GPUQueue::COMPUTE: return m_computeQueueFamilyIndex;
		case GPUQueue::TRANSFER: return m_transferQueueFamilyIndex;
		default: return m_graphicsQueueFamilyIndex;
	}
}

Insight::Graphics::GPUFenceManager* GPUDeviceVulkan::GetDefaultFenceManager()
{
	return GPUFenceManager;
}

Insight::Graphics::GPUSemaphoreManager* GPUDeviceVulkan::GetDefaultSignalManager()
{
	return GPUSignalManager;
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

std::vector<const char*> StringVectorToConstChar(const std::vector<std::string>& vec)
{
	std::vector<const char*> v;
	for (auto& value : vec)
	{
		v.push_back(value.c_str());
	}
	return v;
}

bool GPUDeviceVulkan::Init()
{
	// Get all the extensions we need/want for vulkan.
	std::vector<std::string> instanceExtensions, layerExtentions;
	GetInstanceExtensions(instanceExtensions, layerExtentions);

	// Create a vulkan instance.
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Debug";
	appInfo.pEngineName = "Insight";
	appInfo.apiVersion = VK_MAKE_VERSION(INSIGHT_MAJOR, INSIGHT_MINOR, INSIGHT_PATCH);

	std::vector<const char*> instanceExtensionsCC = StringVectorToConstChar(instanceExtensions);
	std::vector<const char*> layerExtentionsCC = StringVectorToConstChar(layerExtentions);
	VkInstanceCreateInfo instanceCreateInfo{};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pNext = NULL;
	instanceCreateInfo.pApplicationInfo = &appInfo;
	instanceCreateInfo.enabledExtensionCount = static_cast<u32>(instanceExtensionsCC.size());
	instanceCreateInfo.ppEnabledExtensionNames = instanceExtensionsCC.data();
	instanceCreateInfo.ppEnabledLayerNames = layerExtentionsCC.data();
	instanceCreateInfo.enabledLayerCount = static_cast<u32>(layerExtentionsCC.size());
	ThrowIfFailed(vkCreateInstance(&instanceCreateInfo, nullptr, &m_instance));

#ifdef IS_DEBUG
	if ((bool)CONFIG_VAL(Config::GraphicsConfig.Validation))
	{
		// The report flags determine what type of messages for the layers will be displayed
		// For validating (debugging) an application the error and warning bits should sufficeVK_EXT_debug_report
		VkDebugReportFlagsEXT debugReportFlags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
		// Additional flags include performance info, loader and layer debug messages, etc.
		Insight::GraphicsAPI::Vulkan::Debug::SetupDebugging(m_instance, debugReportFlags, VK_NULL_HANDLE);
	}
#endif

	// Get physical device
	u32 gpuCount = 0;
	ThrowIfFailed(vkEnumeratePhysicalDevices(m_instance, &gpuCount, nullptr));
	ASSERT(gpuCount > 0);
	std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
	ThrowIfFailed(vkEnumeratePhysicalDevices(m_instance, &gpuCount, physicalDevices.data()));
	std::vector<GPUAdapterVulkan> adapters;
	u32 gpuIndex = 0;
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
	u32 selectedGpu = 0;
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
	u32 queueCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueCount, nullptr);
	ASSERT(queueCount >= 1);
	m_queueFamilyProps.resize(queueCount);
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueCount, m_queueFamilyProps.data());

	// Query the device features.
	vkGetPhysicalDeviceFeatures(gpu, &m_physicalDeviceFeatures);

	// Setup device extensions and layers.
	std::vector<std::string> deviceExtensions;
	std::vector<std::string> validationLayers;
	GetDeviceExtensionsAndLayers(gpu, deviceExtensions, validationLayers);

	std::vector<const char*> deviceExtensionsCC = StringVectorToConstChar(deviceExtensions);
	std::vector<const char*> validationLayersCC = StringVectorToConstChar(validationLayers);
	VkDeviceCreateInfo deviceInfo{};
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.enabledExtensionCount = static_cast<u32>(deviceExtensionsCC.size());
	deviceInfo.ppEnabledExtensionNames = deviceExtensionsCC.data();
	deviceInfo.enabledLayerCount = static_cast<u32>(validationLayers.size());
	deviceInfo.ppEnabledLayerNames = deviceInfo.enabledLayerCount > 0 ? validationLayersCC.data() : nullptr;

	// Setup queues info
	std::unordered_map<GPUQueue, VkDeviceQueueCreateInfo> queueCreateInfos{};
	VkDeviceQueueCreateInfo* queueInfo = nullptr;
	i32 graphicsQueueIndex = -1;
	i32 computeQueueIndex = -1;
	i32 transferQueueIndex = -1;
	IS_INFO("Found {0} queue families:", m_queueFamilyProps.size());
	u32 numPriorities = 0;
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
				queueInfo = &queueCreateInfos[GPUQueue::GRAPHICS];
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
				queueInfo = &queueCreateInfos[GPUQueue::COMPUTE];
			}
		}

		if ((curProps.queueFlags & VK_QUEUE_TRANSFER_BIT) == VK_QUEUE_TRANSFER_BIT)
		{
			// Favor a non-gfx transfer queue
			if (transferQueueIndex == -1 && (curProps.queueFlags & VK_QUEUE_GRAPHICS_BIT) != VK_QUEUE_GRAPHICS_BIT && (curProps.queueFlags & VK_QUEUE_COMPUTE_BIT) != VK_QUEUE_COMPUTE_BIT)
			{
				transferQueueIndex = i;
				isValidQueue = true;
				queueInfo = &queueCreateInfos[GPUQueue::TRANSFER];
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
		//queueCreateInfos.push_back(VkDeviceQueueCreateInfo{});
		VkDeviceQueueCreateInfo& curQueue = *queueInfo;
		curQueue.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		curQueue.queueFamilyIndex = i;
		curQueue.queueCount = curProps.queueCount;
		numPriorities += curProps.queueCount;
		IS_INFO("- queue family {0}: {1} queues{2}", i, curProps.queueCount, queueTypeInfo);
	}

	//TODO: Remove this when QueueVulkan class is made.
	m_graphicsQueueIndex = graphicsQueueIndex;
	m_computeQueueIndex = computeQueueIndex;
	m_transferQueueIndex = transferQueueIndex;
	m_graphicsQueueFamilyIndex = queueCreateInfos[GPUQueue::GRAPHICS].queueFamilyIndex;
	m_computeQueueFamilyIndex = queueCreateInfos[GPUQueue::COMPUTE].queueFamilyIndex;
	m_transferQueueFamilyIndex = queueCreateInfos[GPUQueue::TRANSFER].queueFamilyIndex;

	std::vector<float> queuePriorities;
	queuePriorities.resize(numPriorities);
	float* currentPriority = queuePriorities.data();
	u32 index = 0;
	for (auto& item : queueCreateInfos)
	{
		VkDeviceQueueCreateInfo& queue = item.second;
		queue.pQueuePriorities = currentPriority;
		const VkQueueFamilyProperties& properties = m_queueFamilyProps[queue.queueFamilyIndex];
		for (I32 queueIndex = 0; queueIndex < (I32)properties.queueCount; queueIndex++)
		{
			*currentPriority++ = 1.0f;
		}
		++index;
	}
	deviceInfo.queueCreateInfoCount = static_cast<u32>(queueCreateInfos.size());
	auto values = UnorderedMapValuesToVector(queueCreateInfos);
	deviceInfo.pQueueCreateInfos = values.data();

	VkPhysicalDeviceFeatures enabledFeatures;
	VulkanPlatform::EnabledPhysicalDeviceFeatures(m_physicalDeviceFeatures, enabledFeatures);
	deviceInfo.pEnabledFeatures = &enabledFeatures;
	ThrowIfFailed(vkCreateDevice(gpu, &deviceInfo, nullptr, &Device));
	ASSERT(Device != VK_NULL_HANDLE);

#ifdef IS_DEBUG
	if ((bool)CONFIG_VAL(Config::GraphicsConfig.Validation))
	{
		::New<Insight::GraphicsAPI::Vulkan::GPUDebugMarkerVulkan>();
		Insight::GraphicsAPI::Vulkan::GPUDebugMarkerVulkan::Instance()->Init();
	}
#endif

	// Create queues
	if (graphicsQueueIndex == -1)
	{
		IS_ERROR("Missing Vulkan graphics queue.");
		return false;
	}
	//GraphicsQueue = ::New<QueueVulkan>(this, graphicsQueueIndex);
	//ComputeQueue = computeQueueIndex != -1 ? ::New<QueueVulkan>(this, computeQueueIndex) : GraphicsQueue;
	//TransferQueue = transferQueueIndex != -1 ? ::New<QueueVulkan>(this, transferQueueIndex) : GraphicsQueue;
	vkGetDeviceQueue(Device, queueCreateInfos[GPUQueue::GRAPHICS].queueFamilyIndex, m_graphicsQueueIndex, &m_graphicsQueue);
	vkGetDeviceQueue(Device, queueCreateInfos[GPUQueue::COMPUTE].queueFamilyIndex, m_computeQueueFamilyIndex, &m_computeQueue);
	vkGetDeviceQueue(Device, queueCreateInfos[GPUQueue::TRANSFER].queueFamilyIndex, m_transferQueueFamilyIndex, &m_transferQueue);

	// Init device limits 
	PhysicalDeviceLimits = m_adapter->GpuProps.limits;
	SampleLevel maxSampleLevel = SampleLevel::None;
	if (m_physicalDeviceFeatures.sampleRateShading)
	{
		const I32 framebufferColorSampleCount = GetMaxSampleCount(PhysicalDeviceLimits.framebufferColorSampleCounts);
		const I32 framebufferDepthSampleCount = GetMaxSampleCount(PhysicalDeviceLimits.framebufferDepthSampleCounts);
		maxSampleLevel = (SampleLevel)std::clamp(std::min<I32>(framebufferColorSampleCount, framebufferDepthSampleCount), 1, 8);
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
	limits.MaximumMipLevelsCount = std::min(static_cast<I32>(std::log2(PhysicalDeviceLimits.maxImageDimension2D)), GPU_MAX_TEXTURE_MIP_LEVELS);
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

		SampleLevel msaa = SampleLevel::None;
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
				msaa = maxSampleLevel;
		}

		m_featuresPerFormat[i] = FormatFeatures(format, msaa, support);
	}


	// Setup memory limit and print memory info
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(gpu, &memoryProperties);
	IS_INFO("Max memory allocations: {0}", m_adapter->GpuProps.limits.maxMemoryAllocationCount);
	IS_INFO("Found {0} device memory heaps:", memoryProperties.memoryHeapCount);
	for (u32 i = 0; i < memoryProperties.memoryHeapCount; ++i)
	{
		const VkMemoryHeap& heap = memoryProperties.memoryHeaps[i];
		bool isGPUHeap = (heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) == VK_MEMORY_HEAP_DEVICE_LOCAL_BIT;
		IS_INFO("-  memory heap {0}: flags 0x{1:x}, size {2} MB (GPU: {3})", i, heap.flags, (u32)(heap.size / 1024 / 1024), isGPUHeap);
		if (isGPUHeap)
			m_totalGraphicsMemory += heap.size;
	}
	IS_INFO("Found {0} device memory types:", memoryProperties.memoryTypeCount);
	for (u32 i = 0; i < memoryProperties.memoryTypeCount; ++i)
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

	m_defaultCommandPool = Insight::GraphicsAPI::Vulkan::GPUCommandPoolVulkan::New();
	m_defaultCommandPool->Init(Insight::Graphics::GPUCommandPoolDesc(Insight::Graphics::GPUCommandPoolFlags::RESET_COMMAND_BUFFER, GPUQueue::GRAPHICS));

	m_samplerCache.SetCreateFunc([]()
	{
		return Insight::Graphics::GPUSampler::New();
	});
	m_shaderCache.SetCreateFunc([]()
	{
		return Insight::Graphics::GPUShader::New();
	});
	m_imageCache.SetCreateFunc([]()
	{
		return Insight::Graphics::GPUImage::New();
	});

	ThrowIfFailed(vmaCreateAllocator(&allocatorInfo, &VmaAllocator));

	//Prepare other things for the engine to use in relation to vulkan.
	FenceManager.Init(this);
	PipelineEventManger.Init(this);

	GPUFenceManager = ::New<Insight::GraphicsAPI::Vulkan::GPUFenceManagerVulkan>();
	GPUSignalManager = ::New<Insight::GraphicsAPI::Vulkan::GPUSemaphoreManagerVulkan>();

	VkQueue vkQueues[] = { m_graphicsQueue };
	U32 vkQueuesIndex[] = { m_graphicsQueueFamilyIndex };
	IS_PROFILE_GPU_INIT_VULKAN(&Device, &m_adapter->Gpu, vkQueues, vkQueuesIndex, 1, nullptr);

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
	m_defaultCommandPool->ReleaseGPU();
	SAFE_DELETE(m_defaultCommandPool);

	m_samplerCache.ReleaseGPUAll();
	m_shaderCache.ReleaseGPUAll();
	m_imageCache.ReleaseGPUAll();

	//SAFE_DELETE(GraphicsQueue);
	//SAFE_DELETE(ComputeQueue);
	//SAFE_DELETE(TransferQueue);
	FenceManager.Dispose();
	PipelineEventManger.Dispose();
	GPUFenceManager->Release();
	SAFE_DELETE(GPUFenceManager);
	SAFE_DELETE(GPUSignalManager);
	IS_PROFILE_GPUI_SHUTDOWN();

	vmaDestroyAllocator(VmaAllocator);
	VmaAllocator = VK_NULL_HANDLE;

	SAFE_DELETE(m_adapter);

#ifdef IS_DEBUG
	if ((bool)CONFIG_VAL(Config::GraphicsConfig.Validation))
	{
		Insight::GraphicsAPI::Vulkan::Debug::FreeDebugCallback(m_instance);
		::Delete(Insight::GraphicsAPI::Vulkan::GPUDebugMarkerVulkan::Instance());

	}
#endif

	vkDestroyDevice(Device, nullptr);
	vkDestroyInstance(m_instance, nullptr);

	GPUDevice::Dispose();
	m_state = DeviceState::Disposed;
}

void GPUDeviceVulkan::WaitForGPU()
{
	vkDeviceWaitIdle(Device);
}

u32 GPUDeviceVulkan::GetQueueIndex(GPUQueue queue)
{
	switch (queue)
	{
		case GPUQueue::GRAPHICS: return m_graphicsQueueIndex;
		case GPUQueue::COMPUTE: return m_computeQueueIndex;
		case GPUQueue::TRANSFER: return m_transferQueueIndex;
	}
	return m_graphicsQueueIndex;
}

void GPUDeviceVulkan::BeginFrame()
{
}

void GPUDeviceVulkan::EndFrame()
{
}

PipelineEventVulkan::PipelineEventVulkan(GPUDeviceVulkan* device, PipelineEventManagerVulkan* owner)
	: m_owner(owner)
{
	auto createInfo = vks::initializers::eventCreateInfo();
	ThrowIfFailed(vkCreateEvent(device->Device, &createInfo, nullptr, &m_event));
}

bool PipelineEventVulkan::IsSignaled() const
{
	return false;
}

PipelineEventManagerVulkan::PipelineEventManagerVulkan()
{
}

PipelineEventManagerVulkan::~PipelineEventManagerVulkan()
{
}

void PipelineEventManagerVulkan::Dispose()
{
	ScopeLock lock(m_device->m_pipelineEventLock);

	ASSERT(m_usedEvents.empty());
	for (PipelineEventVulkan* event : m_freeEvents)
	{
		DestroyEvent(event);
	}
	m_freeEvents.clear();
}

PipelineEventVulkan* PipelineEventManagerVulkan::AllocateEvent(bool createSignaled)
{
	ScopeLock lock(m_device->m_pipelineEventLock);

	PipelineEventVulkan* event;
	if (!m_freeEvents.empty())
	{
		event = m_freeEvents.back();
		m_freeEvents.pop_back();
		m_usedEvents.push_back(event);

		if (createSignaled)
		{
			event->m_signaled = true;
		}

		return event;
	}

	event = ::New<PipelineEventVulkan>(m_device, this);
	m_usedEvents.push_back(event);
	return event;
}

bool PipelineEventManagerVulkan::IsEventSignaled(PipelineEventVulkan* event)
{
	if (event->m_signaled)
	{
		return true;
	}
	return CheckForEventStatus(event);
};

void PipelineEventManagerVulkan::ResetEvent(PipelineEventVulkan*& event)
{
	ThrowIfFailed(vkResetEvent(m_device->Device, event->GetHandle()));
}

void PipelineEventManagerVulkan::ReleaseEvent(PipelineEventVulkan*& event)
{
	ScopeLock lock(m_device->m_pipelineEventLock);

	ResetEvent(event);
	m_usedEvents.erase(m_usedEvents.begin() + VectorFindIndex(m_usedEvents, event));
	m_freeEvents.push_back(event);
	event = nullptr;
}


bool PipelineEventManagerVulkan::CheckForEventStatus(PipelineEventVulkan* event)
{
	ASSERT(VectorContains(m_usedEvents, event));
	ASSERT(!event->m_signaled);

	const VkResult res = vkGetEventStatus(m_device->Device, event->GetHandle());
	if (res == VK_SUCCESS)
	{
		event->m_signaled = true;
		return true;
	}
	return false;
}

void PipelineEventManagerVulkan::DestroyEvent(PipelineEventVulkan* event)
{
	vkDestroyEvent(m_device->Device, event->GetHandle(), nullptr);
	event->m_event = VK_NULL_HANDLE;
	::Delete(event);
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
	//_device->DeferredDeletionQueue.EnqueueResource(DeferredDeletionQueueVulkan::Semaphore, _semaphoreHandle);
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

	fence = ::New<FenceVulkan>(m_device, this, createSignaled);
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