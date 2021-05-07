#include "ispch.h"
#include	"VulkanHeader.h"
#include "VulkanRenderer.h"
#include "VulkanDebug.h"
#include "VulkanDevice.h"

#include <GLFW/glfw3.h>

#include "VulkanImGUIRenderer.h"
#include "glslang/Public/ShaderLang.h"

#include <glm/gtc/constants.hpp>
#include <glm/gtx/string_cast.hpp>

#include "Engine/Config/Config.h"
#include "Engine/Event/EventManager.h"
#include "Engine/Component/CameraComponent.h"
#include "Engine/Component/TransformComponent.h"
#include "Engine/Component/MeshComponent.h"
#include "Engine/Model/Model.h"
#include "Engine/Graphics/Texture.h"
#include "Engine/FileSystem/FileSystem.h"

#include "Engine/Config/Config.h"
#include "Engine/Instrumentor/Instrumentor.h"
#include "Engine/Time/Time.h"
#include "Engine/Graphics/ImGuiRenderer.h"
#include <Engine/GraphicsAPI/Vulkan/VulkanUtils.h>

namespace vks
{
	std::string GetFormatInt(uint64_t i)
	{
		std::string iStr = std::to_string(i);
		std::string returnStr;

		int cIndex = 0;
		for (size_t ii = iStr.size() - 1; ii > 0; --ii)
		{
			if (cIndex % 3 == 0 && ii != iStr.size() - 1)
			{
				returnStr.insert(0, ",");
			}
			cIndex++;
			returnStr.insert(0, std::string(1, iStr[ii]));
		}
		returnStr.insert(0, std::string(1, iStr[0]));
		return returnStr;
	}

//#if defined(IS_EDITOR)
//	VulkanRendererEditorOverlay::VulkanRendererEditorOverlay(SharedPtr<Module::EditorModule> editorModule, SharedPtr<VulkanRenderer> renderer)
//		: EditorWindow(editorModule)
//		, m_renderer(renderer)
//	{
//	}
//
//	VulkanRendererEditorOverlay::~VulkanRendererEditorOverlay()
//	{
//	}
//
//	void VulkanRendererEditorOverlay::Update(const float& deltaTime)
//	{
//
//		ImGui::Begin("VulkanRendererEditorOverlay");
//
//		if (SharedPtr< VulkanRenderer> renderer = m_renderer.lock())
//		{
//			ImGui::Checkbox("Debug overlay", &renderer->debugOverlay);
//
//			const char* items[] = { "Normal", "Colour", "Dynamic Uniform Colour" };
//			static int item_current = renderer->m_debugOverlay.debugOptions.x == 1 ? 0 : 1;
//			ImGui::Combo("combo", &item_current, items, ARRAY_SIZEOF(items));
//			renderer->m_debugOverlay.debugOptions[0] = 0;
//			renderer->m_debugOverlay.debugOptions[1] = 0;
//			renderer->m_debugOverlay.debugOptions[2] = 0;
//			renderer->m_debugOverlay.debugOptions[item_current] = 1;
//
//			UIHelper::DrawString("EditorCamera");
//			UIHelper::DrawString("EditorCamera", ShowInEditor | TextBold);
//			float fov = renderer->m_editorCamera->GetFov();
//			float nearPlane = renderer->m_editorCamera->GetNearPlane();
//			float farPlane = renderer->m_editorCamera->GetFarPlane();
//			float cameraSpeed = renderer->m_editorCamera->GetCameraSpeed();
//			if (UIHelper::DrawFloat("FOV", &fov) || UIHelper::DrawFloat("Near Plane", &nearPlane) || UIHelper::DrawFloat("Far Plane", &farPlane) || UIHelper::DrawFloat("Camera Speed", &cameraSpeed))
//			{
//				renderer->m_editorCamera->SetProjMatrix(fov, nearPlane, farPlane);
//				renderer->m_editorCamera->SetCameraSpeed(cameraSpeed);
//			}
//
//			// TODO: THIS IS NOT WORKING. WHY!!!!!!!!!!!!!!!
//
//			glm::vec4 rotation = glm::vec4(0, 0, 0, 0);
//			rotation.x = glm::degrees(atan2(renderer->m_testModelMatrix[1][2], renderer->m_testModelMatrix[2][2]));
//			rotation.y = glm::degrees(atan2(-renderer->m_testModelMatrix[0][2], glm::sqrt((renderer->m_testModelMatrix[0][0] * renderer->m_testModelMatrix[0][0]) + (renderer->m_testModelMatrix[0][1] * renderer->m_testModelMatrix[0][1]))));
//			rotation.z = glm::degrees(atan2((sin(rotation.x) * renderer->m_testModelMatrix[2][0]) - (cos(rotation.x) * renderer->m_testModelMatrix[1][0]),
//				(cos(rotation.x) * renderer->m_testModelMatrix[1][1] - (sin(rotation.x) * renderer->m_testModelMatrix[2][1]))));
//			if (UIHelper::DrawVector("Rotation", 3, &rotation.x))
//			{
//				glm::mat4 newMatrix(1.0f);
//
//				float radX = glm::radians(rotation.x);
//				float radY = glm::radians(rotation.y);
//				float radZ = glm::radians(rotation.z);
//
//				newMatrix = glm::rotate(newMatrix, radX, glm::vec3(1, 0, 0));
//				//newMatrix = glm::rotate(newMatrix, radY, glm::vec3(0, 1, 0));
//				newMatrix = glm::rotate(newMatrix, radZ, glm::vec3(0, 0, 1));
//
//				renderer->m_testModelMatrix = newMatrix;
//			}
//			IS_CORE_INFO("{0}", rotation.x);
//
//			UIHelper::DrawMat4("Test Model Matrix", &renderer->m_testModelMatrix);
//		}
//		ImGui::End();
//	}
//#endif

	VulkanRenderer::VulkanRenderer()
	{ }

	VulkanRenderer::~VulkanRenderer()
	{
		UNREG_EVENT_HANDLE(EventType::WindowResize);

		glslang::FinalizeProcess();

		::Delete(m_editorEntity);

		m_swapchain.CleanUp();
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			::Delete(m_presentMaterials[i]);
			::Delete(m_presentMeshes[i]);
		}

		DestroyCommandBuffers();

		m_frameBuffer.~VulkanFrameBuffer();
		for (uint32_t i = 0; i < m_presentFrameBuffers.size(); i++)
		{
			vkDestroyFramebuffer(m_device, m_presentFrameBuffers[i], nullptr);
		}
		
