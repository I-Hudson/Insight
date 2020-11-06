#include "ispch.h"
#ifdef IS_VULKAN
#include "Swapchain.h"
#include "Device.h"
#include "Queue.h"
#include "VulkanBuffers.h"
#include "VulkanMaterial.h"

#include "Insight/Instrumentor/Instrumentor.h"
#include "Insight/Library/ShaderLibrary.h"
#include "Insight/Renderer/ShaderModule.h"
#include "Insight/Module/WindowModule.h"
#include "Insight/Event/EventManager.h"

#include "glm/glm.hpp"
#include "Insight/Renderer/ImGuiRenderer.h"
#include "examples/imgui_impl_vulkan.h"

namespace Platform
{
	Swapchain::Swapchain(const Device* device)
		: m_device(const_cast<Device*>(device))
	{
		Insight::EventManager::Bind(Insight::EventType::WindowResize, typeid(Swapchain).name(), BIND_FUNC(Swapchain::RecreateSwapchain, this));

		CreateSwapChain();

		int width = Insight::Module::WindowModule::GetWindow()->GetWidth();
		int height = Insight::Module::WindowModule::GetWindow()->GetHeight();
		glm::ivec2 extent = glm::ivec2(width, height);

#ifdef IS_EDITOR
		std::vector<std::string> shaderPaths = { "vulkan/editor/swapchain_shader.vert", "vulkan/editor/swapchain_shader.frag" };
		m_swapchainShader = NEW_ON_HEAP(VulkanShader, m_device, shaderPaths, extent, m_swapchainFramebuffers[0]->GetRenderpass());
#else
		std::vector<std::string> shaderPaths = { "vulkan/swapchain_shader.vert", "vulkan/swapchain_shader.frag" };
		m_swapchainShader = NEW_ON_HEAP(VulkanShader, m_device, shaderPaths,
			extent, m_swapchainFramebuffers[0]->GetRenderpass());
#endif

		
		Insight::Library::ShaderLibrary::GetInstance()->AddAsset(m_swapchainShader->GetUUID(), m_swapchainShader);

		for (int i = 0; i < 3; i++)
		{
			VulkanMaterial* mat = static_cast<VulkanMaterial*>(Material::Create());
			mat->SetShader(m_swapchainShader);
			m_materials.push_back(mat);
		}

		m_drawCommandPool = NEW_ON_HEAP(CommandPool, m_device, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
		m_drawCommandBuffers = m_drawCommandPool->AllocCommandBuffers(3);
		m_imguiCommandBuffers = m_drawCommandPool->AllocCommandBuffers(3);

		// Position				  // Colour				   // Normal				//UV1
		std::vector<Vertex> vertices =
		{
			{{-1.0f, -1.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f,}},
			{{-1.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 1.0f,}},
			{{1.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {1.0f, 1.0f,}},
			{{1.0f, -1.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {1.0f, 0.0f,}},
		};

		std::vector<unsigned int> indices =
		{
			0,1,2, 0,2,3
		};
		m_fullscreenQuad = NEW_ON_HEAP(Mesh, vertices, indices, std::vector<Texture>(), 0, "", "");

		PrepSwapchainCommandBuffer();

#ifdef IS_EDITOR
		//m_editorFrameBuffer = NEW_ON_HEAP(VulkanFramebuffer, m_swapchainSettings.Device, m_swapchainSettings.Window->GetWidth(), m_swapchainSettings.Window->GetHeight());
		//m_editorFrameBuffer->CreateAttachment(VK_FORMAT_B8G8R8A8_SRGB, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
		//	VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		//m_editorFrameBuffer->CompileFrameBuffer();
		//
		//m_editorCommandPool = NEW_ON_HEAP(CommandPool, m_swapchainSettings.Device, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
		//m_editorCommandBuffer = m_editorCommandPool->AllocCommandBuffer();
#endif

		/*m_frameGraph.Create();
		m_frameGraph.PrintToConsole();

		m_frameGraph.AddNode(nullptr, [&]()
			{
				auto dcb = m_drawCommandBuffers[0];
				dcb->StartRecord();
			}, m_drawCommandBuffers[0], "SwapChain", "Start command buffer Record");

		m_frameGraph.AddNode(m_drawCommandBuffers[0], [&]()
			{
			}, this, "SwapChain", "Bind framebuffer");

		m_frameGraph.AddNode(m_drawCommandBuffers[0], [&]()
			{
			}, this, "SwapChain", "Update texture");

		m_frameGraph.AddNode(m_drawCommandBuffers[0], [&]()
			{
			}, this, "SwapChain", "Render a single model");

		m_frameGraph.AddNode(m_drawCommandBuffers[0], [&]()
			{
			}, this, "SwapChain", "Render a single model");

		m_frameGraph.AddNode(nullptr, [&]()
			{
				auto dcb = m_drawCommandBuffers[0];
				dcb->EndRecord();
			}, m_drawCommandBuffers[0], "SwapChain", false, "Stop command buffer Record");

		m_frameGraph.PrintToConsole();

		m_frameGraph.Execute();*/

		IS_CORE_INFO("SwapChain completed.");
	}

	Swapchain::~Swapchain()
	{
		Insight::EventManager::Unbind(Insight::EventType::WindowResize, typeid(Swapchain).name());

		DELETE_ON_HEAP(m_fullscreenQuad);
		DELETE_ON_HEAP(m_drawCommandPool);

		for (auto it = m_materials.begin(); it != m_materials.end(); ++it)
		{
			DELETE_ON_HEAP(*it);
		}
		for (size_t i = 0; i < m_swapchainFramebuffers.size(); i++)
		{
			DELETE_ON_HEAP(m_swapchainFramebuffers[i]);
		}
		vkDestroySwapchainKHR(m_device->GetDevice(), m_swapchain, nullptr);
	}

	SwapChainSupportDetails Swapchain::QuerySwapChainSupport(VkPhysicalDevice device)
	{
		SwapChainSupportDetails details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_device->GetSurface(), &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_device->GetSurface(), &formatCount, nullptr);
		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_device->GetSurface(), &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_device->GetSurface(), &presentModeCount, nullptr);
		if (presentModeCount != 0)
		{
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_device->GetSurface(), &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	VkSurfaceFormatKHR Swapchain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats)
	{
		for (const auto& availableFormat : formats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return availableFormat;
			}
		}

		return formats[0];
	}

	VkPresentModeKHR Swapchain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& presentModes)
	{
		for (const auto& availablePresentMode : presentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return availablePresentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D Swapchain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != UINT32_MAX)
		{
			return capabilities.currentExtent;
		}
		else
		{
			uint32_t width = static_cast<uint32_t>(Insight::Module::WindowModule::GetWindow()->GetWidth());
			uint32_t height = static_cast<uint32_t>(Insight::Module::WindowModule::GetWindow()->GetHeight());
			VkExtent2D actualExtent = { width, height };

			actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
			actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

			return actualExtent;
		}
	}

