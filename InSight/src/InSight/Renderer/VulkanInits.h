#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

namespace Insight
{
	namespace Render
	{
		class VulkanInits
		{
		public:

			static VkApplicationInfo ApplicationInfo(const char* appTitle, const char* engineTitle = "Engine")
			{
				VkApplicationInfo appInfo = {};
				appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
				appInfo.pApplicationName = appTitle;
				appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
				appInfo.pEngineName = engineTitle;
				appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
				appInfo.apiVersion = VK_API_VERSION_1_0;

				return appInfo;
			}

			static VkInstanceCreateInfo InstanceCreateInfo(const VkApplicationInfo& appInfo, const std::vector<const char*>& extensions, 
															const std::vector<const char*>& validationLayers = std::vector<const char*>(),
															void* debugCreateInfo = nullptr)
			{
				VkInstanceCreateInfo createInfo = {};
				createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
				createInfo.pApplicationInfo = &appInfo;
				createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
				createInfo.ppEnabledExtensionNames = extensions.data();
				createInfo.enabledLayerCount = 0;

				if (validationLayers.size() > 0)
				{
					createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
					createInfo.ppEnabledLayerNames = validationLayers.data();

					createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)debugCreateInfo;
				}
				else
				{
					createInfo.enabledLayerCount = 0;
					createInfo.pNext = nullptr;
				}

				return createInfo;
			}

			static VkDeviceCreateInfo DeviceInfo()
			{
				VkDeviceCreateInfo createInfo = {};
				createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

				return createInfo;
			}

			static VkDeviceQueueCreateInfo DeviceQueueInfo()
			{
				VkDeviceQueueCreateInfo queueCreateInfo = {};
				queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;

				return queueCreateInfo;
			}

			static VkSwapchainCreateInfoKHR SwapChainInfo()
			{
				VkSwapchainCreateInfoKHR createInfo{};
				createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
				return createInfo;
			}

			static VkImageCreateInfo ImageCreateInfo(const VkFormat& format, const VkExtent2D& extent, const VkImageUsageFlags& usage)
			{
				VkImageCreateInfo image{};
				image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
				image.imageType = VK_IMAGE_TYPE_2D;
				image.format = format;
				image.extent.width = extent.width;
				image.extent.height = extent.height;
				image.extent.depth = 1;
				image.mipLevels = 1;
				image.arrayLayers = 1;
				image.samples = VK_SAMPLE_COUNT_1_BIT;
				image.tiling = VK_IMAGE_TILING_OPTIMAL;
				image.usage = usage | VK_IMAGE_USAGE_SAMPLED_BIT;
				return image;
			}

			static VkImageViewCreateInfo ImageViewInfo()
			{
				VkImageViewCreateInfo createInfo{};
				createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				return createInfo;
			}

			static VkMemoryAllocateInfo MemoryAllocInfo()
			{
				VkMemoryAllocateInfo info{};
				info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
				return info;
			}

			static VkShaderModuleCreateInfo ShaderModuleInfo(const std::vector<uint32_t>& code)
			{
				VkShaderModuleCreateInfo createInfo{};
				createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
				createInfo.codeSize = code.size() * sizeof(uint32_t);
				createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
				return createInfo;
			}

			static VkPipelineShaderStageCreateInfo ShaderPipelineInfo()
			{
				VkPipelineShaderStageCreateInfo createInfo{};
				createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				createInfo.pName = "main";
				return createInfo;
			}

			static VkPipelineInputAssemblyStateCreateInfo PipelineInputAssemblyInfo()
			{
				VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
				inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
				inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
				inputAssembly.primitiveRestartEnable = VK_FALSE;

				return inputAssembly;
			}

			static VkPipelineViewportStateCreateInfo PipelineViewportInfo(const VkViewport* viewport, const VkRect2D* scissor)
			{
				VkPipelineViewportStateCreateInfo viewportState{};
				viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
				viewportState.viewportCount = 1;
				viewportState.pViewports = viewport;
				viewportState.scissorCount = 1;
				viewportState.pScissors = scissor;

				return viewportState;
			}

			static VkPipelineRasterizationStateCreateInfo PipelineRasterizationInfo()
			{
				VkPipelineRasterizationStateCreateInfo rasterizer{};
				rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
				rasterizer.depthClampEnable = VK_FALSE;
				rasterizer.rasterizerDiscardEnable = VK_FALSE;
				rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
				rasterizer.lineWidth = 1.0f;
				rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
				rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
				rasterizer.depthBiasEnable = VK_FALSE;
				rasterizer.depthBiasConstantFactor = 0.0f; // Optional
				rasterizer.depthBiasClamp = 0.0f; // Optional
				rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

				return rasterizer;
			}

