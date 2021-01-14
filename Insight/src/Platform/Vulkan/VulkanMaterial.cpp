#include "ispch.h"
#include "VulkanMaterial.h"
#include "VulkanDevice.h"
#include "VulkanTexture.h"

namespace vks
{
	VulkanMaterial::VulkanMaterial()
	{
	}

	VulkanMaterial::~VulkanMaterial()
	{
		CleanUpUniformBuffers();
	}

	void VulkanMaterial::CreateDefault()
	{
		m_init = true;
		m_device = VulkanDevice::Instance();
		std::vector<std::string> shaders = { "./data/shaders/vulkan/default.vert", "./data/shaders/vulkan/default.frag" };
		for (auto& shader : shaders)
		{
			m_shaderData.push_back(Insight::ShaderParser::ParseShader(shader));
		}
		m_pipeline.Create(m_device, shaders, m_device->GetRenderPass(), m_shaderData);

		SetupUniformBuffers();
	}

	void VulkanMaterial::Create(VulkanDevice* device, const std::vector<std::string>& shaders, VkRenderPass& renderPass)
	{
		m_init = true;
		m_device = device;
		for (auto& shader : shaders)
		{
			m_shaderData.push_back(Insight::ShaderParser::ParseShader(shader));
		}
		m_pipeline.Create(device, shaders, renderPass, m_shaderData);

		SetupUniformBuffers();
	}

	void VulkanMaterial::Destroy()
	{
		m_init = false;
		CleanUpUniformBuffers();
	}

	void VulkanMaterial::Update()
	{
		IS_PROFILE_FUNCTION();

		if (m_init)
		{
			UpdateDescriptorSets();
		}
	}

	void VulkanMaterial::Bind(VkCommandBuffer commandBuffer, MaterialBlockData* materialBlockData, VkPipelineBindPoint bindPoint)
	{
		IS_PROFILE_FUNCTION();

		if (m_init)
		{
			std::vector<VkDescriptorSet> sets = GetDescriptorSets();
			m_pipeline.Bind(commandBuffer, bindPoint);

			std::vector<U32> dynamicOffsets;
			if (materialBlockData && materialBlockData->InUse)
			{
				dynamicOffsets.resize(materialBlockData->DynamicBuffers.size());
				int index = 0;
				for (auto& kvp : materialBlockData->DynamicBuffers)
				{
					dynamicOffsets[index++] = kvp.second.Index;
				}
			}
			else
			{
				for (auto& uniform : m_uniformBuffers)
				{
					if (uniform.second.Type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)
					{
						dynamicOffsets.push_back(0);
					}
				}
			}

			vkCmdBindDescriptorSets(commandBuffer, bindPoint, m_pipeline.GetPipelineLayout(), 0, static_cast<U32>(sets.size()), sets.data(), static_cast<U32>(dynamicOffsets.size()), dynamicOffsets.data());
		}
	}

	void VulkanMaterial::UploadUniform(const std::string& key, void* data, const U32& dataSize, MaterialBlockData& materialBlockData)
	{
		if (m_uniformBuffers.find(key) == m_uniformBuffers.end())
		{
			return;
		}

		MaterialUniformBuffer& mub = m_uniformBuffers[key];

		if (mub.Type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)
		{
			return;
			if (!materialBlockData.InUse)
			{
				materialBlockData = FindValidMaterialBlock(mub, key);
			}

			VkDeviceSize offset = materialBlockData.DynamicBuffers[key].Index * mub.DynamicUniformBlock.DynamicUniformAlign;
			void* dstPtr = (void*)((U64*)mub.DynamicUniformBlock.DynamicBuffer + offset);
			void* srcPtr = data;
			memcpy(dstPtr, srcPtr, dataSize);

			//TODO copy ram data to gpu and flush gpu buffer
			mub.Buffer.CopyTo(data, dataSize, offset);
			mub.Buffer.Flush();
		}
		else
		{
			mub.Buffer.CopyTo(data, dataSize);
		}
	}

	void VulkanMaterial::UploadTexture(const std::string& key, WeakPtr<Insight::Render::Texture> texture)
	{
		if (m_uniformTextures.find(key) == m_uniformTextures.end())
		{
			return;
		}

		if (auto textureSP = texture.lock())
		{
			if (textureSP->IsValid())
			{
				SharedPtr<VulkanTextureGPUData> vTex = DynamicPointerCast<VulkanTextureGPUData>(textureSP->GetGPUTextureData());
				m_uniformTextures[key].ImageInfo.imageView = vTex->GetImageView();
				m_uniformTextures[key].ImageInfo.sampler = vTex->GetSampler();
				m_uniformTextures[key].ImageInfo.imageLayout = vTex->GetImageLayout();
			}
		}
	}

