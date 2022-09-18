#if defined(IS_DX12_ENABLED)

#include "Graphics/RHI/DX12/RHI_Buffer_DX12.h"
#include "Graphics/RHI/DX12/DX12Utils.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::DX12
		{
			RHI_Buffer_DX12::~RHI_Buffer_DX12()
			{
				Release();
			}
			
			void RHI_Buffer_DX12::Create(RenderContext* context, BufferType bufferType, u64 sizeBytes, u64 stride, RHI_Buffer_Overrides overrides)
			{
				m_context = static_cast<RenderContext_DX12*>(context);
				m_bufferType = bufferType;
				m_size = sizeBytes;
				m_stride = stride;

				CD3DX12_HEAP_PROPERTIES heapProperties = {};
				CD3DX12_RESOURCE_DESC resourceDesc = {};

				D3D12_RESOURCE_STATES resouceStates = D3D12_RESOURCE_STATE_GENERIC_READ;
				if (bufferType == BufferType::Vertex 
					|| bufferType == BufferType::Index)
				{
					heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
					resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(m_size); /// CB size is required to be 256-byte aligned.
					resouceStates = D3D12_RESOURCE_STATE_COPY_DEST;
				}
				else if (bufferType == BufferType::Uniform)
				{
					heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
					m_size += (D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1) & ~(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1); /// must be a multiple 256 bytes
					resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(m_size); /// CB size is required to be 256-byte aligned.
				}
				else if (bufferType == BufferType::Staging)
				{
					heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
					resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(m_size); /// CB size is required to be 256-byte aligned.
					resouceStates = D3D12_RESOURCE_STATE_GENERIC_READ;
				}

				/// Create the constant buffer.
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
					CD3DX12_RANGE readRange(0, 0);        /// We do not intend to read from this resource on the CPU.
					ThrowIfFailed(m_resource->Map(0, &readRange, reinterpret_cast<void**>(&m_mappedData)));
				}
			}

			RHI_BufferView RHI_Buffer_DX12::Upload(const void* data, u64 sizeInBytes, u64 offset)
			{
				if (m_mappedData)
				{
					Platform::MemCopy(m_mappedData + offset, data, sizeInBytes);
				}
				else
				{
					/// We need a staging buffer to upload data from CPU to GPU.
					RHI_Buffer_DX12 stagingBuffer;
					stagingBuffer.Create(m_context, BufferType::Staging, sizeInBytes, 0, { });
					stagingBuffer.Upload(data, sizeInBytes, 0);

					RHI_CommandList* cmdList = m_context->GetCommandListManager().GetCommandList();
					cmdList->CopyBufferToBuffer(this, &stagingBuffer, offset);
					cmdList->Close();

					m_context->SubmitCommandListAndWait(cmdList);
					m_context->GetCommandListManager().ReturnCommandList(cmdList);

					stagingBuffer.Release();
				}
				return RHI_BufferView(this, offset, sizeInBytes);
			}

			std::vector<Byte> RHI_Buffer_DX12::Download()
			{
				return std::vector<Byte>();
			}

			void RHI_Buffer_DX12::Resize(u64 newSizeInBytes)
			{
				std::vector<Byte> data = Download();
				const u64 data_size = GetSize();

				Release();
				Create(m_context, m_bufferType, newSizeInBytes, m_stride, m_overrides);

				Upload(data.data(), data_size, 0);
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
		}
	}
}

#endif /// if defined(IS_DX12_ENABLED)