			static VkPipelineMultisampleStateCreateInfo PipelineMutisampleInfo()
			{
				VkPipelineMultisampleStateCreateInfo createInfo{};
				createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
				createInfo.sampleShadingEnable = VK_FALSE;
				createInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
				createInfo.minSampleShading = 1.0f; // Optional
				createInfo.pSampleMask = nullptr; // Optional
				createInfo.alphaToCoverageEnable = VK_FALSE; // Optional
				createInfo.alphaToOneEnable = VK_FALSE; // Optional
				return createInfo;
			}

			static VkPipelineDepthStencilStateCreateInfo PipelineDepthStencilInfo()
			{
				VkPipelineDepthStencilStateCreateInfo createInfo{};
				return createInfo;
			}

			static VkPipelineColorBlendStateCreateInfo PipelineColourBlendInfo(const std::vector<VkPipelineColorBlendAttachmentState>& colourBlendStates)
			{
				VkPipelineColorBlendStateCreateInfo colorBlending{};
				colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
				colorBlending.logicOpEnable = VK_FALSE;
				colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
				colorBlending.attachmentCount = static_cast<uint32_t>(colourBlendStates.size());
				colorBlending.pAttachments = colourBlendStates.data();
				colorBlending.blendConstants[0] = 0.0f; // Optional
				colorBlending.blendConstants[1] = 0.0f; // Optional
				colorBlending.blendConstants[2] = 0.0f; // Optional
				colorBlending.blendConstants[3] = 0.0f; // Optional
				return colorBlending;
			}

			static VkPipelineColorBlendAttachmentState PipelineColourBlendState()
			{
				VkPipelineColorBlendAttachmentState colorBlendAttachment{};
				colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
				colorBlendAttachment.blendEnable = VK_FALSE;
				colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
				colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
				colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
				colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
				colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
				colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional
				
				colorBlendAttachment.blendEnable = VK_TRUE;
				colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
				colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
				colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
				colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
				colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
				colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
				return colorBlendAttachment;
			}

			static VkPipelineDynamicStateCreateInfo PipelineDynamicState(const std::vector<VkDynamicState>& dynamicStates)
			{
				VkPipelineDynamicStateCreateInfo dynamicState{};
				dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
				dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
				dynamicState.pDynamicStates = dynamicStates.data();
				return dynamicState;
			}

			static VkPipelineVertexInputStateCreateInfo VertexInputInfo(const std::vector<VkVertexInputBindingDescription>& inputBindingDescs, 
				const std::vector<VkVertexInputAttributeDescription>& vertexBindingDescs)
			{
				VkPipelineVertexInputStateCreateInfo createInfo{};
				createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
				createInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(inputBindingDescs.size());
				createInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexBindingDescs.size());
				createInfo.pVertexBindingDescriptions = inputBindingDescs.data();
				createInfo.pVertexAttributeDescriptions = vertexBindingDescs.data();
				return createInfo;
			}

