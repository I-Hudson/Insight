#pragma once

#include "Core/TypeAlias.h"
#include "Graphics/RHI/RHI_Resource.h"
#include "Graphics/RHI/RHI_Buffer.h"
#include "Graphics/Enums.h"
#include "Graphics/DescriptorBuffer.h"
#include <unordered_map>

namespace Insight
{
	namespace Graphics
	{
		class RenderContext;
		class RHI_DescriptorLayoutManager;

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

			virtual void Update(std::vector<Descriptor> descriptors) = 0;
		};

		class RHI_DescriptorManager
		{
		public:
			RHI_DescriptorManager();
			~RHI_DescriptorManager();

			void SetRenderContext(RenderContext* context) { m_context = context; }
			
			std::unordered_map<int, RHI_Descriptor*> GetDescriptors(const DescriptorBuffer& buffer);

			void ReleaseAll();

		private:
			std::unordered_map<int, RHI_Descriptor*> GetDescriptors(std::unordered_map<int, std::vector<Descriptor>> descriptors);

		private:
			std::unordered_map<u64, RHI_Descriptor*> m_descriptors;
			RenderContext* m_context{ nullptr };
		};


	}
}