	void Swapchain::AcquireNextImage()
	{
		IS_PROFILE_FUNCTION();

		m_inFlightFences[m_currentFrame]->Wait();

		uint32_t prevIndex = m_imageIndex;
		VkResult result = vkAcquireNextImageKHR(m_device->GetDevice(), m_swapchain, Insight::U64_MAX,
			m_swapchainFramebuffers[m_currentFrame]->GetAvailbleSem()->GetSemaphore(),
			VK_NULL_HANDLE, &m_imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		{
			Insight::EventManager::Dispatch(Insight::EventType::WindowResize, Insight::WindowResizeEvent::WindowResizeEvent(640, 480));
		}

		if (m_imagesInFlight[m_imageIndex] != VK_NULL_HANDLE)
		{
			m_imagesInFlight[m_imageIndex]->Wait();
		}
	}

	void Swapchain::Submit(Semaphore* waitSemaphore)
	{
		IS_PROFILE_FUNCTION();

		m_imagesInFlight[m_imageIndex] = m_inFlightFences[m_currentFrame];

		std::vector<VkSemaphore> waitSemaphores = { waitSemaphore->GetSemaphore() };
		std::vector<VkPipelineStageFlags> stageFlags = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		std::vector<VkCommandBuffer> commandBuffers = { m_drawCommandBuffers[m_imageIndex]->GetBuffer(), m_imguiCommandBuffers[m_imageIndex]->GetBuffer() };
		std::vector<VkSemaphore> signalSemaphore = { m_swapchainFramebuffers[m_currentFrame]->GetFinishedSem()->GetSemaphore() };
		VkSubmitInfo submitInfo = VulkanInits::SubmitInfo(waitSemaphores, stageFlags, commandBuffers, signalSemaphore);

		m_inFlightFences[m_currentFrame]->Reset();

		m_device->GetQueue(QueueFamilyType::Graphics).Submit(submitInfo, m_inFlightFences[m_currentFrame]->GetFence());

	}

	void Swapchain::Draw(Semaphore* waitSemaphore, VulkanFramebuffer* offscreenFB)
	{
		IS_PROFILE_FUNCTION();

#if defined(IS_EDITOR) && defined(IMGUI_ENABLED)
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		ImGui::Begin("ViewPort");
		ImVec2 viewPortSize = ImGui::GetContentRegionAvail();
		if (m_sceneTexture == nullptr)
		{
			m_sceneTexture = (ImTextureID)ImGui_ImplVulkan_AddTexture(*offscreenFB->GetSampler(), offscreenFB->GetAttachment(0).View, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
		ImGui::Image(m_sceneTexture, viewPortSize);
		ImGui::End();
		ImGui::PopStyleVar();
#endif

		ImGuiRenderer::GetInstance()->EndFrame();

		m_imguiCommandBuffers[m_currentFrame]->StartRecord(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		m_swapchainFramebuffers[m_currentFrame]->BindBuffer(m_imguiCommandBuffers[m_currentFrame]);
		m_materials[m_currentFrame]->Bind(m_imguiCommandBuffers[m_currentFrame], nullptr);
		ImGuiRenderer::GetInstance()->Render(m_imguiCommandBuffers[m_currentFrame]);
		m_swapchainFramebuffers[m_currentFrame]->UnbindBuffer(m_imguiCommandBuffers[m_currentFrame]);
		m_imguiCommandBuffers[m_currentFrame]->EndRecord();

		Submit(waitSemaphore);
	}

	void Swapchain::Present()
	{
		IS_PROFILE_FUNCTION();

		std::vector<VkSemaphore> signalSemaphore = { m_swapchainFramebuffers[m_currentFrame]->GetFinishedSem()->GetSemaphore() };
		std::vector<VkSwapchainKHR>swapchains = { m_swapchain };
		VkPresentInfoKHR presentInfo = VulkanInits::PresnetInfo(signalSemaphore, swapchains, m_imageIndex);
		m_device->GetQueue(QueueFamilyType::Present).Presnet(presentInfo);
		m_device->GetQueue(QueueFamilyType::Present).Wait();

		m_currentFrame = (m_currentFrame + 1) % MaxFramesInFlight;
	}

	Semaphore* Swapchain::GetAcquireNextImageSemaphore()
	{
		return m_swapchainFramebuffers[m_currentFrame]->GetAvailbleSem();
	}

	void Swapchain::CreateSwapChain()
	{
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_device->GetPhysicalDevice());

		VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
		{
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo = VulkanInits::SwapChainInfo();
		createInfo.surface = m_device->GetSurface();
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		std::vector<uint32_t> queueFamilyIndices
		{
			m_device->GetQueueFamily(QueueFamilyType::Graphics).GetValue(),
			m_device->GetQueueFamily(QueueFamilyType::Present).GetValue()
		};

		if (m_device->GetQueueFamily(QueueFamilyType::Graphics).GetValue() !=
			m_device->GetQueueFamily(QueueFamilyType::Present).GetValue())
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0; // Optional
			createInfo.pQueueFamilyIndices = nullptr; // Optional
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		ThrowIfFailed(vkCreateSwapchainKHR(m_device->GetDevice(), &createInfo, nullptr, &m_swapchain));

		imageCount = 0;
		std::vector<VkImage> swapChainImages;
		vkGetSwapchainImagesKHR(m_device->GetDevice(), m_swapchain, &imageCount, nullptr);
		swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(m_device->GetDevice(), m_swapchain, &imageCount, swapChainImages.data());

		IS_CORE_INFO("Swapchain images completed.");

		m_inFlightFences.resize(imageCount);
		m_imagesInFlight.resize(imageCount);

		for (size_t i = 0; i < imageCount; i++)
		{
			VulkanFramebuffer* fb = NEW_ON_HEAP(VulkanFramebuffer, m_device, extent);
			fb->AttachImage(&swapChainImages[i], VK_FORMAT_B8G8R8A8_SRGB, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
			fb->CompileFrameBuffer();
			m_swapchainFramebuffers.push_back(std::move(fb));

			m_inFlightFences[i] = fb->GetFence();
			m_imagesInFlight[i] = VK_NULL_HANDLE;
		}
	}


	const VkSwapchainKHR& Swapchain::GetSwapchain() const
	{
		return m_swapchain;
	}

	void Swapchain::RecreateSwapchain(const Insight::Event& event)
	{
		Insight::WindowResizeEvent e = static_cast<const Insight::WindowResizeEvent&>(event);
		unsigned int width = e.m_width;
		unsigned int height = e.m_height;

		while (width == 0 || height == 0)
		{
			width = Insight::Module::WindowModule::GetWindow()->GetWidth();
			height = Insight::Module::WindowModule::GetWindow()->GetHeight();
			Insight::Module::WindowModule::GetWindow()->WaitForEvents();
		}

		m_drawCommandPool->FreeCommandBuffers();
		m_drawCommandBuffers = m_drawCommandPool->AllocCommandBuffers(3);
		m_imguiCommandBuffers = m_drawCommandPool->AllocCommandBuffers(3);

		for (size_t i = 0; i < m_swapchainFramebuffers.size(); ++i)
		{
			DELETE_ON_HEAP(m_swapchainFramebuffers[i]);
		}
		m_swapchainFramebuffers.clear();
		vkDestroySwapchainKHR(m_device->GetDevice(), m_swapchain, nullptr);
		CreateSwapChain();

		m_swapchainShader->Resize(width, height);
		for (size_t i = 0; i < m_materials.size(); ++i)
		{
			m_materials[i]->Resize();
		}

#if defined(IS_EDITOR) && defined(IMGUI_ENABLED)
		m_sceneTexture = nullptr;
#endif
	}

	void Swapchain::PrepSwapchainCommandBuffer()
	{
		for (size_t i = 0; i < m_drawCommandBuffers.size(); ++i)
		{
			{
				IS_PROFILE_SCOPE("Swapchain Draw");
				auto dcb = m_drawCommandBuffers[i];

				dcb->StartRecord();

				m_swapchainFramebuffers[i]->BindBuffer(dcb);
				m_materials[m_currentFrame]->Bind(dcb, nullptr);

#ifndef IS_EDITOR
				if (offscreenFB != nullptr)
				{
					m_materials[i]->UpdateSampler2D("OffScreenTexture", &offscreenFB->GetAttachment(0).View, offscreenFB->GetSampler(), 0);
				}
				m_materials[i]->SetUniforms();
#endif

				VkBuffer vertexBuffers[] = { static_cast<VulkanVertexBuffer*>(m_fullscreenQuad->GetVertexBuffer())->GetBuffer() };
				VkDeviceSize offsets[] = { 0 };
				vkCmdBindVertexBuffers(dcb->GetBuffer(), 0, 1, vertexBuffers, offsets);

				vkCmdBindIndexBuffer(dcb->GetBuffer(), static_cast<VulkanIndexBuffer*>(m_fullscreenQuad->GetIndexBuffer())->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

				vkCmdDrawIndexed(dcb->GetBuffer(), static_cast<uint32_t>(m_fullscreenQuad->GetIndicesCount()), 1, 0, 0, 0);

				m_swapchainFramebuffers[i]->UnbindBuffer(dcb);

				dcb->EndRecord();
			}
		}
	}
}
#endif