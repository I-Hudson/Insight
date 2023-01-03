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
				return m_allLayerDescriptorHandle;
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
					m_infos.at(0).Layer_Count,
					m_infos.at(0).Mip_Count,
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
					ImageLayoutToDX12ResouceState(m_infos.at(0).Layout),
					nullptr,
					IID_PPV_ARGS(&m_resource)));

				m_allLayerDescriptorHandle = CreateSharderResouceView(0, 1, m_infos.at(0).Layer_Count, 0);

				// Create a image view for each layer. (Use image views when rendering to different layers).
				for (u32 i = 0; i < createInfo.Layer_Count; ++i)
				{
					m_singleLayerDescriptorHandle.push_back(CreateSharderResouceView(0, 1, 1, i));
				}
			}

			void RHI_Texture_DX12::Upload(void* data, int sizeInBytes)
			{
				m_uploadStatus = DeviceUploadStatus::Uploading;

				/// We need a staging buffer to upload data from CPU to GPU.
				RHI_Buffer_DX12 stagingBuffer;
				stagingBuffer.Create(m_context, BufferType::Staging, sizeInBytes, 0, { });
				stagingBuffer.Upload(data, sizeInBytes, 0, 0);

				RHI_CommandList* cmdList = m_context->GetCommandListManager().GetCommandList();
				cmdList->CopyBufferToImage(this, &stagingBuffer);
				cmdList->Close();

				m_context->SubmitCommandListAndWait(cmdList);
				m_context->GetCommandListManager().ReturnCommandList(cmdList);

				m_uploadStatus = DeviceUploadStatus::Completed;

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

				m_infos.clear();
				m_context->GetDescriptorHeap(DescriptorHeapTypes::CBV_SRV_UAV).FreeHandle(m_allLayerDescriptorHandle);
				for (size_t i = 1; i < m_singleLayerDescriptorHandle.size(); ++i)
				{
					m_context->GetDescriptorHeap(DescriptorHeapTypes::CBV_SRV_UAV).FreeHandle(m_singleLayerDescriptorHandle.at(i));
				}
				m_singleLayerDescriptorHandle.clear();
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

			DescriptorHeapHandle_DX12 RHI_Texture_DX12::CreateSharderResouceView(u32 mip_index, u32 mip_count, u32 layer_count, u32 layer_index)
			{
				DescriptorHeapHandle_DX12 handle = m_context->GetDescriptorHeap(DescriptorHeapTypes::CBV_SRV_UAV).GetNewHandle();

				D3D12_SHADER_RESOURCE_VIEW_DESC shaderResouceViewDesc = {};
				shaderResouceViewDesc.Format = PixelFormatToDX12(GetFormat());
				shaderResouceViewDesc.ViewDimension = TextureTypeToDX12(GetType());
				shaderResouceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
				if (GetType() == TextureType::Tex2D)
				{
					shaderResouceViewDesc.Texture2D.MipLevels = mip_count;
				}
				else if (GetType() == TextureType::Tex2DArray)
				{
					shaderResouceViewDesc.Texture2DArray.ArraySize = layer_count;
					shaderResouceViewDesc.Texture2DArray.FirstArraySlice = layer_index;
					shaderResouceViewDesc.Texture2DArray.MipLevels = mip_count;
					shaderResouceViewDesc.Texture2DArray.MostDetailedMip = 0u;
					shaderResouceViewDesc.Texture2DArray.PlaneSlice = 0u;
					shaderResouceViewDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
				}
				m_context->GetDevice()->CreateShaderResourceView(m_resource.Get(), &shaderResouceViewDesc, handle.CPUPtr);

				return handle;
			}
		}
	}
}

#endif /// if defined(IS_DX12_ENABLED)