		for (auto& shaderModule : m_shaderModules)
		{
			vkDestroyShaderModule(m_device, shaderModule, nullptr);
		}
		vkDestroyImageView(m_device, m_depthStencil.View, nullptr);
		vkDestroyImage(m_device, m_depthStencil.Image, nullptr);
		vkFreeMemory(m_device, m_depthStencil.Mem, nullptr);
		
		vkDestroyCommandPool(m_device, m_cmdPool, nullptr);
		
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			vkDestroySemaphore(m_device, m_semaphores.ImageAquired[i], nullptr);
			vkDestroySemaphore(m_device, m_semaphores.RenderComplete[i], nullptr);
		
			vkDestroyFence(m_device, m_waitFences[i], nullptr);
			m_waitImagesFences[i] = VK_NULL_HANDLE;
		}
		
		for (auto& thread : m_threadData)
		{
			vkFreeCommandBuffers(m_device, thread.commandPool, static_cast<uint32_t>(thread.commandBuffer.size()), thread.commandBuffer.data());
			vkDestroyCommandPool(m_device, thread.commandPool, nullptr);
		}
		vkDestroyRenderPass(m_device, m_presentRenderPass, nullptr);

		IS_PROFILE_GPUI_SHUTDOWN();

		::Delete(m_vulkanDevice);

		if ((bool)CONFIG_VAL(Config::GraphicsConfig.Validation))
		{
			vks::Debug::FreeDebugCallback(m_instance);
		}

		vkDestroyInstance(m_instance, nullptr);
	}

	void VulkanRenderer::Init()
	{
		REG_EVENT_HANDLE(EventType::WindowResize, VulkanRenderer::OnWindowResizeEvent);

		m_numThreads = std::thread::hardware_concurrency();
		IS_CORE_ASSERT(m_numThreads > 0, "Number of threads has to be greater than 0.");
		IS_CORE_INFO("numThreads = {0}", m_numThreads);

		m_threadPool.set_thread_count(m_numThreads);
		m_numObjectsPerThread = 512 / m_numThreads;

		//m_rndEngine.seed((unsigned)time(nullptr));

		glslang::InitializeProcess();
		InitVulkan();
		IS_CORE_INFO("Vulkan Setup Complete.");
	}

	void VulkanRenderer::InitVulkan()
	{
		VkResult err;

		ThrowIfFailed(CreateInstance((bool)CONFIG_VAL(Config::GraphicsConfig.Validation)));

		if ((bool)CONFIG_VAL(Config::GraphicsConfig.Validation))
		{
			// The report flags determine what type of messages for the layers will be displayed
			// For validating (debugging) an application the error and warning bits should sufficeVK_EXT_debug_report
			VkDebugReportFlagsEXT debugReportFlags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
			// Additional flags include performance info, loader and layer debug messages, etc.
			vks::Debug::SetupDebugging(m_instance, debugReportFlags, VK_NULL_HANDLE);
		}

		// Physical device
		uint32_t gpuCount = 0;
		// Get number of available physical devices
		ThrowIfFailed(vkEnumeratePhysicalDevices(m_instance, &gpuCount, nullptr));
		assert(gpuCount > 0);
		// Enumerate devices
		std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
		err = vkEnumeratePhysicalDevices(m_instance, &gpuCount, physicalDevices.data());
		if (err)
		{
			IS_CORE_ASSERT(false, "Could not enumerate physical devices : " + vks::errorString(err));
		}
		// GPU selection

		// Select physical device to be used for the Vulkan example
		// Defaults to the first device unless specified by command line
		uint32_t selectedDevice = 0;
		gpuCount = 0;
		ThrowIfFailed(vkEnumeratePhysicalDevices(m_instance, &gpuCount, nullptr));
		if (gpuCount == 0)
		{
			std::cerr << "No Vulkan devices found!" << "\n";
		}
		else
		{
			// Enumerate devices
			std::cout << "Available Vulkan devices" << "\n";
			std::vector<VkPhysicalDevice> devices(gpuCount);
			ThrowIfFailed(vkEnumeratePhysicalDevices(m_instance, &gpuCount, devices.data()));
			for (uint32_t j = 0; j < gpuCount; j++)
			{
				VkPhysicalDeviceProperties deviceProperties;
				vkGetPhysicalDeviceProperties(devices[j], &deviceProperties);
				IS_CORE_INFO("Device [{0}] : {1}", j, deviceProperties.deviceName);
				IS_CORE_INFO(" Type: {0}", vks::physicalDeviceTypeString(deviceProperties.deviceType));
				IS_CORE_INFO(" API: {0}.{1}.{2}", (deviceProperties.apiVersion >> 22), ((deviceProperties.apiVersion >> 12) & 0x3ff), (deviceProperties.apiVersion & 0xfff));
			}
		}

		m_physicalDevice = physicalDevices[selectedDevice];

		// Store properties (including limits), features and memory properties of the physical device (so that examples can check against them)
		vkGetPhysicalDeviceProperties(m_physicalDevice, &m_deviceProperties);
		vkGetPhysicalDeviceFeatures(m_physicalDevice, &m_deviceFeatures);
		vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &m_deviceMemoryProperties);

		GetEnabledFeatures();

		// Vulkan device creation
		// This is handled by a separate class that gets a logical device representation
		// and encapsulates functions related to a device
		m_vulkanDevice = ::New<vks::VulkanDevice>(m_physicalDevice, m_instance);
		VkResult res = m_vulkanDevice->CreateLogicalDevice(m_enabledFeatures, m_enabledDeviceExtensions, m_deviceCreatepNextChain);
		if (res != VK_SUCCESS)
		{
			IS_CORE_FATEL("Could not create Vulkan device: {0}", vks::errorString(res), res);
		}
		m_device = m_vulkanDevice->m_logicalDevice;
		m_vulkanDevice->m_swapChainFormat = m_swapchain.GetColorFormat();

		// Get a graphics queue from the device
		vkGetDeviceQueue(m_device, m_vulkanDevice->m_queueFamilyIndices.graphics, 0, &m_queue);

		// Find a suitable depth format
		VkBool32 validDepthFormat = vks::getSupportedDepthFormat(m_physicalDevice, &m_depthFormat);
		assert(validDepthFormat);

		m_swapchain.Connect(m_instance, m_physicalDevice, m_device);

		// Create synchronization objects
		VkSemaphoreCreateInfo semaphoreCreateInfo = vks::initializers::semaphoreCreateInfo();
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			// Create a semaphore used to synchronize image presentation
			// Ensures that the image is displayed before we start submitting new commands to the queue
			ThrowIfFailed(vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_semaphores.ImageAquired[i]));
			// Create a semaphore used to synchronize command submission
			// Ensures that the image is not presented until all commands have been submitted and executed
			ThrowIfFailed(vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_semaphores.RenderComplete[i]));
		}
		m_submitInfo = GetSubmitInfo();
		Prepare();

		m_clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
	}

	void VulkanRenderer::Clear()
	{
		IS_PROFILE_FUNCTION();

		//vkWaitForFences(m_device, 1, &m_waitFences[0], VK_TRUE, UINT64_MAX);

		VkResult result = m_swapchain.AcquireNextImage(m_semaphores.ImageAquired[0], &m_imageIndex);
		if ((result == VK_ERROR_OUT_OF_DATE_KHR) || (result == VK_SUBOPTIMAL_KHR))
		{
			OnWindowResizeEvent(WindowResizeEvent(Window::GetWidth(), Window::GetHeight()));
		}
		else
		{
			ThrowIfFailed(result);
		}

		VkFence f = m_frameBuffer.GetFence();
		if (f != VK_NULL_HANDLE)
		{
			//vkWaitForFences(m_device, 1, &f, VK_TRUE, UINT64_MAX);
		}

		//m_waitImagesFences[m_imageIndex] = m_waitFences[0];
		//vkResetFences(m_device, 1, &f);
	}

	// This should be moved to a submodule for every type of rendering we are doing.
	void VulkanRenderer::Render(CameraComponent* mainCamera, std::vector<MeshComponent*>& meshes)
	{
		IS_PROFILE_FUNCTION();

		m_vulkanDevice->EnableIdleCommands();

		Clear();

		{
			IS_PROFILE_SCOPE("Vulkan wait for fence");
			m_frameBuffer.WaitForFence();
			m_frameBuffer.ResetFence();
		}

		if (true)
		{
			BuildCommandBuffers(meshes);
		}
		else
		{
			UpdateCommandBuffer(m_presentFrameBuffers[m_imageIndex]);
		}	
		BuildPresentBuffers();

		// Offscreen rendering
		VkSubmitInfo submitInfo = vks::initializers::submitInfo();
		submitInfo.pWaitDstStageMask = &m_submitPipelineStages;

		VkSemaphore offscreenWaitSemahores[] = { m_semaphores.ImageAquired[0] };
		submitInfo.pWaitSemaphores = offscreenWaitSemahores;
		submitInfo.waitSemaphoreCount = 1;
		VkSemaphore signalSemahores[] = { m_frameBuffer.GetFinishedSemaphore() };
		submitInfo.pSignalSemaphores = signalSemahores;
		submitInfo.signalSemaphoreCount = 1;

		submitInfo.commandBufferCount = 2;
		VkCommandBuffer commandBuffers[] = { m_frameBufferCmdBuffer, m_frameBufferCmdBufferTest };
		submitInfo.pCommandBuffers = commandBuffers;
		ThrowIfFailed(vkQueueSubmit(m_queue, 1, &submitInfo, m_frameBuffer.GetFence()));

		// Scene rendering
		// Wait semaphores
		VkSemaphore waitSemahores[] = { m_frameBuffer.GetFinishedSemaphore() };
		submitInfo.pWaitSemaphores = waitSemahores;
		// Signal ready with render complete semaphore
		signalSemahores[0] = m_semaphores.RenderComplete[0];
		submitInfo.pSignalSemaphores = signalSemahores;
		submitInfo.commandBufferCount = 1;
		// Submit work
		submitInfo.pCommandBuffers = &m_presentCmdBuffers[m_currentFrame];
		ThrowIfFailed(vkQueueSubmit(m_queue, 1, &submitInfo, VK_NULL_HANDLE));

		Present();

		m_editorEntity->OnUpdate(Time::GetDeltaTime());

		//TODO: Look into this. Maybe change how it works.
		m_vulkanDevice->CheckIdleQueue();
	}

	void VulkanRenderer::Present()
	{
		IS_PROFILE_FUNCTION();

		VkResult result = m_swapchain.QueuePresent(m_queue, m_imageIndex, m_semaphores.RenderComplete[0]);
		if (!((result == VK_SUCCESS) || (result == VK_SUBOPTIMAL_KHR)))
		{
			if (result == VK_ERROR_OUT_OF_DATE_KHR)
			{
				// Swap chain is no longer compatible with the surface and needs to be recreated
				WindowResizeEvent(WindowResizeEvent(Window::GetWidth(), Window::GetHeight()));
				return;
			}
			else
			{
				ThrowIfFailed(result);
			}
		}
		m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void VulkanRenderer::WaitForIdle()
	{
		ThrowIfFailed(vkDeviceWaitIdle(m_device));
	}

	void VulkanRenderer::Prepare()
	{
		IS_PROFILE_FUNCTION();

		if (m_vulkanDevice->m_enableDebugMarkers)
		{
			vks::DebugMarker::Setup(m_device);
		}
		
		InitSwapchain();
		SetupSwapchain();
		CreateCommandPool();
		CreateCommandBuffers();
		CreateSynchronizationPrimitives();
		SetupDepthStencil();
		SetupPresentRenderPass();
		SetupFrameBuffer();
		m_vulkanDevice->SetRenderPass(m_frameBuffer.GetRenderPass(), m_frameBuffer.GetRenderPassInfo());
		PrepareMultiThreadedRenderer();

		std::vector<std::string> shaders = { "./data/shaders/vulkan/present.vert",  "./data/shaders/vulkan/present.frag" };
		RenderPassInfo renderPassInfo = { 1 };
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			m_presentMaterials[i] = Material::New();
			dynamic_cast<vks::VulkanMaterial*>(m_presentMaterials[i])->Create(m_vulkanDevice, shaders, m_presentRenderPass, renderPassInfo);
		}
		SetupScreenRender();

		m_editorEntity = ::New<Entity>("Editor entity", false);
		m_editorEntity->AddComponent<TransformComponent>();
		m_editorCamera = m_editorEntity->AddComponent<CameraComponent>();

		m_lightPos = glm::vec4(0.0f, 1.0f, 3.0f, 1.0f);
		m_lightPosAngle = 0.0f;

		IS_PROFILE_GPU_INIT_VULKAN(&m_device, &m_physicalDevice, &m_queue, &m_vulkanDevice->m_queueFamilyIndices.graphics, 1, nullptr);
	}

	void VulkanRenderer::InitSwapchain()
	{
		IS_PROFILE_FUNCTION();
		m_swapchain.InitSurface(Window::m_window);
	}

	void VulkanRenderer::SetupSwapchain()
	{
		IS_PROFILE_FUNCTION();
		m_swapchain.Create(Window::GetWidth(), Window::GetHeight(), (bool)CONFIG_VAL(Config::GraphicsConfig.VSync), (bool)CONFIG_VAL(Config::GraphicsConfig.GSync));
	}

	void VulkanRenderer::CreateCommandBuffers()
	{
		IS_PROFILE_FUNCTION();
		// Create one command buffer for the offscreen rendering

		//m_frameBufferCmdBuffer.resize(m_swapchain.GetImageCount());
		VkCommandBufferAllocateInfo cmdBufAllocateInfo =
			vks::initializers::commandBufferAllocateInfo(
				m_cmdPool,
				VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				1);
		ThrowIfFailed(vkAllocateCommandBuffers(m_device, &cmdBufAllocateInfo, &m_frameBufferCmdBuffer));
		ThrowIfFailed(vkAllocateCommandBuffers(m_device, &cmdBufAllocateInfo, &m_frameBufferCmdBufferTest));

		m_presentCmdBuffers.resize(m_swapchain.GetImageCount());
		cmdBufAllocateInfo =
			vks::initializers::commandBufferAllocateInfo(
				m_cmdPool,
				VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				static_cast<U32>(m_presentCmdBuffers.size()));
		ThrowIfFailed(vkAllocateCommandBuffers(m_device, &cmdBufAllocateInfo, m_presentCmdBuffers.data()));
	}

	void VulkanRenderer::DestroyCommandBuffers()
	{
		IS_PROFILE_FUNCTION();
		vkFreeCommandBuffers(m_device, m_cmdPool, 1, &m_frameBufferCmdBuffer);
		vkFreeCommandBuffers(m_device, m_cmdPool, 1, &m_frameBufferCmdBufferTest);
		vkFreeCommandBuffers(m_device, m_cmdPool, static_cast<U32>(m_presentCmdBuffers.size()), m_presentCmdBuffers.data());
		vkFreeCommandBuffers(m_device, m_cmdPool, 1, &m_primaryCommandBuffer);
	}

	void VulkanRenderer::CreateCommandPool()
	{
		IS_PROFILE_FUNCTION();
		VkCommandPoolCreateInfo cmdPoolInfo = {};
		cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolInfo.queueFamilyIndex = m_swapchain.GetQueuNodeIndex();
		cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		ThrowIfFailed(vkCreateCommandPool(m_device, &cmdPoolInfo, nullptr, &m_cmdPool));
	}

	void VulkanRenderer::SetupScreenRender()
	{
		std::vector<Vertex> fullScreenQuadV =
		{
			Vertex{ {-1,-1,0}, {0,0,0}, {0,0,0,0}, {0,0}},
			Vertex{ {-1,1,0}, {0,0,0}, {0,0,0,0}, {0,1}},
			Vertex{ {1,1,0}, {0,0,0}, {0,0,0,0}, {1,1}},
			Vertex{ {1,-1,0}, {0,0,0}, {0,0,0,0}, {1,0}},
		};
		std::vector<U32> fullScreenQuadI = { 0,2,1, 0,3,2 };
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			m_presentMeshes[i] = ::New<Mesh>();
			m_presentMeshes[i]->SetVertices(fullScreenQuadV);
			m_presentMeshes[i]->SetIndices(fullScreenQuadI);
			m_presentMeshes[i]->Rebuild();
		}

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			m_presentMaterials[i]->UploadTexture("fullScreenQuadPositionTex", m_frameBuffer.GetAttachment("Position").ImageView, m_frameBuffer.GetSampler(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			m_presentMaterials[i]->UploadTexture("fullScreenQuadNormalTex", m_frameBuffer.GetAttachment("Normal").ImageView, m_frameBuffer.GetSampler(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			m_presentMaterials[i]->UploadTexture("fullScreenQuadColorTex", m_frameBuffer.GetAttachment("Color").ImageView, m_frameBuffer.GetSampler(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			//m_presentMaterials[i]->UploadTexture("fullScreenQuadDepthTex", m_frameBuffer.GetAttachment("Depth").ImageView, m_frameBuffer.GetSampler(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			m_presentMaterials[i]->Update();
		}
	}

	void VulkanRenderer::CreateSynchronizationPrimitives()
	{
		IS_PROFILE_FUNCTION();

		m_frameBufferCmdFence.resize(m_swapchain.GetImageCount());
		// Wait fences to sync command buffer access
		VkFenceCreateInfo fenceCreateInfo = vks::initializers::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			ThrowIfFailed(vkCreateFence(m_device, &fenceCreateInfo, nullptr, &m_waitFences[i]));
			m_waitImagesFences[i] = VK_NULL_HANDLE;

			ThrowIfFailed(vkCreateFence(m_device, &fenceCreateInfo, nullptr, &m_frameBufferCmdFence[i]));
		}
	}

	VkResult VulkanRenderer::CreateInstance(bool enableValidation)
	{
		IS_PROFILE_FUNCTION();
		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Debug";
		appInfo.pEngineName = "Insight";
		appInfo.apiVersion = VK_API_VERSION_1_0;

		std::vector<const char*> instanceExtensions;

		// Get extensions supported by the instance and store for later use
		uint32_t extCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr);
		if (extCount > 0)
		{
			std::vector<VkExtensionProperties> extensions(extCount);
			if (vkEnumerateInstanceExtensionProperties(nullptr, &extCount, &extensions.front()) == VK_SUCCESS)
			{
				for (VkExtensionProperties extension : extensions)
				{
					m_supportedInstanceExtensions.push_back(extension.extensionName);
				}
			}
		}

		// Enabled requested instance extensions
		if (m_enabledInstanceExtensions.size() > 0)
		{
			for (const char* enabledExtension : m_enabledInstanceExtensions)
			{
				// Output message if requested extension is not available
				if (std::find(m_supportedInstanceExtensions.begin(), m_supportedInstanceExtensions.end(), enabledExtension) == m_supportedInstanceExtensions.end())
				{
					std::cerr << "Enabled instance extension \"" << enabledExtension << "\" is not present at instance level\n";
				}
				instanceExtensions.push_back(enabledExtension);
			}
		}

		VkInstanceCreateInfo instanceCreateInfo = {};
		instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceCreateInfo.pNext = NULL;
		instanceCreateInfo.pApplicationInfo = &appInfo;
		if ((bool)CONFIG_VAL(Config::GraphicsConfig.Validation))
		{
			uint32_t glfwExtensionCount = 0;
			const char** glfwExtensions;
			glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

			std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

			for (size_t i = 0; i < glfwExtensionCount; ++i)
			{
				instanceExtensions.push_back(extensions[i]);
			}

			instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
		instanceCreateInfo.enabledExtensionCount = (uint32_t)instanceExtensions.size();
		instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();

		if ((bool)CONFIG_VAL(Config::GraphicsConfig.Validation))
		{
			// The VK_LAYER_KHRONOS_validation contains all current validation functionality.
			// Note that on Android this layer requires at least NDK r20
			const char* validationLayerName = "VK_LAYER_KHRONOS_validation";
			// Check if this layer is available at instance level
			uint32_t instanceLayerCount;
			vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
			std::vector<VkLayerProperties> instanceLayerProperties(instanceLayerCount);
			vkEnumerateInstanceLayerProperties(&instanceLayerCount, instanceLayerProperties.data());
			bool validationLayerPresent = false;
			for (VkLayerProperties layer : instanceLayerProperties)
			{
				if (strcmp(layer.layerName, validationLayerName) == 0)
				{
					validationLayerPresent = true;
					break;
				}
			}
			if (validationLayerPresent)
			{
				instanceCreateInfo.ppEnabledLayerNames = &validationLayerName;
				instanceCreateInfo.enabledLayerCount = 1;
			}
			else
			{
				std::cerr << "Validation layer VK_LAYER_KHRONOS_validation not present, validation is disabled";
			}
		}
		return vkCreateInstance(&instanceCreateInfo, nullptr, &m_instance);
	}

	void VulkanRenderer::BuildCommandBuffers(std::vector<MeshComponent*>& meshes)
	{
		IS_PROFILE_FUNCTION();

		VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();
		//cmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

		auto clearColors = m_frameBuffer.GetClearAttachments();
		VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
		renderPassBeginInfo.renderPass = m_frameBuffer.GetRenderPass() /*m_vulkanDevice->GetRenderPass()*/;
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width = m_frameBuffer.GetWidth();
		renderPassBeginInfo.renderArea.extent.height = m_frameBuffer.GetHeight();
		renderPassBeginInfo.clearValueCount = static_cast<U32>(clearColors.size());
		renderPassBeginInfo.pClearValues = clearColors.data();

		MVP mvp;
		mvp.proj = m_editorCamera->GetProjMatrix();
		mvp.proj[1][1] *= -1;
		mvp.view = m_editorCamera->GetViewMatrix();
		mvp.model = glm::mat4(1.0f);
		m_lightPos.x = 20 * glm::cos(m_lightPosAngle);
		m_lightPos.z = 20 * glm::sin(m_lightPosAngle);
		m_lightPosAngle += Time::GetDeltaTime() * 1.0f;
		mvp.lightPos = m_lightPos;

		auto updateMaterail = [&](MeshComponent* meshCom, Material* material, MaterialBlockData& materialBlockData)
		{
			IS_PROFILE_SCOPE("Uniform Update");
			auto vMat = dynamic_cast<VulkanMaterial*>(material);

			vMat->UploadUniform("UBO", &mvp, sizeof(MVP), materialBlockData);
			auto tc = meshCom->GetComponent<TransformComponent>();
			auto transform = tc->GetTransform();
			vMat->UploadUniform("MODELUBO", &transform, sizeof(glm::mat4), materialBlockData);
		};

		// TODO rework this. Really there should be a material manager with handles all the material lifetimes.
		{
			IS_PROFILE_SCOPE("Material reset");
			for (auto& mesh : meshes)
			{
				if (mesh)
				{
					for (auto& mat : mesh->GetMaterials())
					{
						if (mat)
						{
							mat->ResetUniformInfo();
						}
					}
				}
			}
		}

		vkResetCommandBuffer(m_frameBufferCmdBufferTest, 0);
		{
			// Set target frame buffer
			renderPassBeginInfo.framebuffer = m_frameBuffer.GetFrameBuffer();

			ThrowIfFailed(vkBeginCommandBuffer(m_frameBufferCmdBuffer, &cmdBufInfo));
			ThrowIfFailed(vkBeginCommandBuffer(m_frameBufferCmdBufferTest, &cmdBufInfo));

			vkCmdBeginRenderPass(m_frameBufferCmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBeginRenderPass(m_frameBufferCmdBufferTest, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			VkViewport viewport = vks::initializers::viewport(static_cast<float>(m_frameBuffer.GetWidth()), static_cast<float>(m_frameBuffer.GetHeight()), 0.0f, 1.0f);
			vkCmdSetViewport(m_frameBufferCmdBuffer, 0, 1, &viewport);
			vkCmdSetViewport(m_frameBufferCmdBufferTest, 0, 1, &viewport);

			VkRect2D scissor = vks::initializers::rect2D(m_frameBuffer.GetWidth(), m_frameBuffer.GetHeight(), 0, 0);
			vkCmdSetScissor(m_frameBufferCmdBuffer, 0, 1, &scissor);
			vkCmdSetScissor(m_frameBufferCmdBufferTest, 0, 1, &scissor);

			// This should be replaced by the mesh components in the scene.
			{
				IS_PROFILE_SCOPE("All Draws");
				for (auto& mesh : meshes)
				{
					if (mesh)
					{
						{
							IS_PROFILE_SCOPE("Single Draw");
							//mesh->Draw(m_frameBufferCmdBuffer, updateMaterail);
							//mesh->Draw(m_frameBufferCmdBufferTest, updateMaterail);
						}
					}
				}
			}

			vkCmdEndRenderPass(m_frameBufferCmdBuffer);
			vkCmdEndRenderPass(m_frameBufferCmdBufferTest);
			ThrowIfFailed(vkEndCommandBuffer(m_frameBufferCmdBuffer));
			ThrowIfFailed(vkEndCommandBuffer(m_frameBufferCmdBufferTest));
		}

		//vkResetCommandBuffer(m_frameBufferCmdBufferTest, 0);
		//{
		//	ThrowIfFailed(vkBeginCommandBuffer(m_frameBufferCmdBufferTest, &cmdBufInfo));

		//	vkCmdBeginRenderPass(m_frameBufferCmdBufferTest, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		//	VkViewport viewport = vks::initializers::viewport(static_cast<float>(m_frameBuffer.GetWidth()), static_cast<float>(m_frameBuffer.GetHeight()), 0.0f, 1.0f);
		//	vkCmdSetViewport(m_frameBufferCmdBufferTest, 0, 1, &viewport);

		//	VkRect2D scissor = vks::initializers::rect2D(m_frameBuffer.GetWidth(), m_frameBuffer.GetHeight(), 0, 0);
		//	vkCmdSetScissor(m_frameBufferCmdBufferTest, 0, 1, &scissor);

		//	// This should be replaced by the mesh components in the scene.
		//	{
		//		IS_PROFILE_SCOPE("All Draws");
		//		for (auto& mesh : meshes)
		//		{
		//			if (mesh)
		//			{
		//				{
		//					IS_PROFILE_SCOPE("Single Draw");
		//					mesh->Draw(m_frameBufferCmdBufferTest, updateMaterail);
		//				}
		//			}
		//		}
		//	}

		//	vkCmdEndRenderPass(m_frameBufferCmdBufferTest);
		//	ThrowIfFailed(vkEndCommandBuffer(m_frameBufferCmdBufferTest));
		//}
	}

	void VulkanRenderer::BuildPresentBuffers()
	{
		IS_PROFILE_FUNCTION();

		U32 width = Window::GetWidth();
		U32 height = Window::GetHeight();

		VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

		VkClearValue clearValues[2];
		clearValues[0].color = VkClearColorValue{ 1.0f, 0.15f, 0.15f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
		renderPassBeginInfo.renderPass = m_presentRenderPass;
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width = width;
		renderPassBeginInfo.renderArea.extent.height = height;
		renderPassBeginInfo.clearValueCount = 2;
		renderPassBeginInfo.pClearValues = clearValues;

		Insight::GraphicsAPI::Vulkan::VulkanImGUIRenderer* imgui = static_cast<Insight::GraphicsAPI::Vulkan::VulkanImGUIRenderer*>(ImGuiRenderer::Instance());
		imgui->EndFrame();

		int i = m_currentFrame;
		{
			// Set target frame buffer
			renderPassBeginInfo.framebuffer = m_presentFrameBuffers[i];

			ThrowIfFailed(vkBeginCommandBuffer(m_presentCmdBuffers[i], &cmdBufInfo));

			vkCmdBeginRenderPass(m_presentCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			VkViewport viewport = vks::initializers::viewport(static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f);
			vkCmdSetViewport(m_presentCmdBuffers[i], 0, 1, &viewport);

			VkRect2D scissor = vks::initializers::rect2D(width, height, 0, 0);
			vkCmdSetScissor(m_presentCmdBuffers[i], 0, 1, &scissor);

			dynamic_cast<vks::VulkanMaterial*>(m_presentMaterials[i])->Bind(m_presentCmdBuffers[i]);
			// DRAW
			vkCmdDraw(m_presentCmdBuffers[i], 3, 1, 0, 0);

			//imgui->Render(m_presentCmdBuffers[i]);

			vkCmdEndRenderPass(m_presentCmdBuffers[i]);
			ThrowIfFailed(vkEndCommandBuffer(m_presentCmdBuffers[i]));
		}
	}

	void VulkanRenderer::SetupDepthStencil()
	{
		IS_PROFILE_FUNCTION();
		VkImageCreateInfo imageCI{};
		imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCI.imageType = VK_IMAGE_TYPE_2D;
		imageCI.format = m_depthFormat;
		imageCI.extent = { (uint32_t)Window::GetWidth(), (uint32_t)Window::GetHeight(), 1 };
		imageCI.mipLevels = 1;
		imageCI.arrayLayers = 1;
		imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

		ThrowIfFailed(vkCreateImage(m_device, &imageCI, nullptr, &m_depthStencil.Image));
		VkMemoryRequirements memReqs{};
		vkGetImageMemoryRequirements(m_device, m_depthStencil.Image, &memReqs);

		VkMemoryAllocateInfo memAllloc{};
		memAllloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memAllloc.allocationSize = memReqs.size;
		memAllloc.memoryTypeIndex = m_vulkanDevice->GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		ThrowIfFailed(vkAllocateMemory(m_device, &memAllloc, nullptr, &m_depthStencil.Mem));
		ThrowIfFailed(vkBindImageMemory(m_device, m_depthStencil.Image, m_depthStencil.Mem, 0));

		VkImageViewCreateInfo imageViewCI{};
		imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCI.image = m_depthStencil.Image;
		imageViewCI.format = m_depthFormat;
		imageViewCI.subresourceRange.baseMipLevel = 0;
		imageViewCI.subresourceRange.levelCount = 1;
		imageViewCI.subresourceRange.baseArrayLayer = 0;
		imageViewCI.subresourceRange.layerCount = 1;
		imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		// Stencil aspect should only be set on depth + stencil formats (VK_FORMAT_D16_UNORM_S8_UINT..VK_FORMAT_D32_SFLOAT_S8_UINT
		if (m_depthFormat >= VK_FORMAT_D16_UNORM_S8_UINT)
		{
			imageViewCI.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
		ThrowIfFailed(vkCreateImageView(m_device, &imageViewCI, nullptr, &m_depthStencil.View));
	}

	void VulkanRenderer::SetupFrameBuffer()
	{
		IS_PROFILE_FUNCTION();
		VkImageView attachments[2];

		// Depth/Stencil attachment is the same for all frame buffers
		attachments[1] = m_depthStencil.View;

		VkFramebufferCreateInfo frameBufferCreateInfo = {};
		frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frameBufferCreateInfo.pNext = NULL;
		frameBufferCreateInfo.renderPass = m_presentRenderPass;
		frameBufferCreateInfo.attachmentCount = 2;
		frameBufferCreateInfo.pAttachments = attachments;
		frameBufferCreateInfo.width = (uint32_t)Window::GetWidth();
		frameBufferCreateInfo.height = (uint32_t)Window::GetHeight();
		frameBufferCreateInfo.layers = 1;

		// Create frame buffers for every swap chain image
		m_presentFrameBuffers.resize(m_swapchain.GetImageCount());
		for (uint32_t i = 0; i < m_presentFrameBuffers.size(); ++i)
		{
			attachments[0] = m_swapchain.GetImageView(i);
			ThrowIfFailed(vkCreateFramebuffer(m_device, &frameBufferCreateInfo, nullptr, &m_presentFrameBuffers[i]));
		}

		m_frameBuffer.SetRect((uint32_t)Window::GetWidth(), (uint32_t)Window::GetHeight());
		m_frameBuffer.CreateAttachment(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, "Color");
		m_frameBuffer.CreateAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, "Normal");
		m_frameBuffer.CreateAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, "Position");
		VkFormat attDepthFormat;
		vks::getSupportedDepthFormat(m_physicalDevice, &attDepthFormat);
		IS_CORE_ASSERT(attDepthFormat, "[VulkanRenderer::SetupFrameBuffer] Must have a valid depth format.");
		m_frameBuffer.CreateAttachment(attDepthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, "Depth");
		m_frameBuffer.CreateRenderPass();
	}

	void VulkanRenderer::SetupPresentRenderPass()
	{
		IS_PROFILE_FUNCTION();
		std::array<VkAttachmentDescription, 2> attachments = {};
		// Color attachment
		attachments[0].format = m_swapchain.GetColorFormat();
		attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		// Depth attachment
		attachments[1].format = m_depthFormat;
		attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorReference = {};
		colorReference.attachment = 0;
		colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthReference = {};
		depthReference.attachment = 1;
		depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpassDescription = {};
		subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescription.colorAttachmentCount = 1;
		subpassDescription.pColorAttachments = &colorReference;
		subpassDescription.pDepthStencilAttachment = &depthReference;
		subpassDescription.inputAttachmentCount = 0;
		subpassDescription.pInputAttachments = nullptr;
		subpassDescription.preserveAttachmentCount = 0;
		subpassDescription.pPreserveAttachments = nullptr;
		subpassDescription.pResolveAttachments = nullptr;

		// Subpass dependencies for layout transitions
		std::array<VkSubpassDependency, 2> dependencies;

		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpassDescription;
		renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
		renderPassInfo.pDependencies = dependencies.data();

		ThrowIfFailed(vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_presentRenderPass));
	}

	void VulkanRenderer::GetEnabledFeatures()
	{
		m_enabledFeatures.samplerAnisotropy = VK_TRUE;
	}

	VkSubmitInfo VulkanRenderer::GetSubmitInfo()
	{
		// Set up submit info structure
		// Semaphores will stay the same during application lifetime
		// Command buffer submission info is set by each example
		VkSubmitInfo submitInfo = vks::initializers::submitInfo();

		submitInfo.pWaitDstStageMask = &m_submitPipelineStages;

		VkSemaphore waitSemahores[] = { m_semaphores.ImageAquired[0] };
		submitInfo.pWaitSemaphores = waitSemahores;
		submitInfo.waitSemaphoreCount = 1;

		VkSemaphore signalSemahores[] = { m_semaphores.RenderComplete[0] };
		submitInfo.pSignalSemaphores = signalSemahores;
		submitInfo.signalSemaphoreCount = 1;

		return submitInfo;
	}

	float VulkanRenderer::rnd(float range)
	{
		//std::uniform_real_distribution<float> rndDist(0.0f, range);
		//return rndDist(m_rndEngine);
		return 0.f;
	}

	void VulkanRenderer::PrepareMultiThreadedRenderer()
	{
		IS_PROFILE_FUNCTION();
		// Since this demo updates the command buffers on each frame
		// we don't use the per-framebuffer command buffers from the
		// base class, and create a single primary command buffer instead
		VkCommandBufferAllocateInfo cmdBufAllocateInfo =
			vks::initializers::commandBufferAllocateInfo(
				m_cmdPool,
				VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				1);
		ThrowIfFailed(vkAllocateCommandBuffers(m_device, &cmdBufAllocateInfo, &m_primaryCommandBuffer));

		// Create additional secondary CBs for background and ui
		cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
		ThrowIfFailed(vkAllocateCommandBuffers(m_device, &cmdBufAllocateInfo, &m_secondaryCommandBuffers.UI));

		m_threadData.resize(m_numThreads);

		float maxX = (float)std::floor(std::sqrt((double)m_numThreads * (double)m_numObjectsPerThread));
		uint32_t posX = 0;
		uint32_t posZ = 0;

		for (uint32_t i = 0; i < m_numThreads; i++)
		{
			ThreadData* thread = &m_threadData[i];

			// Create one command pool for each thread
			VkCommandPoolCreateInfo cmdPoolInfo = vks::initializers::commandPoolCreateInfo();
			cmdPoolInfo.queueFamilyIndex = m_swapchain.GetQueuNodeIndex();
			cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			ThrowIfFailed(vkCreateCommandPool(m_device, &cmdPoolInfo, nullptr, &thread->commandPool));

			// One secondary command buffer per object that is updated by this thread
			thread->commandBuffer.resize(m_numObjectsPerThread);
			// Generate secondary command buffers for each thread
			VkCommandBufferAllocateInfo secondaryCmdBufAllocateInfo =
				vks::initializers::commandBufferAllocateInfo(
					thread->commandPool,
					VK_COMMAND_BUFFER_LEVEL_SECONDARY,
					static_cast<uint32_t>(thread->commandBuffer.size()));
			ThrowIfFailed(vkAllocateCommandBuffers(m_device, &secondaryCmdBufAllocateInfo, thread->commandBuffer.data()));

			thread->pushConstBlock.resize(m_numObjectsPerThread);
			thread->objectData.resize(m_numObjectsPerThread);

			for (uint32_t j = 0; j < m_numObjectsPerThread; j++)
			{
				float theta = 2.0f * glm::pi<float>() * rnd(1.0f);
				float phi = acos(1.0f - 2.0f * rnd(1.0f));
				thread->objectData[j].pos = glm::vec3(sin(phi) * cos(theta), 0.0f, cos(phi)) * 35.0f;

				thread->objectData[j].rotation = glm::vec3(0.0f, rnd(360.0f), 0.0f);
				thread->objectData[j].deltaT = rnd(1.0f);
				thread->objectData[j].rotationDir = (rnd(100.0f) < 50.0f) ? 1.0f : -1.0f;
				thread->objectData[j].rotationSpeed = (2.0f + rnd(4.0f)) * thread->objectData[j].rotationDir;
				thread->objectData[j].scale = 0.75f + rnd(0.5f);

				thread->pushConstBlock[j].color = glm::vec3(rnd(1.0f), rnd(1.0f), rnd(1.0f));
			}
		}
	}

	void VulkanRenderer::UpdateSecondaryCommandBuffers(VkCommandBufferInheritanceInfo inheritanceInfo)
	{
		IS_PROFILE_FUNCTION();
		// Secondary command buffer for the sky sphere
		VkCommandBufferBeginInfo commandBufferBeginInfo = vks::initializers::commandBufferBeginInfo();
		commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
		commandBufferBeginInfo.pInheritanceInfo = &inheritanceInfo;

		/*
			User interface
			With VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS, the primary command buffer's content has to be defined
			by secondary command buffers, which also applies to the UI overlay command buffer
		*/

		uint32_t width = (uint32_t)Window::GetWidth();
		uint32_t height = (uint32_t)Window::GetHeight();

		ThrowIfFailed(vkBeginCommandBuffer(m_secondaryCommandBuffers.UI, &commandBufferBeginInfo));

		VkViewport viewport = vks::initializers::viewport((float)width, (float)height, 0.0f, 1.0f);
		vkCmdSetViewport(m_secondaryCommandBuffers.UI, 0, 1, &viewport);

		VkRect2D scissor = vks::initializers::rect2D(width, height, 0, 0);
		vkCmdSetScissor(m_secondaryCommandBuffers.UI, 0, 1, &scissor);

		//vkCmdBindPipeline(m_secondaryCommandBuffers.UI, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.starsphere);

		Insight::GraphicsAPI::Vulkan::VulkanImGUIRenderer* imgui = static_cast<Insight::GraphicsAPI::Vulkan::VulkanImGUIRenderer*>(ImGuiRenderer::Instance());
		imgui->EndFrame();
		//imgui->Render(m_secondaryCommandBuffers.UI);

		ThrowIfFailed(vkEndCommandBuffer(m_secondaryCommandBuffers.UI));
	}

	void VulkanRenderer::UpdateCommandBuffer(VkFramebuffer frameBuffer)
	{
		IS_PROFILE_FUNCTION();
		// Contains the list of secondary command buffers to be submitted
		std::vector<VkCommandBuffer> commandBuffers;

		uint32_t width = (uint32_t)Window::GetWidth();
		uint32_t height = (uint32_t)Window::GetHeight();

		VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

		VkClearValue clearValues[2];
		clearValues[0].color = m_clearColor;
		clearValues[1].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
		renderPassBeginInfo.renderPass = m_vulkanDevice->GetRenderPass();
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width = width;
		renderPassBeginInfo.renderArea.extent.height = height;
		renderPassBeginInfo.clearValueCount = 2;
		renderPassBeginInfo.pClearValues = clearValues;
		renderPassBeginInfo.framebuffer = frameBuffer;

		// Set target frame buffer

		ThrowIfFailed(vkBeginCommandBuffer(m_primaryCommandBuffer, &cmdBufInfo));

		// The primary command buffer does not contain any rendering commands
		// These are stored (and retrieved) from the secondary command buffers
		vkCmdBeginRenderPass(m_primaryCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

		// Inheritance info for the secondary command buffers
		VkCommandBufferInheritanceInfo inheritanceInfo = vks::initializers::commandBufferInheritanceInfo();
		inheritanceInfo.renderPass = m_vulkanDevice->GetRenderPass();
		// Secondary command buffer also use the currently active framebuffer
		inheritanceInfo.framebuffer = frameBuffer;

		// Update secondary sene command buffers
		UpdateSecondaryCommandBuffers(inheritanceInfo);

		// Add a job to the thread's queue for each object to be rendered
		for (uint32_t t = 0; t < m_numThreads; t++)
		{
			for (uint32_t i = 0; i < m_numObjectsPerThread; i++)
			{
				if (m_threadData[t].objectData[i].visible)
				{
					m_threadPool.push([=] { ThreadRenderCode(t, i, inheritanceInfo); });
				}
			}
		}

		{
			IS_PROFILE_SCOPE("WAIT FOR ALL THREADS");
			m_threadPool.join();
		}

		// Only submit if object is within the current view frustum
		for (uint32_t t = 0; t < m_numThreads; t++)
		{
			for (uint32_t i = 0; i < m_numObjectsPerThread; i++)
			{
				if (m_threadData[t].objectData[i].visible)
				{
					commandBuffers.push_back(m_threadData[t].commandBuffer[i]);
				}
			}
		}

		commandBuffers.push_back(m_secondaryCommandBuffers.UI);

		// Execute render commands from the secondary command buffer
		vkCmdExecuteCommands(m_primaryCommandBuffer, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

		vkCmdEndRenderPass(m_primaryCommandBuffer);

		ThrowIfFailed(vkEndCommandBuffer(m_primaryCommandBuffer));
	}

	void VulkanRenderer::ThreadRenderCode(int32_t threadIndex, uint32_t cmdBufferIndex, VkCommandBufferInheritanceInfo inheritanceInfo)
	{
		IS_PROFILE_FUNCTION();
		ThreadData* thread = &m_threadData[threadIndex];
		ObjectData* objectData = &thread->objectData[cmdBufferIndex];

		// Check visibility against view frustum using a simple sphere check based on the radius of the mesh
		//objectData->visible = frustum.checkSphere(objectData->pos, models.ufo.dimensions.radius * 0.5f);

		if (!objectData->visible)
		{
			return;
		}

		uint32_t width = (uint32_t)Window::GetWidth();
		uint32_t height = (uint32_t)Window::GetHeight();

		VkCommandBufferBeginInfo commandBufferBeginInfo = vks::initializers::commandBufferBeginInfo();
		commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
		commandBufferBeginInfo.pInheritanceInfo = &inheritanceInfo;

		VkCommandBuffer cmdBuffer = thread->commandBuffer[cmdBufferIndex];

		ThrowIfFailed(vkBeginCommandBuffer(cmdBuffer, &commandBufferBeginInfo));

		VkViewport viewport = vks::initializers::viewport((float)width, (float)height, 0.0f, 1.0f);
		vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);

		VkRect2D scissor = vks::initializers::rect2D(width, height, 0, 0);
		vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

		// Fill in models data or have a render function ptr to 
		// a render function for objects in a model class.

		ThrowIfFailed(vkEndCommandBuffer(cmdBuffer));
	}

	void VulkanRenderer::OnWindowResizeEvent(const Event& event)
	{
		IS_PROFILE_FUNCTION();
		WindowResizeEvent resizeEvent = static_cast<const WindowResizeEvent&>(event);
		IS_CORE_INFO("{0}", resizeEvent.ToString());

		m_vulkanDevice->WaitForIdle();

		int width = 0, height = 0;
		glfwGetFramebufferSize(Window::m_window, &width, &height);
		while (width == 0 && height == 0)
		{
			glfwGetFramebufferSize(Window::m_window, &width, &height);
			glfwWaitEvents();
		}

		SetupSwapchain();

		// Recreate the frame buffers
		vkDestroyImageView(m_device, m_depthStencil.View, nullptr);
		vkDestroyImage(m_device, m_depthStencil.Image, nullptr);
		vkFreeMemory(m_device, m_depthStencil.Mem, nullptr);
		SetupDepthStencil();
		for (uint32_t i = 0; i < m_presentFrameBuffers.size(); i++)
		{
			vkDestroyFramebuffer(m_device, m_presentFrameBuffers[i], nullptr);
		}
		SetupFrameBuffer();

		// Command buffers need to be recreated as they may store
		// references to the recreated frame buffer
		DestroyCommandBuffers();
		CreateCommandBuffers();

		EVENT_DISPATCH(EventType::VulkanWindowResize, VulkanResizeEvent(Window::GetWidth(), Window::GetHeight()));
	}
}