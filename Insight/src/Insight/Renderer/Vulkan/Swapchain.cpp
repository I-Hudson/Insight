#include "ispch.h"
#ifdef IS_VULKAN
#include "Insight/Renderer/Vulkan/Vulkan.h"
#include "Insight/Renderer/Vulkan/Swapchain.h"
#include "Insight/Renderer/Vulkan/Device.h"
#include "Insight/Renderer/Vulkan/Queue.h"

#include "Insight/Renderer/ShaderModule.h"
#include "Insight/Renderer/Vulkan/VulkanBuffers.h"
#include "Insight/Renderer/Vulkan/VulkanMaterial.h"
#include "Insight/Renderer/Vulkan/VulkanBuffers.h"

#include "Insight/Module/WindowModule.h"
#include "Insight/Event/EventManager.h"

namespace Insight
{
	namespace Render
	{
		Swapchain::Swapchain(const SwapchainSettings swapchainSettings)
			: m_swapchainSettings(swapchainSettings)
		{
			EventManager::Bind(EventType::WindowResize, typeid(Swapchain).name(), BIND_FUNC(Swapchain::RecreateSwapchain, this));

			CreateSwapChain();

			ShaderData data
			{
				m_swapchainSettings.Device,
				{"vulkan/swapchain_shader.vert", "vulkan/swapchain_shader.frag"},
				VkExtent2D{ static_cast<uint32_t>(m_swapchainSettings.Window->GetWidth()), static_cast<uint32_t>(m_swapchainSettings.Window->GetHeight())},
				m_swapchainFramebuffers[0]->GetRenderpass()
			};
			m_swapchainShader = Memory::MemoryManager::NewOnFreeList<VulkanShader>(data);

			for (int i = 0; i < 3; i++)
			{
				Material* mat = Material::Create();
				mat->SetShader(m_swapchainShader);
				m_materials.push_back(mat);
			}

			m_drawCommandPool = Memory::MemoryManager::NewOnFreeList<CommandPool>(m_swapchainSettings.Device, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
			m_drawCommandBuffers = m_drawCommandPool->AllocCommandBuffers(3);

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
			m_fullscreenQuad = Memory::MemoryManager::NewOnFreeList<Mesh>(vertices, indices, std::vector<Texture>());

			IS_CORE_INFO("SwapChain completed.");
		}

		Swapchain::~Swapchain()
		{
			EventManager::Unbind(EventType::WindowResize, typeid(Swapchain).name());

			Memory::MemoryManager::DeleteOnFreeList<Mesh>(m_fullscreenQuad);
			Memory::MemoryManager::DeleteOnFreeList(m_drawCommandPool);
			Memory::MemoryManager::DeleteOnFreeList(m_swapchainShader);

			for (auto it = m_materials.begin(); it != m_materials.end(); ++it)
			{
				Memory::MemoryManager::DeleteOnFreeList(*it);
			}
			for (size_t i = 0; i < m_swapchainFramebuffers.size(); i++)
			{
				Memory::MemoryManager::DeleteOnFreeList(m_swapchainFramebuffers[i]);
			}
			vkDestroySwapchainKHR(m_swapchainSettings.Device->GetDevice(), m_swapchain, nullptr);
		}

		SwapChainSupportDetails Swapchain::QuerySwapChainSupport(VkPhysicalDevice device)
		{
			SwapChainSupportDetails details;
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_swapchainSettings.Device->GetSurface(), &details.capabilities);

			uint32_t formatCount;
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_swapchainSettings.Device->GetSurface(), &formatCount, nullptr);
			if (formatCount != 0)
			{
				details.formats.resize(formatCount);
				vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_swapchainSettings.Device->GetSurface(), &formatCount, details.formats.data());
			}

