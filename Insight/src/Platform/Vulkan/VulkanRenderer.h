#pragma once

#ifdef IS_VULKAN
#include "Insight/Core/Core.h"
#include "Insight/Renderer/Renderer.h"
#include "Insight/Module/WindowModule.h"

#include "Swapchain.h"
#include "VulkanMaterial.h"
#include "VulkanFrameBuffer.h"
#include "Insight/Model/Mesh.h"

#include "Insight/Renderer/ImGuiRenderer.h"
#include "Insight/Threading/Threadpool.hpp"

#include <glm/glm.hpp>

class Model;
class CameraComponent;
class Entity;

namespace Insight
{
	class Event;
}

const int MAX_FRAMES_IN_FLIGHT = 3;

namespace vks
{
//#if defined(IS_EDITOR)
//	class VulkanRendererEditorOverlay : public Insight::Editor::EditorWindow
//	{
//	public:
//		VulkanRendererEditorOverlay(SharedPtr<Insight::Module::EditorModule> editorModule, SharedPtr<VulkanRenderer> renderer);
//		~VulkanRendererEditorOverlay();
//
//		virtual void Update(const float& deltaTime) override;
//
//	private:
//		WeakPtr<VulkanRenderer> m_renderer;
//	};
//#endif

	class VulkanDevice;

