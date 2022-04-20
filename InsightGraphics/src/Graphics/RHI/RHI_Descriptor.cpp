#include "Graphics/RHI/RHI_Descriptor.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/RHI/DX12/RHI_Descriptor_DX12.h"

namespace Insight
{
	namespace Graphics
	{
		/// <summary>
		/// RH_DescriptorLayout
		/// </summary>
		/// <returns></returns>
		RHI_DescriptorLayout* RHI_DescriptorLayout::New()
		{
			if (GraphicsManager::IsVulkan()) { return nullptr; }
			else if (GraphicsManager::IsDX12()) { return NewTracked(RHI::DX12::RHI_DescriptorLayout_DX12); }
			return nullptr;
		}

		/// <summary>
		/// RHI_DescriptorLayoutManager
		/// </summary>
		RHI_DescriptorLayoutManager::RHI_DescriptorLayoutManager()
		{
		}

		RHI_DescriptorLayoutManager::~RHI_DescriptorLayoutManager()
		{
			ReleaseAll();
		}

		RHI_DescriptorLayout* RHI_DescriptorLayoutManager::GetLayout(int set, std::vector<Descriptor> descriptors)
		{
			u64 hash = 0;
			for (const Descriptor& descriptor : descriptors)
			{
				HashCombine(hash, descriptor.GetHash(false));
			}

			auto itr = m_layouts.find(hash);
			if (itr != m_layouts.end())
			{
				return itr->second;
			}

			RHI_DescriptorLayout* newLayout = RHI_DescriptorLayout::New();
			assert(newLayout);
			m_layouts[hash] = newLayout;
			newLayout->Create(m_context, set, descriptors);
			return newLayout;
		}

		void RHI_DescriptorLayoutManager::ReleaseAll()
		{
			for (auto& pair : m_layouts)
			{
				pair.second->Release();
				DeleteTracked(pair.second);
			}
			m_layouts.clear();
		}


		/// <summary>
		/// RHI_Descriptor
		/// </summary>
		/// <returns></returns>
		RHI_Descriptor* RHI_Descriptor::New()
		{
			return nullptr;
		}


		/// <summary>
		/// RHI_DescriptorManager
		/// </summary>
		RHI_DescriptorManager::RHI_DescriptorManager()
		{
		}

		RHI_DescriptorManager::~RHI_DescriptorManager()
		{
			ReleaseAll();
		}

		RHI_Descriptor* RHI_DescriptorManager::GetDescriptor(const DescriptorBuffer& buffer)
		{
			std::vector<Descriptor> descriptors;
			for (const auto& sets : buffer.GetUniforms())
			{
				const int setIndex = sets.first;
				for (const auto& binding : sets.second )
				{
					const int bindingIndex = binding.first;
					const int bindingSize = binding.second.Size;
					const void* resource = binding.second.Ptr;
					descriptors.push_back(Descriptor(setIndex, 
						bindingIndex, 
						0, 
						bindingSize, 
						DescriptorType::Unifom_Buffer, 
						DescriptorResourceType::CBV, 
						nullptr));
				}
			}
			return GetDescriptor(descriptors);
		}

		void RHI_DescriptorManager::ReleaseAll()
		{
			for (auto& pair : m_descriptors)
			{
				pair.second->Release();
				DeleteTracked(pair.second);
			}
			m_descriptors.clear();
		}

		RHI_Descriptor* RHI_DescriptorManager::GetDescriptor(std::vector<Descriptor> descriptors)
		{
			u64 hash = 0;
			for (const Descriptor& descriptor : descriptors)
			{
				HashCombine(hash, descriptor.GetHash(true));
			}

			auto itr = m_descriptors.find(hash);
			if (itr != m_descriptors.end())
			{

			}
			return nullptr;
		}
	}
}