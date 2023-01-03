#pragma once

#if defined(IS_DX12_ENABLED)

#include "Graphics/RHI/RHI_Texture.h"
#include "Graphics/RHI/DX12/RenderContext_DX12.h"
#include "Graphics/RHI/DX12/RHI_PhysicalDevice_DX12.h"
#include "Graphics/RHI/DX12/RHI_Descriptor_DX12.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::DX12
		{
			class RHI_Texture_DX12 : public RHI_Texture
			{
			public:
				virtual ~RHI_Texture_DX12() = default;

				ID3D12Resource* GetResource() const { return m_resource.Get(); }
				DescriptorHeapHandle_DX12 GetDescriptorHandle() const;

				/// RHI_Texture
				virtual void Create(RenderContext* context, RHI_TextureInfo createInfo) override;
				virtual void Upload(void* data, int sizeInBytes) override;
				virtual std::vector<Byte> Download(void* data, int sizeInBytes) override;

				/// RHI_Resource
				virtual void Release() override;
				virtual bool ValidResource() override;
				virtual void SetName(std::string name) override;

			private:
				DescriptorHeapHandle_DX12 CreateSharderResouceView(u32 mip_index, u32 mip_count, u32 layer_count, u32 layer_index);

			private:
				ComPtr<ID3D12Resource> m_resource;
				RenderContext_DX12* m_context = nullptr;

				DescriptorHeapHandle_DX12 m_allLayerDescriptorHandle;					// Image view for all layers.
				std::vector<DescriptorHeapHandle_DX12> m_singleLayerDescriptorHandle;	// Image view for each layer.

				friend class RenderContext_DX12;
			};
		}
	}
}

#endif /// if defined(IS_DX12_ENABLED)