			static VkPipelineLayoutCreateInfo PipelineLayoutInfo(const std::vector<VkDescriptorSetLayout>& descriptorSetLayout)
			{
				VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
				pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
				pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayout.size());
				pipelineLayoutInfo.pSetLayouts = pipelineLayoutInfo.setLayoutCount > 0 ? descriptorSetLayout.data() : nullptr;
				pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
				pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional
				return pipelineLayoutInfo;
			}

			static VkGraphicsPipelineCreateInfo GraphicsPipelineInfo(const std::vector<VkPipelineShaderStageCreateInfo>* stages,
				const VkPipelineVertexInputStateCreateInfo* vertexInputInfo, const VkPipelineInputAssemblyStateCreateInfo* inputAssembly,
				const VkPipelineViewportStateCreateInfo* viewportState, const VkPipelineRasterizationStateCreateInfo* rasterizer,
				const VkPipelineMultisampleStateCreateInfo* multisampling, const VkPipelineDepthStencilStateCreateInfo* depthStencil,
				const VkPipelineColorBlendStateCreateInfo* colorBlending, const VkPipelineDynamicStateCreateInfo* dynamicState,
				const VkPipelineLayout& pipelineLayout, const VkRenderPass& renderPass, const int& subpass,
				const VkPipeline& basePipelineHandle = VK_NULL_HANDLE, const int& basePipelineIndex = -1)
			{
				VkGraphicsPipelineCreateInfo pipelineInfo{};
				pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
				pipelineInfo.stageCount = static_cast<uint32_t>(stages->size());
				pipelineInfo.pStages = stages->data();

				pipelineInfo.pVertexInputState = vertexInputInfo;
				pipelineInfo.pInputAssemblyState = inputAssembly;
				pipelineInfo.pViewportState = viewportState;
				pipelineInfo.pRasterizationState = rasterizer;
				pipelineInfo.pMultisampleState = multisampling;
				pipelineInfo.pDepthStencilState = depthStencil; // Optional
				pipelineInfo.pColorBlendState = colorBlending;
				pipelineInfo.pDynamicState = dynamicState; // Optional
			
				pipelineInfo.layout = pipelineLayout;
			
				pipelineInfo.renderPass = renderPass;
				pipelineInfo.subpass = subpass;

				pipelineInfo.basePipelineHandle = basePipelineHandle; // Optional
				pipelineInfo.basePipelineIndex = basePipelineIndex; // Optional
			
				return pipelineInfo;
			}

			static VkRenderPassCreateInfo RenderPassInfo(const std::vector<VkAttachmentDescription>& attachments, const VkSubpassDescription& subpasses, const std::vector<VkSubpassDependency>& dependenies)
			{
				VkRenderPassCreateInfo renderPassInfo{};
				renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
				renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
				renderPassInfo.pAttachments = attachments.data();
				IS_TODO("Look into why this is not allowing a vector. RenderPassInfo().")
				renderPassInfo.subpassCount = 1;
				renderPassInfo.pSubpasses = &subpasses;
				renderPassInfo.dependencyCount = static_cast<uint32_t>(dependenies.size());
				renderPassInfo.pDependencies = dependenies.data();
				return renderPassInfo;
			}

			static VkFramebufferCreateInfo FramebufferInfo(const VkRenderPass& renderpass, const std::vector<VkImageView>& imageViews, 
														   const int& width, const int& height, const int& layers = 1)
			{
				VkFramebufferCreateInfo framebufferInfo{};
				framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				framebufferInfo.renderPass = renderpass;
				framebufferInfo.attachmentCount = static_cast<uint32_t>(imageViews.size());
				framebufferInfo.pAttachments = imageViews.data();
				framebufferInfo.width = static_cast<uint32_t>(width);
				framebufferInfo.height = static_cast<uint32_t>(height);
				framebufferInfo.layers = layers;
				return framebufferInfo;
			}

			static VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo(const std::vector<VkDescriptorSetLayoutBinding>& bindings)
			{
				VkDescriptorSetLayoutCreateInfo layoutInfo{};
				layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
				layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
				layoutInfo.pBindings = bindings.data();
				return layoutInfo;
			}

			static VkDescriptorSetLayoutBinding DescriptorSetLayoutBinding()
			{
				VkDescriptorSetLayoutBinding uboLayoutBinding{};
				return uboLayoutBinding;
			}

			static VkDescriptorPoolSize DescPoolSize(const int& count)
			{
				VkDescriptorPoolSize poolSize = {};
				poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				poolSize.descriptorCount = count;

				return poolSize;
			}

			static VkDescriptorPoolCreateInfo DescPoolCreateInfo(const int& count, VkDescriptorPoolSize poolSize, const int& maxSets)
			{
				VkDescriptorPoolCreateInfo poolInfo = {};
				poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
				poolInfo.poolSizeCount = count;
				poolInfo.pPoolSizes = &poolSize;
				poolInfo.maxSets = maxSets;

				return poolInfo;
			}

			static VkDescriptorSetAllocateInfo DescriptorSetAllocInfo(const VkDescriptorPool* pool, const int& setCount, 
				const std::vector<VkDescriptorSetLayout>& layouts)
			{
				VkDescriptorSetAllocateInfo allocInfo{};
				allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
				allocInfo.descriptorPool = *pool;
				allocInfo.descriptorSetCount = static_cast<uint32_t>(setCount);
				allocInfo.pSetLayouts = layouts.data();
				return allocInfo;
			}

			static VkWriteDescriptorSet WriteDescriptorSet(const VkDescriptorSet& descriptorSets, const int& binding, const int& arrayElement,
				const VkDescriptorType type, const int& descriptorCount, const VkDescriptorBufferInfo* bufferInfo)
			{
				VkWriteDescriptorSet descriptorWrite{};
				descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite.dstSet = descriptorSets;
				descriptorWrite.dstBinding = binding;
				descriptorWrite.dstArrayElement = arrayElement;
				descriptorWrite.descriptorType = type;
				descriptorWrite.descriptorCount = descriptorCount;
				descriptorWrite.pBufferInfo = bufferInfo;
				descriptorWrite.pImageInfo = nullptr; // Optional
				descriptorWrite.pTexelBufferView = nullptr; // Optional
				return descriptorWrite;
			}

			static VkCommandPoolCreateInfo CommandPoolInfo(const VkCommandPoolCreateFlags& createFlag, const int& queueFailyIndex)
			{
				VkCommandPoolCreateInfo poolInfo{};
				poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
				poolInfo.queueFamilyIndex = queueFailyIndex;
				poolInfo.flags = createFlag; // Optional
				return poolInfo;
			}

			static VkCommandBufferAllocateInfo CommandBufferAllocInfo(const VkCommandPool& commandPool, const int& commandBufferSize)
			{
				VkCommandBufferAllocateInfo allocInfo{};
				allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
				allocInfo.commandPool = commandPool;
				allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
				allocInfo.commandBufferCount = commandBufferSize;
				return allocInfo;
			}

			static VkCommandBufferBeginInfo CommandBufferBeginInfo(const VkCommandBufferUsageFlags& usageFlags, 
																   const VkCommandBufferInheritanceInfo* inheritanceInfo)
			{
				VkCommandBufferBeginInfo beginInfo{};
				beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				beginInfo.flags = usageFlags; // Optional
				beginInfo.pInheritanceInfo = inheritanceInfo; // Optional
				return beginInfo;
			}

			static VkRenderPassBeginInfo RenderPassBeginInfo(const VkRenderPass& renderpass, const VkFramebuffer& framebuffer, const VkExtent2D extent, 
				const std::vector<VkClearValue>& clearColours)
			{
				VkRenderPassBeginInfo renderPassInfo{};
				renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				renderPassInfo.renderPass = renderpass;
				renderPassInfo.framebuffer = framebuffer;
				renderPassInfo.renderArea.offset = { 0, 0 };
				renderPassInfo.renderArea.extent = extent;
				renderPassInfo.clearValueCount = static_cast<uint32_t>(clearColours.size());
				renderPassInfo.pClearValues = clearColours.data();
				return renderPassInfo;
			}

			static VkSemaphoreCreateInfo SemaphoreInfo()
			{
				VkSemaphoreCreateInfo semaphoreInfo{};
				semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
				return semaphoreInfo;
			}

			static VkFenceCreateInfo FenceInfo()
			{
				VkFenceCreateInfo fenceInfo{};
				fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
				fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
				return fenceInfo;
			}

			static VkSubmitInfo SubmitInfo(const std::vector<VkSemaphore>& waitSemaphore, const std::vector<VkPipelineStageFlags>& pipelineStageFlags,
				const std::vector<VkCommandBuffer>& commandBuffers, const std::vector<VkSemaphore>& signalSemaphore)
			{
				VkSubmitInfo submitInfo{};
				submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
				submitInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphore.size());
				submitInfo.pWaitSemaphores = waitSemaphore.data();
				submitInfo.pWaitDstStageMask = pipelineStageFlags.data();
				submitInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());
				submitInfo.pCommandBuffers = commandBuffers.data();
				submitInfo.signalSemaphoreCount = static_cast<uint32_t>(signalSemaphore.size());
				submitInfo.pSignalSemaphores = signalSemaphore.data();
				return submitInfo;
			}

			static VkPresentInfoKHR PresnetInfo(const std::vector<VkSemaphore>& signalSemaphore, const std::vector<VkSwapchainKHR>& swapchain, const uint32_t& imageIndex)
			{
				VkPresentInfoKHR presentInfo{};
				presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
				presentInfo.waitSemaphoreCount = static_cast<uint32_t>(signalSemaphore.size());
				presentInfo.pWaitSemaphores = signalSemaphore.data();
				presentInfo.swapchainCount = static_cast<uint32_t>(swapchain.size());
				presentInfo.pSwapchains = swapchain.data();
				presentInfo.pImageIndices = &imageIndex;
				presentInfo.pResults = nullptr; // Optional

				return presentInfo;
			}

			static VkBufferCreateInfo BufferInfo()
			{
				VkBufferCreateInfo bufferInfo{};
				bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
				return bufferInfo;
			}

			static VkDebugUtilsMessengerCreateInfoEXT DebugCreateInfoEXT(PFN_vkDebugUtilsMessengerCallbackEXT callback)
			{
				VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
				createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
				createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
				createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
				createInfo.pfnUserCallback = callback;
				createInfo.pUserData = nullptr; // Optional

				return createInfo;
			}

			static VkSamplerCreateInfo Sampler()
			{
				VkSamplerCreateInfo samplerCreateInfo{};
				samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
				samplerCreateInfo.maxAnisotropy = 1.0f;
				return samplerCreateInfo;
			}
		};
	}
}