			uint32_t presentModeCount;
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_swapchainSettings.Device->GetSurface(), &presentModeCount, nullptr);
			if (presentModeCount != 0)
			{
				details.presentModes.resize(presentModeCount);
				vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_swapchainSettings.Device->GetSurface(), &presentModeCount, details.presentModes.data());
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
				uint32_t width = static_cast<uint32_t>(m_swapchainSettings.Window->GetWidth());
				uint32_t height = static_cast<uint32_t>(m_swapchainSettings.Window->GetHeight());
				VkExtent2D actualExtent = { width, height };

				actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
				actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

				return actualExtent;
			}
		}

		void Swapchain::AcquireNextImage()
		{
			m_inFlightFences[m_currentFrame]->Wait();

			uint32_t prevIndex = m_imageIndex;
			VkResult result = vkAcquireNextImageKHR(m_swapchainSettings.Device->GetDevice(), m_swapchain, U64_MAX, 
													m_swapchainFramebuffers[m_currentFrame]->GetAvailbleSem()->GetSemaphore(),
													VK_NULL_HANDLE, &m_imageIndex);

			if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
			{
				EventManager::Dispatch(EventType::WindowResize, WindowResizeEvent(640, 480));
			}

			if (m_imagesInFlight[m_imageIndex] != VK_NULL_HANDLE)
			{
				m_imagesInFlight[m_imageIndex]->Wait();
			}
		}

		void Swapchain::Submit(Semaphore* waitSemaphore)
		{
			m_imagesInFlight[m_imageIndex] = m_inFlightFences[m_currentFrame];

			std::vector<VkSemaphore> waitSemaphores = { waitSemaphore->GetSemaphore() };
			std::vector<VkPipelineStageFlags> stageFlags = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
			std::vector<VkCommandBuffer> commandBuffers = { m_drawCommandBuffers[m_imageIndex]->GetBuffer() };
			std::vector<VkSemaphore> signalSemaphore = { m_swapchainFramebuffers[m_currentFrame]->GetFinishedSem()->GetSemaphore() };
			VkSubmitInfo submitInfo = VulkanInits::SubmitInfo(waitSemaphores, stageFlags, commandBuffers, signalSemaphore);

			m_inFlightFences[m_currentFrame]->Reset();

			GraphicsQueueInfo info;
			info.SubmitInfo = &submitInfo;
			info.SyncFence = m_inFlightFences[m_currentFrame];
			m_swapchainSettings.Device->GetQueue(QueueFamilyType::Graphics).Submit(info);
		}

		void Swapchain::Draw(Semaphore* waitSemaphore, VulkanFramebuffer* offscreenFB)
		{
			if (offscreenFB != nullptr)
			{
				m_materials[m_imageIndex]->UpdateSampler2D("OffScreenTexture", &offscreenFB->GetAttachment(0).View, offscreenFB->GetSampler(), 0);
			}
			m_materials[m_imageIndex]->SetUniforms();

			int i = 0;
			++tempShader;
			for (auto it = m_drawCommandBuffers.begin(); it != m_drawCommandBuffers.end(); ++it)
			{
				(*it)->StartRecord();
				m_swapchainFramebuffers[i]->BindBuffer(*it);
				static_cast<VulkanMaterial*>(m_materials[i])->Bind(*it);

				VkBuffer vertexBuffers[] = { static_cast<VulkanVertexBuffer*>(m_fullscreenQuad->GetVertexBuffer())->GetBuffer() };
				VkDeviceSize offsets[] = { 0 };
				vkCmdBindVertexBuffers((*it)->GetBuffer(), 0, 1, vertexBuffers, offsets);

				vkCmdBindIndexBuffer((*it)->GetBuffer(), static_cast<VulkanIndexBuffer*>(m_fullscreenQuad->GetIndexBuffer())->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

				vkCmdDrawIndexed((*it)->GetBuffer(), static_cast<uint32_t>(m_fullscreenQuad->GetIndicesCount()), 1, 0, 0, 0);
				
				m_swapchainFramebuffers[i]->UnbindBuffer(*it);
			
				(*it)->EndRecord();
				i++;
			}
			Submit(waitSemaphore);
		}

		void Swapchain::Present()
		{
			std::vector<VkSemaphore> signalSemaphore = { m_swapchainFramebuffers[m_currentFrame]->GetFinishedSem()->GetSemaphore() };
			std::vector<VkSwapchainKHR>swapchains = { m_swapchain };
			VkPresentInfoKHR presentInfo = VulkanInits::PresnetInfo(signalSemaphore, swapchains, m_imageIndex);
			PresentQueueInfo presentQueueInfo;
			presentQueueInfo.PresentInfo = &presentInfo;
			m_swapchainSettings.Device->GetQueue(QueueFamilyType::Present).Submit(presentQueueInfo);
			m_swapchainSettings.Device->GetQueue(QueueFamilyType::Present).Wait();

			m_currentFrame = (m_currentFrame + 1) % MaxFramesInFlight;
		}

		Semaphore* Swapchain::GetAcquireNextImageSemaphore()
		{
			return m_swapchainFramebuffers[m_currentFrame]->GetAvailbleSem();
		}

		void Swapchain::CreateSwapChain()
		{
			SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_swapchainSettings.Device->GetPhysicalDevice());

			VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
			VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
			VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

			uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
			if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
			{
				imageCount = swapChainSupport.capabilities.maxImageCount;
			}

			VkSwapchainCreateInfoKHR createInfo = VulkanInits::SwapChainInfo();
			createInfo.surface = m_swapchainSettings.Device->GetSurface();
			createInfo.minImageCount = imageCount;
			createInfo.imageFormat = surfaceFormat.format;
			createInfo.imageColorSpace = surfaceFormat.colorSpace;
			createInfo.imageExtent = extent;
			createInfo.imageArrayLayers = 1;
			createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

			std::vector<uint32_t> queueFamilyIndices
			{
				m_swapchainSettings.Device->GetQueueFamily(Render::QueueFamilyType::Graphics).GetValue(),
				m_swapchainSettings.Device->GetQueueFamily(Render::QueueFamilyType::Present).GetValue()
			};

			if (m_swapchainSettings.Device->GetQueueFamily(Render::QueueFamilyType::Graphics).GetValue() !=
				m_swapchainSettings.Device->GetQueueFamily(Render::QueueFamilyType::Present).GetValue())
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

			ThrowIfFailed(vkCreateSwapchainKHR(m_swapchainSettings.Device->GetDevice(), &createInfo, nullptr, &m_swapchain));

			imageCount = 0;
			std::vector<VkImage> swapChainImages;
			vkGetSwapchainImagesKHR(m_swapchainSettings.Device->GetDevice(), m_swapchain, &imageCount, nullptr);
			swapChainImages.resize(imageCount);
			vkGetSwapchainImagesKHR(m_swapchainSettings.Device->GetDevice(), m_swapchain, &imageCount, swapChainImages.data());

			IS_CORE_INFO("Swapchain images completed.");

			m_inFlightFences.resize(imageCount);
			m_imagesInFlight.resize(imageCount);

			for (size_t i = 0; i < imageCount; i++)
			{
				VulkanFramebuffer* fb = Memory::MemoryManager::NewOnFreeList<VulkanFramebuffer>(m_swapchainSettings.Device, extent);
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

		void Swapchain::RecreateSwapchain(const Event& event)
		{
			WindowResizeEvent e = static_cast<const WindowResizeEvent&>(event);
			unsigned int width = e.m_width;
			unsigned int height = e.m_height;

			while (width == 0 || height == 0)
			{
				width = m_swapchainSettings.Window->GetWidth();
				height = m_swapchainSettings.Window->GetHeight();
				m_swapchainSettings.Window->WaitForEvents();
			}

			m_drawCommandPool->FreeCommandBuffers();
			m_drawCommandBuffers = m_drawCommandPool->AllocCommandBuffers(3);

			for (size_t i = 0; i < m_swapchainFramebuffers.size(); ++i)
			{
				Memory::MemoryManager::DeleteOnFreeList(m_swapchainFramebuffers[i]);
			}
			m_swapchainFramebuffers.clear();
			vkDestroySwapchainKHR(m_swapchainSettings.Device->GetDevice(), m_swapchain, nullptr);
			CreateSwapChain();

			Memory::MemoryManager::DeleteOnFreeList(m_swapchainShader);
			ShaderData data
			{
				m_swapchainSettings.Device,
				{"shader.vert", "shader.frag"},
				VkExtent2D{ static_cast<uint32_t>(m_swapchainSettings.Window->GetWidth()), static_cast<uint32_t>(m_swapchainSettings.Window->GetHeight())},
				m_swapchainFramebuffers[0]->GetRenderpass()
			};
			m_swapchainShader = Memory::MemoryManager::NewOnFreeList<VulkanShader>(data);
			for (auto it = m_materials.begin(); it != m_materials.end(); ++it)
			{
				(*it)->SetShader(m_swapchainShader);
			}
		}
	}
}
#endif