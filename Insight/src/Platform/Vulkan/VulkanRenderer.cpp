#include "ispch.h"
#ifdef IS_VULKAN
#include "Vulkan.h"
#include "VulkanRenderer.h"
#include "QueueFamily.h"
#include "VulkanBuffers.h"
#include "VulkanMaterial.h"

#include "Insight/Module/ModuleManager.h"
#include "Insight/Library/ShaderLibrary.h"
#include "Insight/Event/EventManager.h"
#include "Insight/Component/MeshComponent.h"
#include "Insight/Component/TransformComponent.h"
#include "Insight/Entitiy/Entity.h"
#include "Insight/Camera.h"

#include "Insight/Time/Time.h"
#include "Insight/Config/Config.h"
#include "Insight/Instrumentor/Instrumentor.h"


namespace Platform
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

	VulkanRenderer::VulkanRenderer()
	{
		VulkanVertexBuffer::s_Renderer = this;
		VulkanIndexBuffer::s_Renderer = this;
		VulkanMaterial::s_Renderer = this;


		m_windowModule = Insight::Module::ModuleManager::GetInstance()->GetModule < Insight::Module::WindowModule>();
		m_recordCommandBuffers = true;

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

		m_device = NEW_ON_HEAP(Device, m_validationLayers, m_deviceExtensions, GetRequiredExtensions());

		m_framebuffer = NEW_ON_HEAP(VulkanFramebuffer, m_device, m_windowModule->GetWindow()->GetWidth(), m_windowModule->GetWindow()->GetHeight());
		m_framebuffer->CreateAttachment(VK_FORMAT_B8G8R8A8_SRGB, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		m_framebuffer->CreateAttachment(VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		m_framebuffer->CompileFrameBuffer();

		std::vector<std::string> shaderPaths = { "vulkan/offscreen_shader.vert", "vulkan/offscreen_shader.frag" };
		glm::ivec2 extent = glm::ivec2(m_windowModule->GetWindow()->GetWidth(), m_windowModule->GetWindow()->GetHeight());
		m_shader = NEW_ON_HEAP(VulkanShader, m_device, shaderPaths, extent, m_framebuffer->GetRenderpass());
		Insight::Library::ShaderLibrary::GetInstance()->AddAsset(m_shader->GetUUID(), m_shader);

		m_material = static_cast<VulkanMaterial*>(Material::Create());
		m_material->SetShader(m_shader);

		m_commandPool = NEW_ON_HEAP(CommandPool, m_device, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
		m_commandBuffer = m_commandPool->AllocCommandBuffer();

		m_swapchain = NEW_ON_HEAP(Swapchain, m_device);

		Insight::EventManager::Bind(Insight::EventType::WindowResize, typeid(VulkanRenderer).name(), BIND_FUNC(VulkanRenderer::RecreateFramebuffers, this));
		Insight::EventManager::Bind(Insight::EventType::Deserialize, typeid(VulkanRenderer).name(), BIND_FUNC(VulkanRenderer::DeserializeFromFile, this));

		IS_CORE_INFO("Vulkan Setup Complete.");
	}

	VulkanRenderer::~VulkanRenderer()
	{
		m_device->WaitForIdle();

		Insight::EventManager::Unbind(Insight::EventType::WindowResize, typeid(VulkanRenderer).name());
		Insight::EventManager::Unbind(Insight::EventType::Deserialize, typeid(VulkanRenderer).name());

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


	void VulkanRenderer::Render(Insight::Camera* mainCamera, std::vector<MeshComponent*> meshes)
	{
		IS_PROFILE_FUNCTION();

		glm::mat4 model(1.0f);

		m_material->UpdateMVPUniform(mainCamera->GetProjMatrix(), mainCamera->GetViewMatrix(), model);
		std::vector<glm::mat4> objectsMatrix;
		{
			if (meshes.size() > 0)
			{
				IS_PROFILE_SCOPE("Object matrixs");
				//for (auto it = meshes.begin(); it != meshes.end(); ++it)
				//{
				//	objectsMatrix.push_back((*it)->GetEntity()->GetComponent<TransformComponent>()->GetTransform());
				//}
				//if (objectsMatrix.size() > 0)
				//{
				m_material->UpdateObjectsUniforms();
				//}
			}
		}

		m_swapchain->AcquireNextImage();

		m_framebuffer->GetFence()->Wait();

		if (m_recordCommandBuffers)
		{
			m_recordCommandBuffers = false;
			vertexCount = 0;
			triCount = 0;
			meshCount = 0;
			{
				IS_PROFILE_SCOPE("All Draw");

				m_material->SetUniforms();

				m_commandBuffer->StartRecord();
				m_framebuffer->BindBuffer(m_commandBuffer);

				int drawIndex = 0;
				for (auto it = meshes.begin(); it != meshes.end(); ++it)
				{
					IS_PROFILE_SCOPE("Single Darw");

					if ((*it)->GetMesh() == nullptr)
					{
						continue;
					}

					m_material->Bind(m_commandBuffer, *it);
					drawIndex++;

					vertexCount += (*it)->GetMesh()->GetVertexCount();
					triCount += (*it)->GetMesh()->GetIndicesCount();
					meshCount++;

					VkBuffer vertexBuffers[] = { static_cast<VulkanVertexBuffer*>((*it)->GetMesh()->GetVertexBuffer())->GetBuffer() };
					VkBuffer indexBuffer = static_cast<VulkanIndexBuffer*>((*it)->GetMesh()->GetIndexBuffer())->GetBuffer();
					VkDeviceSize offsets[] = { 0 };

					vkCmdBindVertexBuffers(m_commandBuffer->GetBuffer(), 0, 1, vertexBuffers, offsets);
					vkCmdBindIndexBuffer(m_commandBuffer->GetBuffer(), indexBuffer, 0, VK_INDEX_TYPE_UINT32);

					vkCmdDrawIndexed(m_commandBuffer->GetBuffer(), static_cast<uint32_t>((*it)->GetMesh()->GetIndicesCount()), 1, 0, 0, 0);
				}
			}
			m_framebuffer->UnbindBuffer(m_commandBuffer);
			m_commandBuffer->EndRecord();
		}

		{
			IS_PROFILE_SCOPE("Submit for draw");

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

		}
		std::string vStr = GetFormatInt(vertexCount);
		std::string tStr = GetFormatInt(triCount / 3);

#if defined(IS_EDITOR) && defined(IMGUI_ENABLED)
		ImGui::Begin("Renderer");
		ImGui::Text("Vertex Count: %s", vStr.c_str());
		ImGui::Text("Tri Count: %s", tStr.c_str());
		ImGui::Text("Mesh Count: %d", meshCount);
		ImGui::End();
#endif
		m_swapchain->Draw(m_framebuffer->GetFinishedSem(), m_framebuffer);
	}

	void VulkanRenderer::Present()
	{
		IS_PROFILE_FUNCTION();

		m_swapchain->Present();
	}

	Material* VulkanRenderer::GetDefaultMaterial()
	{
		return m_material;
	}

	void VulkanRenderer::RecreateFramebuffers(const Insight::Event& event)
	{
		m_commandPool->FreeCommandBuffers();
		m_commandBuffer = m_commandPool->AllocCommandBuffer();

		m_framebuffer->Resize(m_windowModule->GetWindow()->GetWidth(), m_windowModule->GetWindow()->GetHeight());
		m_shader->Resize(m_windowModule->GetWindow()->GetWidth(), m_windowModule->GetWindow()->GetHeight());
		m_material->Resize();

		m_recordCommandBuffers = true;
	}

	void VulkanRenderer::DeserializeFromFile(const Insight::Event& event)
	{
		m_recordCommandBuffers = true;
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
#endif