	void VulkanMaterial::SetupUniformBuffers()
	{
		std::vector<VkWriteDescriptorSet> writeDescriptorSets;
		std::unordered_map<int, VkDescriptorSet> descriptorSets;

		int currentSet = -1;

		for (auto& shader : m_shaderData)
		{
			for (auto& uniformBlock : shader.UniformBlocks)
			{
				if (currentSet == -1)
				{
					currentSet = uniformBlock.Set;
				}
				else
				{
					if (uniformBlock.Set != currentSet)
					{
						currentSet = uniformBlock.Set;
					}
				}

				std::string key = uniformBlock.Name;
				if (uniformBlock.Type == Insight::ShaderUniformBlockType::UniformBuffer)
				{
					VulkanBuffer ubo;
					ThrowIfFailed(m_device->CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
														 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
														 uniformBlock.Size, &ubo));
					ThrowIfFailed(ubo.Map());

					m_uniformBuffers[key].Buffer = ubo;
					m_uniformBuffers[key].Binding = uniformBlock.Binding;
					m_uniformBuffers[key].Type = uniformBlock.GetVulkanType();
				}
				else if (uniformBlock.Type == Insight::ShaderUniformBlockType::UniformBufferDynamic)
				{
					VulkanBuffer ubo;
					m_uniformBuffers[key].Buffer = ubo;

					CalcDynamicUniformAlig(m_uniformBuffers[key].DynamicUniformBlock.DynamicUniformAlign, uniformBlock.Size);
					CreateDynamicUniformBuffer(m_uniformBuffers[key], m_uniformBuffers[key].DynamicUniformBlock.DynamicUniformAlign);

					m_uniformBuffers[key].Binding = uniformBlock.Binding;
					m_uniformBuffers[key].Type = uniformBlock.GetVulkanType();
				}
				else if (uniformBlock.Type == Insight::ShaderUniformBlockType::Sampler2D)
				{
					m_uniformTextures[key].ImageInfo.imageView = VK_NULL_HANDLE;
					m_uniformTextures[key].Binding = uniformBlock.Binding;
					m_uniformTextures[key].Type = uniformBlock.GetVulkanType();
				}

				if (descriptorSets.find(currentSet) != descriptorSets.end())
				{
					if (uniformBlock.Type == Insight::ShaderUniformBlockType::UniformBuffer || uniformBlock.Type == Insight::ShaderUniformBlockType::UniformBufferDynamic)
					{
						m_uniformBuffers[key].Set = descriptorSets[currentSet];
					}
					else if (uniformBlock.Type == Insight::ShaderUniformBlockType::Sampler2D)
					{
						m_uniformTextures[key].Set = descriptorSets[currentSet];
					}
					continue;
				}

				// Allocates an empty descriptor set without actual descriptors from the pool using the set layout
				VkDescriptorSetAllocateInfo allocateInfo{};
				allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
				allocateInfo.descriptorPool = m_device->GetDescriptorPool();
				allocateInfo.descriptorSetCount = 1;
				allocateInfo.pSetLayouts = &m_pipeline.GetDescriptorLayout(currentSet);
				ThrowIfFailed(vkAllocateDescriptorSets(*m_device, &allocateInfo, &descriptorSets[currentSet]));

				if (uniformBlock.Type == Insight::ShaderUniformBlockType::UniformBuffer || uniformBlock.Type == Insight::ShaderUniformBlockType::UniformBufferDynamic)
				{
					m_uniformBuffers[key].Set = descriptorSets[currentSet];
				}
				else if (uniformBlock.Type == Insight::ShaderUniformBlockType::Sampler2D)
				{
					m_uniformTextures[key].Set = descriptorSets[currentSet];
				}
			}
		}