	class IS_API VulkanRenderer : public Insight::Renderer
		, public std::enable_shared_from_this<VulkanRenderer>
	{
	public:
		VulkanRenderer();
		virtual ~VulkanRenderer() override;

		virtual void OnCreate() override;

		void InitVulkan();

		virtual void Clear() override;
		virtual void Render(CameraComponent* mainCamera, std::vector<WeakPtr<MeshComponent>>& meshes) override;
		virtual void Present() override;
		virtual void WaitForIdle() override;

		/** @brief Return our instance */
		VkInstance GetInstance() { return m_instance; }
		/** @brief Return our device wrapper */
		SharedPtr<vks::VulkanDevice> GetDevice() { return m_vulkanDevice; }
		/** @brief Return our physical device */
		VkPhysicalDevice GetPhysicalDevice() { return m_physicalDevice; }
		/** @brief Return the graphics queue */
		VkQueue GetQueue() { return m_queue; }
		/** @brief Return the default view port */
		VkViewport GetViewPort() { return m_viewPort; }
		/** @brief Return the default view port */
		VkRect2D GetScissor() { return m_scissor; }
		VkRenderPass GetPresentRenderPass() { return m_presentRenderPass; }

	private:
		void Prepare();
		void InitSwapchain();
		void SetupSwapchain();
		void CreateCommandBuffers();
		void DestroyCommandBuffers();
		void CreateCommandPool();
		void SetupScreenRender();
		void SetupPresentRenderPass();

		void CreateSynchronizationPrimitives();

		/** @brief (Virtual) Creates the application wide Vulkan instance */
		virtual VkResult CreateInstance(bool enableValidation);
		/** @brief (Virtual) Called when resources have been recreated that require a rebuild of the command buffers (e.g. frame buffer), to be implemented by the sample application */
		virtual void BuildCommandBuffers(std::vector<WeakPtr<MeshComponent>>& meshes);
		virtual void BuildPresentBuffers();
		/** @brief (Virtual) Setup default depth and stencil views */
		virtual void SetupDepthStencil();
		/** @brief (Virtual) Setup default framebuffers for all requested swapchain images */
		virtual void SetupFrameBuffer();
		/** @brief (Virtual) Called after the physical device features have been read, can be used to set features to enable on the device */
		virtual void GetEnabledFeatures();

		VkSubmitInfo GetSubmitInfo();

		float rnd(float range);
		void PrepareMultiThreadedRenderer();
		void UpdateSecondaryCommandBuffers(VkCommandBufferInheritanceInfo inheritanceInfo);
		void UpdateCommandBuffer(VkFramebuffer frameBuffer);

		void ThreadRenderCode(int32_t threadIndex, uint32_t cmdBufferIndex, VkCommandBufferInheritanceInfo inheritanceInfo);

		void WindowResizeEvent(const Insight::Event& event);

		// Use push constants to update shader
		// parameters on a per-thread base
		struct ThreadPushConstantBlock
		{
			glm::mat4 mvp;
			glm::vec3 color;
		};

		struct ObjectData
		{
			glm::mat4 model;
			glm::vec3 pos;
			glm::vec3 rotation;
			float rotationDir;
			float rotationSpeed;
			float scale;
			float deltaT;
			float stateT = 0;
			bool visible = false;
		};

		struct ThreadData
		{
			VkCommandPool commandPool;
			// One command buffer per render object
			std::vector<VkCommandBuffer> commandBuffer;
			// One push constant block per render object
			std::vector<ThreadPushConstantBlock> pushConstBlock;
			// Per object information (position, rotation, etc.)
			std::vector<ObjectData> objectData;
		};

	private:
		bool m_recordCommandBuffers;
		VulkanFrameBuffer m_frameBuffer;

		std::vector<ThreadData> m_threadData;
		// Multi threaded stuff
		// Max. number of concurrent threads
		uint32_t m_numThreads;
		thread_pool m_threadPool;
		uint32_t m_numObjectsPerThread;

		std::default_random_engine m_rndEngine;

		// Vulkan objects
		Swapchain m_swapchain;

		VkClearColorValue m_clearColor;

		struct
		{
			// Swap chain image presentation
			VkSemaphore ImageAquired[MAX_FRAMES_IN_FLIGHT];
			// Command buffer submission and execution
			VkSemaphore RenderComplete[MAX_FRAMES_IN_FLIGHT];
		}m_semaphores;

		struct
		{
			VkImage Image;
			VkDeviceMemory Mem;
			VkImageView View;
		} m_depthStencil;

		// Vulkan instance, stores all per-application states
		VkInstance m_instance;
		std::vector<std::string> m_supportedInstanceExtensions;
		// Physical device (GPU) that Vulkan will use
		VkPhysicalDevice m_physicalDevice;
		// Stores physical device properties (for e.g. checking device limits)
		VkPhysicalDeviceProperties m_deviceProperties;
		// Stores the features available on the selected physical device (for e.g. checking if a feature is available)
		VkPhysicalDeviceFeatures m_deviceFeatures;
		// Stores all available memory (type) properties for the physical device
		VkPhysicalDeviceMemoryProperties m_deviceMemoryProperties;
		/** @brief Set of physical device features to be enabled for this example (must be set in the derived constructor) */
		VkPhysicalDeviceFeatures m_enabledFeatures{};
		/** @brief Set of device extensions to be enabled for this example (must be set in the derived constructor) */
		std::vector<const char*> m_enabledDeviceExtensions;
		std::vector<const char*> m_enabledInstanceExtensions;
		/** @brief Optional pNext structure for passing extension structures to device creation */
		void* m_deviceCreatepNextChain = nullptr;

		/** @brief Encapsulated physical and logical vulkan device */
		SharedPtr<vks::VulkanDevice> m_vulkanDevice;

		/** @brief Logical device, application's view of the physical device (GPU) */
		VkDevice m_device;
		// Handle to the device graphics queue that command buffers are submitted to
		VkQueue m_queue;
		// Depth buffer format (selected during Vulkan initialization)
		VkFormat m_depthFormat;
		// Command buffer pool
		VkCommandPool m_cmdPool;
		/** @brief Pipeline stages used to wait at for graphics queue submissions */
		VkPipelineStageFlags m_submitPipelineStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		// Contains command buffers and semaphores to be presented to the queue
		VkSubmitInfo m_submitInfo;

		// Present 
		std::vector<VkCommandBuffer> m_presentCmdBuffers;
		// List of available frame buffers (same as number of swap chain images)
		std::vector<VkFramebuffer> m_presentFrameBuffers;
		SharedPtr<Material> m_presentMaterials[MAX_FRAMES_IN_FLIGHT];
		SharedPtr<Mesh> m_presentMeshes[MAX_FRAMES_IN_FLIGHT];
		VkRenderPass m_presentRenderPass;

		VkCommandBuffer m_frameBufferCmdBuffer;
		//std::vector<VkCommandBuffer> m_frameBufferCmdBuffer;
		std::vector<VkFence> m_frameBufferCmdFence;
		// Pirmary Command buffers used for rendering
		VkCommandBuffer m_primaryCommandBuffer;

		struct
		{
			VkCommandBuffer UI;
		} m_secondaryCommandBuffers;
		uint32_t m_imageIndex = 0;
		uint32_t m_currentFrame = 0;
		// List of shader modules created (stored for cleanup)
		std::vector<VkShaderModule> m_shaderModules;
		// Default viewport and scissor
		VkViewport m_viewPort;
		VkRect2D m_scissor;

		VkFence m_waitFences[MAX_FRAMES_IN_FLIGHT];
		VkFence m_waitImagesFences[MAX_FRAMES_IN_FLIGHT];

		SharedPtr<Entity> m_editorEntity;
		SharedPtr<CameraComponent> m_editorCamera;

		struct MVP
		{
			glm::mat4 proj;
			glm::mat4 view;
			glm::mat4 model;
			glm::vec4 lightPos;
		};
		glm::vec4 m_lightPos;
		float m_lightPosAngle;

		struct DebugOverlay
		{
			glm::vec3 debugOptions;
			int debugOverlay;
		};
		bool debugOverlay;
		DebugOverlay m_debugOverlay;
	};
}
#endif