#pragma once

#include "Core/TypeAlias.h"
#include "Graphics/RHI/RHI_Resource.h"
#include "Graphics/Enums.h"
#include <unordered_map>

namespace Insight
{
	namespace Graphics
	{
		class RenderContext;
		class RHI_DescriptorLayoutManager;

		struct Descriptor
		{
			Descriptor() { }
			Descriptor(int set, int binding, int stage, int size, DescriptorType type)
				: Set(set), Binding(binding), Stage(stage), Type(type)
			{ }

			int Set = 0;
			int Binding = 0;
			int Stage = 0;
			int Size = 0;
			DescriptorType Type = DescriptorType::Unknown;
			void* Resource = nullptr;

			u64 GetHash(bool includeResource) const
			{
				u64 hash = 0;

				HashCombine(hash, Set);
				HashCombine(hash, Binding);
				HashCombine(hash, Stage);
				HashCombine(hash, static_cast<u64>(Type));
				if (includeResource)
				{
					HashCombine(hash, Resource);
				}
				return hash;
			}
		};

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
		};

		class RHI_DescriptorManager
		{
		public:
			RHI_DescriptorManager();
			~RHI_DescriptorManager();

			void SetRenderContext(RenderContext* context) { m_context = context; }
			
			RHI_Descriptor* GetDescriptor(u64 hash);

			void ReleaseAll();

		private:
			std::unordered_map<u64, RHI_Descriptor*> m_descriptors;
			RenderContext* m_context{ nullptr };
		};
	}
}