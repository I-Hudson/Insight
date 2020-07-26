#include "ispch.h"
#ifdef IS_VULKAN
#include "Insight/Renderer/Vulkan/Vulkan.h"
#include "Insight/Renderer/Vulkan/VulkanRenderer.h"
#include "Insight/Renderer/Vulkan/QueueFamily.h"
#include "Insight/Renderer/Vulkan/VulkanBuffers.h"
#include "Insight/Renderer/Vulkan/VulkanMaterial.h"

#include "Insight/Event/EventManager.h"
#include "Insight/Component/MeshComponent.h"
#include "Insight/Component/TransformComponent.h"
#include "Insight/Entitiy/Entity.h"
#include "Insight/Camera.h"

#include "Insight/Time/Time.h"
#include "Insight/Config/Config.h"

namespace Insight
{
	namespace Render
	{
		VulkanRenderer::VulkanRenderer(RendererStartUpData& startupData)
		{
			Render::VulkanVertexBuffer::s_Renderer = this;
			Render::VulkanIndexBuffer::s_Renderer = this;
			Render::VulkanMaterial::s_Renderer = this;


			m_windowModule = startupData.WindowModule;

			m_validationLayers =
			{
				"VK_LAYER_KHRONOS_validation"
			};

			m_deviceExtensions =
			{
				VK_KHR_SWAPCHAIN_EXTENSION_NAME
			};

			if (!CheckValidationLayerSupport(m_validationLayers))
			{
				m_validationLayers.clear();
			}

			DeviceSettings deviceSettings
			{
				m_validationLayers,
				m_deviceExtensions,
				GetRequiredExtensions()
			};
			m_device = NEW_ON_HEAP(Device, deviceSettings);

			m_framebuffer = NEW_ON_HEAP(VulkanFramebuffer, m_device, m_windowModule->GetWindow()->GetWidth(), m_windowModule->GetWindow()->GetHeight());
			m_framebuffer->CreateAttachment(VK_FORMAT_B8G8R8A8_SRGB, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
											VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			m_framebuffer->CreateAttachment(VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 
											VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
											VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
			m_framebuffer->CompileFrameBuffer();

			ShaderData data
			{
				m_device,
				{"vulkan/offscreen_shader.vert", "vulkan/offscreen_shader.frag"},
				VkExtent2D{ static_cast<uint32_t>(m_windowModule->GetWindow()->GetWidth()), static_cast<uint32_t>(m_windowModule->GetWindow()->GetHeight())},
				m_framebuffer->GetRenderpass()
			};
			m_shader = NEW_ON_HEAP(VulkanShader, data);
			
			m_material = static_cast<VulkanMaterial*>(Material::Create());
			m_material->SetShader(m_shader);

			m_commandPool = NEW_ON_HEAP(CommandPool, m_device, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
			m_commandBuffer = m_commandPool->AllocCommandBuffer();

			SwapchainSettings swapchainSettings
			{
				m_windowModule->GetWindow(),
				m_device
			};
			m_swapchain = NEW_ON_HEAP(Swapchain, swapchainSettings);

			// Position				  // Colour				   // Normal				//UV1
			std::vector<Vertex> vertices =
			{
				{{1.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f,}},
				{{-1.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f,}},
				{{-1.0f, -1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f,}},
				{{1.0f, -1.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f,}},
			};

			std::vector<unsigned int> indices =
			{
				0,1,2, 2, 3, 0
			};
			m_testMesh = NEW_ON_HEAP(Mesh, vertices, indices, std::vector<Texture>());

			EventManager::Bind(EventType::WindowResize, typeid(VulkanRenderer).name(), BIND_FUNC(VulkanRenderer::RecreateFramebuffers, this));

			IS_CORE_INFO("Vulkan Setup Complete.");
		}

		VulkanRenderer::~VulkanRenderer()
		{
			m_device->WaitForIdle();

			EventManager::Unbind(EventType::WindowResize, typeid(VulkanRenderer).name());

			DELETE_ON_HEAP(m_testMesh);

			DELETE_ON_HEAP(m_shader);
			DELETE_ON_HEAP(m_material);
			DELETE_ON_HEAP(m_framebuffer);
			DELETE_ON_HEAP(m_commandPool);
			
			DELETE_ON_HEAP(m_swapchain);
			DELETE_ON_HEAP(m_device);
		}

		void VulkanRenderer::Clear()
		{
		}


		struct ColorUniform
		{
			glm::vec4 Colour1;
		};

		void VulkanRenderer::Render(Camera* mainCamera, std::vector<MeshComponent*> meshes)
		{
			glm::mat4 model(1.0f);

			m_material->UpdateMVPUniform(mainCamera->GetProjMatrix(), mainCamera->GetViewMatrix(), model);
			m_material->SetUniforms();

			m_swapchain->AcquireNextImage();

			m_framebuffer->GetFence()->Wait();

			m_commandBuffer->StartRecord();
			m_framebuffer->BindBuffer(m_commandBuffer);
			m_material->Bind(m_commandBuffer);
			
			for (auto it = meshes.begin(); it != meshes.end(); ++it)
			{
				if ((*it)->GetMesh() == nullptr)
				{
					continue;
				}

				VkBuffer vertexBuffers[] = { static_cast<VulkanVertexBuffer*>((*it)->GetMesh()->GetVertexBuffer())->GetBuffer() };
				VkBuffer indexBuffer = static_cast<VulkanIndexBuffer*>((*it)->GetMesh()->GetIndexBuffer())->GetBuffer();
				VkDeviceSize offsets[] = { 0 };

				vkCmdBindVertexBuffers(m_commandBuffer->GetBuffer(), 0, 1, vertexBuffers, offsets);
				vkCmdBindIndexBuffer(m_commandBuffer->GetBuffer(), indexBuffer, 0, VK_INDEX_TYPE_UINT32);

				vkCmdDrawIndexed(m_commandBuffer->GetBuffer(), static_cast<uint32_t>((*it)->GetMesh()->GetIndicesCount()), 1, 0, 0, 0);
			}
			m_framebuffer->UnbindBuffer(m_commandBuffer);
			m_commandBuffer->EndRecord();
			
			std::vector<VkSemaphore> waitSemaphores = { m_swapchain->GetAcquireNextImageSemaphore()->GetSemaphore() };
			std::vector<VkPipelineStageFlags> stageFlags = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
			std::vector<VkCommandBuffer> commandBuffers = { m_commandBuffer->GetBuffer() };
			std::vector<VkSemaphore> signalSemaphore = { m_framebuffer->GetFinishedSem()->GetSemaphore() };
			VkSubmitInfo submitInfo = VulkanInits::SubmitInfo(waitSemaphores, stageFlags, commandBuffers, signalSemaphore);

			m_framebuffer->GetFence()->Reset();

			GraphicsQueueInfo info;
			info.SubmitInfo = &submitInfo;
			info.SyncFence = m_framebuffer->GetFence();
			m_device->GetQueue(QueueFamilyType::Graphics).Submit(info);

			m_swapchain->Draw(m_framebuffer->GetFinishedSem(), m_framebuffer);
		}

		void VulkanRenderer::Present()
		{
			m_swapchain->Present();
		}

		void VulkanRenderer::RecreateFramebuffers(const Event& event)
		{
			m_commandPool->FreeCommandBuffers();
			m_commandBuffer = m_commandPool->AllocCommandBuffer();

			m_framebuffer->Resize(m_windowModule->GetWindow()->GetWidth(), m_windowModule->GetWindow()->GetHeight());
			m_shader->Resize(m_windowModule->GetWindow()->GetWidth(), m_windowModule->GetWindow()->GetHeight());
		}

		bool VulkanRenderer::CheckDeviceExtensionSupport(VkPhysicalDevice device)
		{
			uint32_t extensionCount;
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

			std::vector<VkExtensionProperties> availableExtensions(extensionCount);
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

			std::set<std::string> requiredExtensions(m_deviceExtensions.begin(), m_deviceExtensions.end());

			for (const auto& extension : availableExtensions) {
				requiredExtensions.erase(extension.extensionName);
			}

			return requiredExtensions.empty();
		}

		bool VulkanRenderer::CheckValidationLayerSupport(const std::vector<const char*>& validationLayers)
		{
			uint32_t layerCount;
			vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

			std::vector<VkLayerProperties> availableLayers(layerCount);
			vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

			for (const char* layerName : validationLayers) {
				bool layerFound = false;

				for (const auto& layerProperties : availableLayers) {
					if (strcmp(layerName, layerProperties.layerName) == 0) {
						layerFound = true;
						break;
					}
				}

				if (!layerFound) {
					return false;
				}
			}

			return true;
		}

		std::vector<const char*> VulkanRenderer::GetRequiredExtensions()
		{
			uint32_t glfwExtensionCount = 0;
			const char** glfwExtensions;
			glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

			std::vector<const char*> extensionVector(glfwExtensions, glfwExtensions + glfwExtensionCount);

			if (m_enableValidationLayers)
			{
				extensionVector.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			}

			return extensionVector;
		}

		void VulkanRenderer::VulkanEnumExtProps()
		{
			UINT numExt = 0;
			ThrowIfFailed(vkEnumerateInstanceExtensionProperties(nullptr, &numExt, nullptr));


			printf("Found %d extensions \n", numExt);
			std::vector<VkExtensionProperties> props;
			props.resize(numExt);

			ThrowIfFailed(vkEnumerateInstanceExtensionProperties(NULL, &numExt, &props[0]));

			uint32_t extensionCompleted = 0;
			std::vector<const char*> extensions = GetRequiredExtensions();

			for (UINT i = 0; i < numExt; ++i)
			{
				printf("Instance extension %d - %s\n", i, props[i].extensionName);

				for (size_t j = 0; j < extensions.size(); ++j)
				{
					const char* reqEx = extensions[j];
					if (strcmp(props[i].extensionName, reqEx) == 0)
					{
						extensionCompleted++;
						break;
					}
				}
			}

			IS_CORE_INFO("Required extensions found: {0}", extensionCompleted == extensions.size());
		}
	}
}
#endif