		/*
			Execute the writes to update descriptors for this set
			Note that it's also possible to gather all writes and only run updates once, even for multiple sets
			This is possible because each VkWriteDescriptorSet also contains the destination set to be updated
		*/
		//vkUpdateDescriptorSets(*m_device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
		UpdateDescriptorSets();
	}
	void VulkanMaterial::CleanUpUniformBuffers()
	{
		for (auto& ubo : m_uniformBuffers)
		{
			ubo.second.Buffer.Unmap();
			ubo.second.Buffer.Destroy();

			if (ubo.second.DynamicUniformBlock.DynamicBuffer)
			{
				_aligned_free(ubo.second.DynamicUniformBlock.DynamicBuffer);
			}
		}
		std::vector<VkDescriptorSet> sets = GetDescriptorSets();
		vkFreeDescriptorSets(*m_device, m_device->GetDescriptorPool(), static_cast<uint32_t>(sets.size()), sets.data());
	}

	void VulkanMaterial::UpdateDescriptorSets()
	{
		std::vector<VkWriteDescriptorSet> writesSets;
		for (auto& ubo : m_uniformBuffers)
		{
			writesSets.push_back(vks::initializers::writeDescriptorSet(ubo.second.Set, ubo.second.Type, ubo.second.Binding, &ubo.second.Buffer.descriptor));
		}
		for (auto & tex : m_uniformTextures)
		{
			if (tex.second.ImageInfo.imageView != VK_NULL_HANDLE)
			{
				writesSets.push_back(vks::initializers::writeDescriptorSet(tex.second.Set, tex.second.Type, tex.second.Binding, &tex.second.ImageInfo));
			}
		}

		vkUpdateDescriptorSets(*m_device, static_cast<uint32_t>(writesSets.size()), writesSets.data(), 0, nullptr);
	}

	void VulkanMaterial::CalcDynamicUniformAlig(U64& v, const U64& uniformSize)
	{
		// Calculate required alignment based on minimum device offset alignment
		size_t minUboAlignment = VulkanDevice::Instance()->GetProperties().limits.minUniformBufferOffsetAlignment;
		v = uniformSize;
		if (minUboAlignment > 0)
		{
			v = (v + minUboAlignment - 1) & ~(minUboAlignment - 1);
		}
	}

	void VulkanMaterial::CreateDynamicUniformBuffer(MaterialUniformBuffer& materialBlock, const U64& newSize)
	{
		if (materialBlock.DynamicUniformBlock.DynamicBuffer != nullptr)
		{
			// Old data.
			void* oldArrPtr = materialBlock.DynamicUniformBlock.DynamicBuffer;
			U64 oldArrSize = materialBlock.DynamicUniformBlock.DynamicBufferSize;

			// New arr for data.
			materialBlock.DynamicUniformBlock.DynamicBuffer = Insight::Memory::MemoryManager::Instance()->AlignedAlloc(newSize, materialBlock.DynamicUniformBlock.DynamicUniformAlign);
			materialBlock.DynamicUniformBlock.DynamicBufferSize = newSize;

			// Copy and delete old data.
			memcpy_s(materialBlock.DynamicUniformBlock.DynamicBuffer, materialBlock.DynamicUniformBlock.DynamicBufferSize, oldArrPtr, oldArrSize);
			delete oldArrPtr;

			//VulkanDevice::Instance()->QueueIdleCommand([&]()
			//	{
					materialBlock.Buffer.Unmap();
					materialBlock.Buffer.Destroy();
			//	});
		}
		else
		{
			// New arr for data.
			materialBlock.DynamicUniformBlock.DynamicBuffer = Insight::Memory::MemoryManager::Instance()->AlignedAlloc(newSize, materialBlock.DynamicUniformBlock.DynamicUniformAlign);
			materialBlock.DynamicUniformBlock.DynamicBufferSize = newSize;
		}

		//VulkanDevice::Instance()->QueueIdleCommand([&]()
		//	{
				ThrowIfFailed(m_device->CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
					newSize, &materialBlock.Buffer));
				ThrowIfFailed(materialBlock.Buffer.Map());

				materialBlock.Buffer.CopyTo(materialBlock.DynamicUniformBlock.DynamicBuffer, newSize);
				materialBlock.Buffer.Flush();
		//	});
	}

	MaterialBlockData VulkanMaterial::FindValidMaterialBlock(MaterialUniformBuffer& materialUniformBuffer, const std::string& uniformKey)
	{
		MaterialUniformDynamicBlock& dynamicBlock = materialUniformBuffer.DynamicUniformBlock;
		U32 blockIndex = -1;
		for (U32 i = 0; i < dynamicBlock.DynamicBlocks.size(); ++i)
		{
			if (!dynamicBlock.DynamicBlocks[i].InUse)
			{
				blockIndex = i;
				break;
			}
		}

		if (blockIndex == -1)
		{
			U32 orginalVectorSize = static_cast<U32>(dynamicBlock.DynamicBlocks.size());
			if (dynamicBlock.DynamicBlocks.size() == 0)
			{
				dynamicBlock.DynamicBlocks.resize(1);
			}
			else
			{
				// Double the MaterialDynamicUniformInfo in the vector.
				dynamicBlock.DynamicBlocks.resize(orginalVectorSize * (U64)2);
			}
			blockIndex = orginalVectorSize;

			//TODO: Rebuild buffer as the size has changed.
			CreateDynamicUniformBuffer(materialUniformBuffer, dynamicBlock.DynamicUniformAlign * dynamicBlock.DynamicBlocks.size());
		}

		MaterialBlockData& materialBlockData = dynamicBlock.DynamicBlocks[blockIndex];
		materialBlockData.UUID = Insight::UUID::GenUUID_U128();
		materialBlockData.InUse = true;
		materialBlockData.DynamicBuffers.clear();
		materialBlockData.DynamicBuffers[uniformKey].InUse = true;
		materialBlockData.DynamicBuffers[uniformKey].Index = blockIndex;

		// Return a copy of the MaterialBlockData we have just populated.
		return dynamicBlock.DynamicBlocks[blockIndex];
	}

	std::vector<VkDescriptorSet> VulkanMaterial::GetDescriptorSets()
	{
		std::vector<VkDescriptorSet> sets;
		for (auto& ubo : m_uniformBuffers)
		{
			if (std::find(sets.begin(), sets.end(), ubo.second.Set) == sets.end())
			{
				sets.push_back(ubo.second.Set);
			}
		}
		return sets;
	}
}