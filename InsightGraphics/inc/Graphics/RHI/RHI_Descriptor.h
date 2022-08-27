#pragma once

#include "Core/TypeAlias.h"
#include "Graphics/RHI/RHI_Resource.h"
#include "Graphics/RHI/RHI_Buffer.h"
#include "Graphics/Enums.h"
#include "Graphics/DescriptorBuffer.h"
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


		class RHI_DescriptorLayout : public RHI_Resource
		{
		public:
			static RHI_DescriptorLayout* New();

		protected:
			virtual void Create(RenderContext* context, int set, std::vector<Descriptor> descriptors) = 0;

			friend class RHI_DescriptorLayoutManager;
		};

		class RHI_DescriptorLayoutManager
		{
		public:
			RHI_DescriptorLayoutManager();
			~RHI_DescriptorLayoutManager();

			void SetRenderContext(RenderContext* context) { m_context = context; }

			RHI_DescriptorLayout* GetLayout(std::vector<Descriptor> descriptors);

			void ReleaseAll();

		private:
			std::unordered_map<u64, RHI_DescriptorLayout*> m_layouts;
			RenderContext* m_context{ nullptr };
		};

		class RHI_DescriptorSet : public RHI_Resource
		{
		public:
			RHI_DescriptorSet() = default;
			RHI_DescriptorSet(RenderContext* context, const std::vector<Descriptor>& descriptors, RHI_DescriptorLayout* layout);

			void* GetResource() const { return m_resource; }

			// RHI_Resource
			virtual void Release() override;
			virtual bool ValidResouce() override;
			virtual void SetName(std::wstring name) override;

		private:
			void Create(RHI_DescriptorLayout* layout);
			void Update(const std::vector<Descriptor>& descriptors);

		private:
			RenderContext* m_context = nullptr;
			u64 m_currentDescriptorHash = 0;
			void* m_resource = nullptr;

			friend class RHI_DescriptorSetManager;
		};

		class RHI_DescriptorSetManager
		{
		public:

			RHI_DescriptorSet* GetSet(const std::vector<Descriptor>& descriptors);
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

			void SetUniform(int set, int binding, void* data, u32 size);
			void SetTexture(int set, int binding, RHI_Texture* texture);

			Descriptor GetDescriptor(int set, int binding);

			void SetRenderContext(RenderContext* context);
			bool GetDescriptorSets(std::vector<RHI_DescriptorSet*>& descriptors);

			void ClearDescriptors();
			void Reset();
			void Destroy();

		private:
			void CreateUniformBufferIfNoExist();

		protected:
			std::unordered_map<u32, std::vector<Descriptor>> m_descriptors; // Current descriptors information. 
			UPtr<RHI_Buffer> m_uniformBuffer;
			u64 m_uniformBufferOffset = 0;

		private:
			RenderContext* m_context = nullptr;
		};
	}
}