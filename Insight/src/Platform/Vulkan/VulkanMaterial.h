#pragma once

#include "Insight/Core/Core.h"
#include "VulkanHeader.h"
#include "VulkanPipeline.h"
#include "VulkanBuffer.h"
#include "Insight/ShaderParser/ShaderParser.h"

#include <bitset>

namespace vks
{
	struct MaterialDynamicUniformInfo
	{
		U32 Index;
		bool InUse;

		MaterialDynamicUniformInfo()
			: InUse(false)
		{ }

		MaterialDynamicUniformInfo(U32 const& index, bool const& inUse)
			: Index(index)
			, InUse(inUse)
		{ }
	};

	struct MaterialBlockData
	{
		std::unordered_map<std::string, MaterialDynamicUniformInfo> DynamicBuffers;
		U128 UUID;
		bool InUse;

		MaterialBlockData()
			: InUse(false)
		{ }
	};

	struct MaterialUniformDynamicBlock
	{
		void* DynamicBuffer;
		U64 DynamicBufferSize;
		U64 DynamicUniformAlign;
		std::vector<MaterialBlockData> DynamicBlocks;

		MaterialUniformDynamicBlock()
			: DynamicUniformAlign(0)
			, DynamicBuffer(nullptr)
			, DynamicBufferSize(0)
		{ }
	};

	struct MaterialUniformBuffer
	{
		VulkanBuffer Buffer;
		VkDescriptorSet Set;
		uint32_t Binding;
		VkDescriptorType Type;

		MaterialUniformDynamicBlock DynamicUniformBlock;

		MaterialUniformBuffer()
			: Binding(-1)
		{ }
	};

	class IS_API VulkanMaterial
	{
	public:
		VulkanMaterial();
		~VulkanMaterial();

		void Create(VulkanDevice* device, const std::vector<std::string>& shaders, VkRenderPass& renderPass);
		void Destroy();

		void Update();
		void Bind(VkCommandBuffer commandBuffer, MaterialBlockData* materialBlockData = nullptr, VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS);

		template<typename T>
		void UploadUniform(const std::string& key, T data, MaterialBlockData& materialBlockData);

	private:
		void SetupUniformBuffers();
		void CleanUpUniformBuffers();
		void UpdateDescriptorSets();

		void CalcDynamicUniformAlig(U64& v, const U64& uniformSize);
		void CreateDynamicUniformBuffer(MaterialUniformBuffer& materialBlock, const U64& newSize);

		MaterialBlockData FindValidMaterialBlock(MaterialUniformBuffer& materialUniformBuffer, const std::string& uniformKey);

		std::vector<VkDescriptorSet> GetDescriptorSets();

	private:
		VulkanDevice* m_device;

		bool m_init;
		std::vector<Insight::ParsedShadeData> m_shaderData;
		
		U8 m_numberOfDescriptorSets;
		std::unordered_map<std::string, MaterialUniformBuffer> m_uniformBuffers;
		VulkanPipeline m_pipeline;
	};

	template<typename T>
	inline void VulkanMaterial::UploadUniform(const std::string& key, T data, MaterialBlockData& materialBlockData)
	{
		if (m_uniformBuffers.find(key) == m_uniformBuffers.end())
		{
			return;
		}

		MaterialUniformBuffer& mub = m_uniformBuffers[key];

		U64 dataSize = sizeof(T);
		if (mub.Type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)
		{
			return;
			if (!materialBlockData.InUse)
			{
				materialBlockData = FindValidMaterialBlock(mub, key);
			}

			VkDeviceSize offset = materialBlockData.DynamicBuffers[key].Index * mub.DynamicUniformBlock.DynamicUniformAlign;
			void* dstPtr = (void*)((U64*)mub.DynamicUniformBlock.DynamicBuffer + offset);
			void* srcPtr = (void*)(&data);
			memcpy(dstPtr, srcPtr, dataSize);

			//TODO copy ram data to gpu and flush gpu buffer
			mub.Buffer.CopyTo(&data, dataSize, offset);
			mub.Buffer.Flush();
		}
		else
		{
			mub.Buffer.CopyTo(&data, dataSize);
		}
	}
}