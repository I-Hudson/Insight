#include "Graphics/RHI/DX12/RHI_Buffer_DX12.h"
#include "Graphics/RHI/DX12/DX12Utils.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::DX12
		{
			void RHI_Buffer_DX12::Create(RenderContext* context, BufferType bufferType, u64 sizeBytes)
			{
				m_context = dynamic_cast<RenderContext_DX12*>(context);
				m_bufferType = bufferType;
				m_size = sizeBytes;

				// Create the constant buffer.
				const UINT constantBufferSize = sizeof(SceneConstantBuffer);    // CB size is required to be 256-byte aligned.

				ThrowIfFailed(m_context->GetDevice()->CreateCommittedResource(
					&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
					D3D12_HEAP_FLAG_NONE,
					&CD3DX12_RESOURCE_DESC::Buffer(constantBufferSize),
					D3D12_RESOURCE_STATE_GENERIC_READ,
					nullptr,
					IID_PPV_ARGS(&m_resource)));

				// Describe and create a constant buffer view.
				D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
				cbvDesc.BufferLocation = m_resource->GetGPUVirtualAddress();
				cbvDesc.SizeInBytes = constantBufferSize;
				m_context->GetDevice()->CreateConstantBufferView(&cbvDesc, m_cbvHeap->GetCPUDescriptorHandleForHeapStart());

				// Map and initialize the constant buffer. We don't unmap this until the
				// app closes. Keeping things mapped for the lifetime of the resource is okay.
				CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
				ThrowIfFailed(m_resource->Map(0, &readRange, reinterpret_cast<void**>(&m_pCbvDataBegin)));
				memcpy(m_pCbvDataBegin, &m_constantBufferData, sizeof(m_constantBufferData));

			}

			RHI_BufferView RHI_Buffer_DX12::Upload(void* data, int sizeInBytes, int offset)
			{
				return RHI_BufferView();
			}

			void RHI_Buffer_DX12::Release()
			{
			}

			void RHI_Buffer_DX12::Resize(int newSizeInBytes)
			{
			}
		}
	}
}