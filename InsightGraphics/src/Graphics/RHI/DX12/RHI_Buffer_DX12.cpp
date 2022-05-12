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

				CD3DX12_HEAP_PROPERTIES heapProperties = {};
				CD3DX12_RESOURCE_DESC resourceDesc = {};

				D3D12_RESOURCE_STATES resouceStates = D3D12_RESOURCE_STATE_GENERIC_READ;
				if (bufferType == BufferType::Vertex 
					|| bufferType == BufferType::Index)
				{
					heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
					resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(m_size); // CB size is required to be 256-byte aligned.
					resouceStates = D3D12_RESOURCE_STATE_COPY_DEST;
				}
				else if (bufferType == BufferType::Uniform)
				{
					heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
					m_size += (D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1) & ~(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1); // must be a multiple 256 bytes
					resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(m_size); // CB size is required to be 256-byte aligned.
				}
				else if (bufferType == BufferType::Staging)
				{
					heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
					resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(m_size); // CB size is required to be 256-byte aligned.
					resouceStates = D3D12_RESOURCE_STATE_GENERIC_READ;
				}

				// Create the constant buffer.
				ThrowIfFailed(m_context->GetDevice()->CreateCommittedResource(
					&heapProperties,
					D3D12_HEAP_FLAG_NONE,
					&resourceDesc,
					resouceStates,
					nullptr,
					IID_PPV_ARGS(&m_resource)));

				if (m_bufferType == BufferType::Uniform
					|| m_bufferType == BufferType::Storage
					|| m_bufferType == BufferType::Raw
					|| m_bufferType == BufferType::Staging
					|| m_bufferType == BufferType::Readback)
				{
					CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
					ThrowIfFailed(m_resource->Map(0, &readRange, reinterpret_cast<void**>(&m_mappedData)));
				}
			}

			RHI_BufferView RHI_Buffer_DX12::Upload(void* data, int sizeInBytes, int offset)
			{
				if (m_mappedData)
				{
					Platform::MemCopy(m_mappedData + offset, data, sizeInBytes);
				}
				else
				{
					// We need a staging buffer to upload data from CPU to GPU.
					RHI_Buffer_DX12 stagingBuffer;
					stagingBuffer.Create(m_context, BufferType::Staging, sizeInBytes);
					stagingBuffer.Upload(data, sizeInBytes, offset);

					RHI_CommandList* cmdList = m_context->GetFrameResouce().CommandListManager.GetSingleUseCommandList();
					cmdList->CopyBufferToBuffer(this, &stagingBuffer);
					cmdList->Close();

					m_context->SubmitCommandListAndWait(cmdList);
					m_context->GetFrameResouce().CommandListManager.ReturnSingleUseCommandList(cmdList);

					stagingBuffer.Release();
				}
				return RHI_BufferView(this, offset, sizeInBytes);
			}

			std::vector<Byte> RHI_Buffer_DX12::Download()
			{
				return std::vector<Byte>();
			}

			void RHI_Buffer_DX12::Release()
			{
				if (m_resource)
				{
					if (m_mappedData)
					{
						m_resource->Unmap(0, nullptr);
						m_mappedData = nullptr;
					}
					m_resource.Reset();
				}
			}

			bool RHI_Buffer_DX12::ValidResouce()
			{
				return m_resource;
			}

			void RHI_Buffer_DX12::SetName(std::wstring name)
			{
				if (m_resource)
				{
					m_resource->SetName(name.c_str());
				}
			}

			void RHI_Buffer_DX12::Resize(int newSizeInBytes)
			{
				if (m_resource && m_size < newSizeInBytes)
				{
					std::vector<Byte> data = Download();
					Release();
					Create(m_context, m_bufferType, newSizeInBytes);
					Upload(data.data(), (int)data.size(), 0);
				}
			}
		}
	}
}