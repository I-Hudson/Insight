#if defined(IS_DX12_ENABLED)

#include "Graphics/RHI/DX12/RHI_Texture_DX12.h"
#include "Graphics/RHI/DX12/RHI_Buffer_DX12.h"
#include "Graphics/RHI/DX12/DX12Utils.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::DX12
		{
			DescriptorHeapHandle_DX12 RHI_Texture_DX12::GetDescriptorHandle() const
			{
				return m_descriptorHandle;
			}

			void RHI_Texture_DX12::Create(RenderContext* context, RHI_TextureInfo createInfo)
			{
				m_context = static_cast<RenderContext_DX12*>(context);
				for (size_t i = 0; i < createInfo.Mip_Count; ++i)
				{
					m_infos.push_back(createInfo);
				}

				CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(
					PixelFormatToDX12(m_infos.at(0).Format),
					m_infos.at(0).Width,
					m_infos.at(0).Height,
					m_infos.at(0).Depth,
					0,
					1,
					0,
					D3D12_RESOURCE_FLAG_NONE,
					D3D12_TEXTURE_LAYOUT_UNKNOWN,
					0);

				/// Create the texture.
				ThrowIfFailed(m_context->GetDevice()->CreateCommittedResource(
					&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
					D3D12_HEAP_FLAG_NONE,
					&resourceDesc,
					D3D12_RESOURCE_STATE_COPY_DEST,
					nullptr,
					IID_PPV_ARGS(&m_resource)));

				m_descriptorHandle = m_context->GetDescriptorHeap(DescriptorHeapTypes::CBV_SRV_UAV).GetNewHandle();

				D3D12_SHADER_RESOURCE_VIEW_DESC shaderResouceViewDesc = {};
				shaderResouceViewDesc.Format = PixelFormatToDX12(GetFormat());
				shaderResouceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				shaderResouceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
				shaderResouceViewDesc.Texture2D.MipLevels = m_infos.at(0).Mip_Count;
				shaderResouceViewDesc.Texture2D.MostDetailedMip = 0;
				shaderResouceViewDesc.Texture2D.PlaneSlice = 0;
				shaderResouceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;
				m_context->GetDevice()->CreateShaderResourceView(m_resource.Get(), &shaderResouceViewDesc, m_descriptorHandle.CPUPtr);
			}

			void RHI_Texture_DX12::Upload(void* data, int sizeInBytes)
			{
				/// We need a staging buffer to upload data from CPU to GPU.
				RHI_Buffer_DX12 stagingBuffer;
				stagingBuffer.Create(m_context, BufferType::Staging, sizeInBytes, 0, { });
				stagingBuffer.Upload(data, sizeInBytes, 0);

				RHI_CommandList* cmdList = m_context->GetCommandListManager().GetCommandList();
				cmdList->CopyBufferToImage(this, &stagingBuffer);
				cmdList->Close();

				m_context->SubmitCommandListAndWait(cmdList);
				m_context->GetCommandListManager().ReturnCommandList(cmdList);

				stagingBuffer.Release();
			}

			std::vector<Byte> RHI_Texture_DX12::Download(void* data, int sizeInBytes)
			{
				return std::vector<Byte>();
			}

			void RHI_Texture_DX12::Release()
			{
				if (m_resource)
				{
					m_resource.Reset();
				}
			}

			bool RHI_Texture_DX12::ValidResource()
			{
				return m_resource;
			}

			void RHI_Texture_DX12::SetName(std::string name)
			{
				if (m_resource)
				{
					m_context->SetObjectName(name, m_resource.Get());
				}
				m_name = std::move(name);
			}
		}
	}
}

#endif /// if defined(IS_DX12_ENABLED)