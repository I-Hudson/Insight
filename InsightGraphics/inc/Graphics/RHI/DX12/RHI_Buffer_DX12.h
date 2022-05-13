#pragma once

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

				virtual void Create(RenderContext* context, BufferType bufferType, u64 sizeBytes) override;
				virtual RHI_BufferView Upload(void* data, int sizeInBytes, int offset) override;
				virtual std::vector<Byte> Download() override;

				virtual void Release() override;
				virtual bool ValidResouce() override;
				virtual void SetName(std::wstring name) override;

				ID3D12Resource* GetResouce() const { return m_resource.Get(); }

			protected:
				virtual void Resize(int newSizeInBytes) override;

			private:
				RenderContext_DX12* m_context = nullptr;
				Byte* m_mappedData = nullptr;
				ComPtr<ID3D12Resource> m_resource;
			};
		}
	}
}