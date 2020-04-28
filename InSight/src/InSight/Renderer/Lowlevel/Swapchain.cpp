#include "ispch.h"
#include "Swapchain.h"

#include "Insight/Renderer/Lowlevel/Device.h"
#include "Insight/Renderer/Lowlevel/Queue.h"
#include "Insight/Module/WindowModule.h"
#include "Insight/Renderer/VulkanInits.h"

#include "Insight/Memory/MemoryManager.h"

#include "Insight/Renderer/Lowlevel/ShaderModule.h"

namespace Insight
{
	namespace Render
	{
		Swapchain::Swapchain(const SwapchainSettings swapchainSettings)
			: m_swapchainSettings(swapchainSettings)
		{
			std::vector<ImageAttachment> imageAttachements;
			imageAttachements.push_back(ImageAttachment
				{
					VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_FORMAT_B8G8R8A8_SRGB, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_CLEAR,
					VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
					VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
				});

			RenderpassData renderpassData
			{
				imageAttachements,
				VK_PIPELINE_BIND_POINT_GRAPHICS
			};
			m_renderpass = Memory::MemoryManager::NewOnFreeList<Renderpass>(m_swapchainSettings.Device, renderpassData);

			std::vector<ShaderModuleBase> shaderModules =
			{ VertexShader(m_swapchainSettings.Device, "shader.vert"), 
			  FragmentShader(m_swapchainSettings.Device, "shader.frag") 
			};
			ShaderData data
			{
				m_swapchainSettings.Device,
				shaderModules,
				VkExtent2D{ static_cast<uint32_t>(m_swapchainSettings.Window->GetWidth()), static_cast<uint32_t>(m_swapchainSettings.Window->GetHeight())},
				m_renderpass
			};
			m_swapchainShader = Memory::MemoryManager::NewOnFreeList<Shader>(data);

			std::vector<ShaderModuleBase> shaderModules1 =
			{ VertexShader(m_swapchainSettings.Device, "shader - Copy.vert"), 
			  FragmentShader(m_swapchainSettings.Device, "shader.frag") };
			ShaderData data1
			{
				m_swapchainSettings.Device,
				shaderModules1,
				VkExtent2D{ static_cast<uint32_t>(m_swapchainSettings.Window->GetWidth()), static_cast<uint32_t>(m_swapchainSettings.Window->GetHeight())},
				m_renderpass
			};
			m_swapchainTestShader = Memory::MemoryManager::NewOnFreeList<Shader>(data1);

			CreateSwapChain();

			m_drawCommandPool = Memory::MemoryManager::NewOnFreeList<CommandPool>(m_swapchainSettings.Device, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
			m_drawCommandBuffers = m_drawCommandPool->AllocCommandBuffers(3);

			m_imageAvaliable = Memory::MemoryManager::NewOnFreeList<Semaphore>(m_swapchainSettings.Device);
			m_renderFinished = Memory::MemoryManager::NewOnFreeList<Semaphore>(m_swapchainSettings.Device);

			IS_CORE_INFO("SwapChain completed.");
		}

		Swapchain::~Swapchain()
		{
			Memory::MemoryManager::DeleteOnFreeList(m_imageAvaliable);
			Memory::MemoryManager::DeleteOnFreeList(m_renderFinished);
			Memory::MemoryManager::DeleteOnFreeList(m_drawCommandPool);
			Memory::MemoryManager::DeleteOnFreeList(m_renderpass);

			Memory::MemoryManager::DeleteOnFreeList(m_swapchainShader);
			Memory::MemoryManager::DeleteOnFreeList(m_swapchainTestShader);

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

				actualExtent.width = max(capabilities.minImageExtent.width, min(capabilities.maxImageExtent.width, actualExtent.width));
				actualExtent.height = max(capabilities.minImageExtent.height, min(capabilities.maxImageExtent.height, actualExtent.height));

				return actualExtent;
			}
		}

		void Swapchain::AcquireNextImage()
		{
			m_swapchainFramebuffers[m_currentFrame]->GetFence()->Wait();

			vkAcquireNextImageKHR(m_swapchainSettings.Device->GetDevice(), m_swapchain, U64_MAX, 
								  m_swapchainFramebuffers[m_currentFrame]->GetAvailbleSem()->GetSemaphore(),
								  VK_NULL_HANDLE, &m_imageIndex);
		
			if (m_swapchainFramebuffers[m_imageIndex]->GetFence()->GetInUse())
			{
				m_swapchainFramebuffers[m_imageIndex]->GetFence()->Wait();
			}

			std::vector<VkSemaphore> waitSemaphore = { m_swapchainFramebuffers[m_currentFrame]->GetAvailbleSem()->GetSemaphore() };
			std::vector<VkPipelineStageFlags> stageFlags = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
			std::vector<VkCommandBuffer> commandBuffers = { m_drawCommandBuffers[m_imageIndex]->GetBuffer() };
			std::vector<VkSemaphore> signalSemaphore = { m_swapchainFramebuffers[m_currentFrame]->GetFinishedSem()->GetSemaphore() };
			VkSubmitInfo submitInfo = VulkanInits::SubmitInfo(waitSemaphore, stageFlags, commandBuffers, signalSemaphore);

			m_swapchainFramebuffers[m_imageIndex]->GetFence()->Reset();

			GraphicsQueueInfo info;
			info.SubmitInfo = &submitInfo;
			info.SyncFence = m_swapchainFramebuffers[m_currentFrame]->GetFence();
			m_swapchainSettings.Device->GetQueue(QueueFamilyType::Graphics).Submit(info);
		}

		void Swapchain::Draw()
		{
			int i = 0;
			++tempShader;
			for (auto it = m_drawCommandBuffers.begin(); it != m_drawCommandBuffers.end(); ++it)
			{
				(*it)->StartRecord();

				m_swapchainFramebuffers[i++]->BindBuffer(*it);

				if (tempShader % 10000 == 0)
				{
					shaderIndex = ++shaderIndex % 2;
				}

				if (shaderIndex == 0)
				{
					m_swapchainShader->Bind(*it);
				}
				else
				{
					m_swapchainTestShader->Bind(*it);
				}

				vkCmdDraw((*it)->GetBuffer(), 3, 1, 0, 0);
				
				vkCmdEndRenderPass((*it)->GetBuffer());
			
				(*it)->EndRecord();
			}
			AcquireNextImage();
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

			for (size_t i = 0; i < imageCount; i++)
			{
				Framebuffer* fb = Memory::MemoryManager::NewOnFreeList<Framebuffer>(m_swapchainSettings.Device, &swapChainImages[i], surfaceFormat.format, extent, m_renderpass);
				m_swapchainFramebuffers.push_back(std::move(fb));
			}
		}


		const VkSwapchainKHR& Swapchain::GetSwapchain() const
		{
			return m_swapchain;
		}
	}
}