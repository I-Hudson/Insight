#pragma once

#if defined(IS_DX12_ENABLED)

#include "Graphics/RHI/RHI_Buffer.h"
#include "Graphics/RHI/DX12/RenderContext_DX12.h"
#include "Graphics/RHI/DX12/RHI_PhysicalDevice_DX12.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::DX12
		{
			class RHI_Buffer_DX12 : public RHI_Buffer
			{
			public:
				virtual ~RHI_Buffer_DX12() override;

				virtual void Create(RenderContext* context, BufferType bufferType, u64 sizeBytes, u64 stride, RHI_Buffer_Overrides overrides) override;
				virtual RHI_BufferView Upload(const void* data, u64 sizeInBytes, u64 offset) override;
				virtual std::vector<Byte> Download() override;
				virtual void Resize(u64 newSizeBytes) override;

				virtual void Release() override;
				virtual bool ValidResource() override;
				virtual void SetName(std::wstring name) override;

				ID3D12Resource* GetResource() const { return m_resource.Get(); }

			private:
				RenderContext_DX12* m_context = nullptr;
				Byte* m_mappedData = nullptr;
				ComPtr<ID3D12Resource> m_resource;
			};
		}
	}
}

#endif /// if defined(IS_DX12_ENABLED)