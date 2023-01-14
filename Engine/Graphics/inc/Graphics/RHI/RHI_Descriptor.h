#pragma once

#include "Core/TypeAlias.h"
#include "Graphics/RHI/RHI_Resource.h"
#include "Graphics/RHI/RHI_Buffer.h"
#include "Graphics/Enums.h"
#include "Graphics/Descriptors.h"
#include "Graphics/PipelineStateObject.h"
#include <unordered_map>

namespace Insight
{
	namespace Graphics
	{
		class RenderContext;
		class RHI_DescriptorLayoutManager;
		class RHI_DescriptorSetManager;
		class RHI_Texture;
		class RHI_Sampler;

		constexpr u32 c_VulkanTextureBindingShift = 1000;
		constexpr u32 c_VulkanUnorderedBindingShift = 2000;
		constexpr u32 c_VulkanSamplerBindingShift = 0;

		namespace RHI::DX12
		{
			class DescriptorHeapGPU_DX12;
		}

		class RHI_DescriptorLayout : public RHI_Resource
		{
		public:
			static RHI_DescriptorLayout* New();

		protected:
			virtual void Create(RenderContext* context, int set, DescriptorSet descriptor_set) = 0;

			friend class RHI_DescriptorLayoutManager;
		};

		class RHI_DescriptorLayoutManager
		{
		public:
			RHI_DescriptorLayoutManager();
			~RHI_DescriptorLayoutManager();

			void SetRenderContext(RenderContext* context) { m_context = context; }

			RHI_DescriptorLayout* GetLayout(const DescriptorSet& descriptor_set);

			void ReleaseAll();

		private:
			std::unordered_map<u64, RHI_DescriptorLayout*> m_layouts;
			RenderContext* m_context{ nullptr };
		};

		class RHI_DescriptorSet : public RHI_Resource
		{
		public:
			RHI_DescriptorSet() = default;
			RHI_DescriptorSet(RenderContext* context, const DescriptorSet& descriptor_set, RHI_DescriptorLayout* layout);

			void* GetResource() const { return m_resource; }

			/// RHI_Resource
			virtual void Release() override;
			virtual bool ValidResource() override;
			virtual void SetName(std::string name) override;

		private:
			void Create(RHI_DescriptorLayout* layout);
			void Update(const DescriptorSet& descriptor_set);

		private:
			RenderContext* m_context = nullptr;
			u64 m_currentDescriptorHash = 0;
			void* m_resource = nullptr;

			friend class RHI_DescriptorSetManager;
		};

#define DESCRIPTOR_CACHE_UNIFOM_DATA
#ifdef DESCRIPTOR_CACHE_UNIFOM_DATA
#define DESCRIPTOR_CACHE_UNIFOM_DATA_4_BYTE
#endif 

		class RHI_DescriptorSetManager
		{
		public:

			RHI_DescriptorSet* GetSet(const DescriptorSet& descriptor_set);
			void Reset();
			
			void ReleaseAll();

		private:
			std::unordered_map<u64, std::list<RHI_DescriptorSet*>> m_freeSets;
			std::unordered_map<u64, std::unordered_map<u64, RHI_DescriptorSet*>> m_usedSets;
		};

		class DescriptorAllocator
		{
		public:
			DescriptorAllocator();

			void SetPipeline(PipelineStateObject pso);

			bool WasUniformBufferResized() const;
			RHI_BufferView UploadUniform(const void* data, u32 size);

			void SetUniform(u32 set, u32 binding, const void* data, u32 size);
			void SetUniform(u32 set, u32 binding, RHI_BufferView buffer_view);
			void SetTexture(u32 set, u32 binding, const RHI_Texture* texture, const RHI_Sampler* sampler);
			void SetSampler(u32 set, u32 binding, const RHI_Sampler* sampler);

			std::vector<DescriptorSet> const& GetAllocatorDescriptorSets() const;

			void SetRenderContext(RenderContext* context);
			bool GetDescriptorSets(std::vector<RHI_DescriptorSet*>& descriptors);
			std::vector<u32> GetDynamicOffsets() const;

			void ClearDescriptors();
			void Reset();
			void Destroy();

		private:
			void CreateUniformBufferIfNoExist();
			bool CheckSetAndBindingBounds(u32 set, u32 binding);

			DescriptorSet* GetDescriptorSet(u32 set);
			DescriptorBinding* GetDescriptorBinding(DescriptorSet* set, u32 binding, DescriptorType descriptorType);

		protected:
			std::vector<DescriptorSet> m_descriptor_sets; // Current descriptors information. 
			UPtr<RHI_Buffer> m_uniformBuffer;
#ifdef DESCRIPTOR_CACHE_UNIFOM_DATA
			/// @brief Store a map of hash to buffer values to reduce the amount of copied data uploaded.
			std::unordered_map<u64, RHI_BufferView> m_cachedBufferData;
#endif
			bool m_uniformBufferResized = false;
			u64 m_uniformBufferOffset = 0;

		private:
			RenderContext* m_context = nullptr;
		};
	}
}