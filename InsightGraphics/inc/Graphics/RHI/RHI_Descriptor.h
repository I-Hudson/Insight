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


			RHI_DescriptorLayout* GetLayout(int set, std::vector<Descriptor> descriptors);

			void ReleaseAll();

		private:
			std::unordered_map<u64, RHI_DescriptorLayout*> m_layouts;
			RenderContext* m_context{ nullptr };
		};

		class RHI_Descriptor : public RHI_Resource
		{
		public:
			static RHI_Descriptor* New();

			virtual void Update(const std::vector<Descriptor>& descriptors) = 0;
			virtual u64 GetHash(bool includeResouce = false) = 0;
		};

		class DescriptorAllocator
		{
		public:

			void SetPipeline(PipelineStateObject pso);

			void SetUniform(int set, int binding, RHI_BufferView view);
			void SetTexture(int set, int binding, RHI_Texture* texture);

			Descriptor GetDescriptor(int set, int binding);

			virtual void SetRenderContext(RenderContext* context) = 0;
			virtual bool GetDescriptors(std::vector<RHI_Descriptor*>& descriptors) = 0;

			virtual void Reset() = 0;
			virtual void Destroy() = 0;

		protected:
			std::unordered_map<u32, std::vector<Descriptor>> m_descriptors; // Current descriptors information. 
		
		private:
			RenderContext* m_context = nullptr;
		};
	}
}