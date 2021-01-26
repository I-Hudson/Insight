#include "ispch.h"
#include "GPUDeviceVulkan.h"
#include "VulkanInitializers.h"
#include "VulkanUtils.h"
#include "Engine/Config/Config.h"
#include "GLFW/glfw3.h"
#include "VulkanPlatform.h"

#include "GPUAdapterVulkan.h"

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

GPUContext* GPUDeviceVulkan::GetMainContext()
{
	return nullptr;
}

GPUAdapter* GPUDeviceVulkan::GetAdapter()
{
	return m_adapter;
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

		const I32 queueIndex = queueCreateInfos.size();
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
	ThrowIfFailed(vkCreateDevice(gpu, &deviceInfo, nullptr, &m_device));

	return true;
}

bool GPUDeviceVulkan::LoadContent()
{
	return false;
}

void GPUDeviceVulkan::Dispose()
{
	::Delete(m_adapter);

#ifdef IS_DEBUG
	if ((bool)CONFIG_VAL(Config::GraphicsConfig.Validation))
	{
		Debug::FreeDebugCallback(m_instance);
	}
#endif

	vkDestroyDevice(m_device, nullptr);
	vkDestroyInstance(m_instance, nullptr);
}

void GPUDeviceVulkan::WaitForGPU()
{
}

GPUBuffer* GPUDeviceVulkan::NewBuffer()
{
	return nullptr;
}

GPUTexture* GPUDeviceVulkan::NewTexture()
{
	return nullptr;
}
