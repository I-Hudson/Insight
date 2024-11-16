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
			class RHI_CommandList_DX12;

			class RHI_Buffer_DX12 : public RHI_Buffer
			{
			public:
				virtual ~RHI_Buffer_DX12() override;

				ID3D12Resource* GetResource() const { return m_resource; }
				D3D12_RESOURCE_STATES GetResourceState() const;
				void SetResourceState(D3D12_RESOURCE_STATES resourceState);

				// RHI_Buffer
				virtual void Create(RenderContext* context, BufferType bufferType, u64 sizeBytes, u64 stride, RHI_Buffer_Overrides overrides) override;
				virtual RHI_BufferView Upload(const void* data, u64 sizeInBytes, u64 offset, u64 alignment) override;
				virtual RHI_BufferView Upload(RHI_Buffer* srcBuffer) override;
				virtual std::vector<Byte> Download() override;
				virtual void Resize(u64 newSizeBytes) override;

				// RHI_Resource
				virtual void Release() override;
				virtual bool ValidResource() override;
				virtual void SetName(std::string name) override;


			private:
				RenderContext_DX12* m_context = nullptr;
				D3D12_RESOURCE_STATES m_currentResouceState;
				ID3D12Resource* m_resource;
				D3D12MA::Allocation* m_d3d12maAllocation;

				friend class RHI_CommandList_DX12;
			};
		}
	}
}

#endif /// if defined(IS_DX12_ENABLED)