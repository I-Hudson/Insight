#if defined(IS_DX12_ENABLED)

#include "Graphics/RHI/DX12/RHI_Texture_DX12.h"
#include "Graphics/RHI/DX12/RHI_Buffer_DX12.h"
#include "Graphics/RHI/DX12/DX12Utils.h"
#include "Graphics/PixelFormatExtensions.h"

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

			DescriptorHeapHandle_DX12 RHI_Texture_DX12::GetRenderTargetHandle() const
			{
				return m_allLayerRenderTargetHandle;
			}

			DescriptorHeapHandle_DX12 RHI_Texture_DX12::GetDepthStencilHandle() const
			{
				return m_allLayerDepthStencilHandle;
			}

			DescriptorHeapHandle_DX12 RHI_Texture_DX12::GetSingleLayerDescriptorHandle(u32 const index) const
			{
				if (index >= 0 && index < static_cast<u32>(m_singleLayerDescriptorHandle.size()))
				{
					return m_singleLayerDescriptorHandle.at(index);
				}
				return DescriptorHeapHandle_DX12();
			}

			DescriptorHeapHandle_DX12 RHI_Texture_DX12::GetSingleLayerRenderTargetHandle(u32 const index) const
			{
				if (index >= 0 && index < static_cast<u32>(m_singleLayerRenderTargetHandle.size()))
				{
					return m_singleLayerRenderTargetHandle.at(index);
				}
				return DescriptorHeapHandle_DX12();
			}

			DescriptorHeapHandle_DX12 RHI_Texture_DX12::GetSingleLayerDepthStencilHandle(u32 const index) const
			{
				if (index >= 0 && index < static_cast<u32>(m_singleLayerDepthStencilHandle.size()))
				{
					return m_singleLayerDepthStencilHandle.at(index);
				}
				return DescriptorHeapHandle_DX12();
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
					ImageUsageFlagsToDX12(m_infos.at(0).ImageUsage),
					D3D12_TEXTURE_LAYOUT_UNKNOWN,
					0);

				D3D12_CLEAR_VALUE defaultClearColour = {};
				defaultClearColour.Format = resourceDesc.Format;

				bool enableOptimizeClearColour = m_infos.at(0).ImageUsage & ImageUsageFlagsBits::DepthStencilAttachment 
					|| m_infos.at(0).ImageUsage & ImageUsageFlagsBits::ColourAttachment;
				if (m_infos.at(0).ImageUsage & ImageUsageFlagsBits::DepthStencilAttachment)
				{
					defaultClearColour.DepthStencil.Depth = RenderContext::Instance().IsRenderOptionsEnabled(RenderOptions::ReverseZ) ? 0.0f : 1.0f;
					defaultClearColour.DepthStencil.Stencil = 0;
				}
				if (m_infos.at(0).ImageUsage & ImageUsageFlagsBits::ColourAttachment)
				{
					FLOAT Color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
					Platform::MemCopy(defaultClearColour.Color, Color, sizeof(Color));
				}

				CD3DX12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
				/// Create the texture.
				ThrowIfFailed(m_context->GetDevice()->CreateCommittedResource(
					&heapProperties,
					D3D12_HEAP_FLAG_NONE,
					&resourceDesc,
					ImageLayoutToDX12ResouceState(m_infos.at(0).Layout),
					enableOptimizeClearColour ? &defaultClearColour : nullptr,
					IID_PPV_ARGS(&m_resource)));

				if (m_infos.at(0).ImageUsage & ImageUsageFlagsBits::Sampled)
				{
					m_allLayerDescriptorHandle = CreateSharderResouceView(0, 1, m_infos.at(0).Layer_Count, 0, DescriptorHeapTypes::CBV_SRV_UAV);
				}
				if (m_infos.at(0).ImageUsage & ImageUsageFlagsBits::DepthStencilAttachment)
				{
					m_allLayerDepthStencilHandle = CreateSharderResouceView(0, 1, m_infos.at(0).Layer_Count, 0, DescriptorHeapTypes::DepthStencilView);
				}
				if (m_infos.at(0).ImageUsage & ImageUsageFlagsBits::ColourAttachment)
				{
					m_allLayerRenderTargetHandle= CreateSharderResouceView(0, 1, m_infos.at(0).Layer_Count, 0, DescriptorHeapTypes::RenderTargetView);
				}
				// Create a image view for each layer. (Use image views when rendering to different layers).
				for (u32 i = 0; i < createInfo.Layer_Count; ++i)
				{
					if (m_infos.at(0).ImageUsage & ImageUsageFlagsBits::Sampled)
					{
						m_singleLayerDescriptorHandle.push_back(CreateSharderResouceView(0, 1, 1, i, DescriptorHeapTypes::CBV_SRV_UAV));
					}
					if (m_infos.at(0).ImageUsage & ImageUsageFlagsBits::DepthStencilAttachment)
					{
						m_singleLayerDepthStencilHandle.push_back(CreateSharderResouceView(0, 1, 1, i, DescriptorHeapTypes::DepthStencilView));
					}
					if (m_infos.at(0).ImageUsage & ImageUsageFlagsBits::ColourAttachment)
					{
						m_singleLayerRenderTargetHandle.push_back(CreateSharderResouceView(0, 1, 1, i, DescriptorHeapTypes::RenderTargetView));
					}
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
				m_context->GetDescriptorHeap(m_allLayerDescriptorHandle.HeapType).FreeHandle(m_allLayerDescriptorHandle);
				m_context->GetDescriptorHeap(m_allLayerRenderTargetHandle.HeapType).FreeHandle(m_allLayerRenderTargetHandle);
				m_context->GetDescriptorHeap(m_allLayerDepthStencilHandle.HeapType).FreeHandle(m_allLayerDepthStencilHandle);

				for (size_t i = 0; i < m_singleLayerDescriptorHandle.size(); ++i)
				{
					m_context->GetDescriptorHeap(m_singleLayerDescriptorHandle.at(i).HeapType).FreeHandle(m_singleLayerDescriptorHandle.at(i));
				}
				for (size_t i = 0; i < m_singleLayerRenderTargetHandle.size(); ++i)
				{
					m_context->GetDescriptorHeap(m_singleLayerRenderTargetHandle.at(i).HeapType).FreeHandle(m_singleLayerRenderTargetHandle.at(i));
				}
				for (size_t i = 0; i < m_singleLayerDepthStencilHandle.size(); ++i)
				{
					m_context->GetDescriptorHeap(m_singleLayerDepthStencilHandle.at(i).HeapType).FreeHandle(m_singleLayerDepthStencilHandle.at(i));
				}
				m_singleLayerDescriptorHandle.clear();
				m_singleLayerRenderTargetHandle.clear();
				m_singleLayerDepthStencilHandle.clear();
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

			DescriptorHeapHandle_DX12 RHI_Texture_DX12::CreateSharderResouceView(u32 mip_index, u32 mip_count, u32 layer_count, u32 layer_index, DescriptorHeapTypes heap)
			{
				DescriptorHeapHandle_DX12 handle = m_context->GetDescriptorHeap(heap).GetNewHandle();

				if (heap == DescriptorHeapTypes::CBV_SRV_UAV)
				{
					PixelFormat format = GetFormat();
					if (format == PixelFormat::D32_Float)
					{
						format = PixelFormat::R32_Float;
					}

					D3D12_SHADER_RESOURCE_VIEW_DESC shaderResouceViewDesc = {};
					shaderResouceViewDesc.Format = PixelFormatToDX12(format);
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
				}
				else if (heap == DescriptorHeapTypes::RenderTargetView)
				{
					m_context->GetDevice()->CreateRenderTargetView(m_resource.Get(), nullptr, handle.CPUPtr);
				}
				else if (heap == DescriptorHeapTypes::DepthStencilView)
				{
					D3D12_DEPTH_STENCIL_VIEW_DESC desc = {};
					desc.Format = PixelFormatToDX12(GetFormat());
					desc.ViewDimension = TextureTypeToDX12DSVDimension(GetType());
					desc.Flags = D3D12_DSV_FLAG_NONE;

					if (GetType() == TextureType::Tex2D)
					{
						desc.Texture2D.MipSlice = mip_index;
					}
					else if (GetType() == TextureType::Tex2DArray)
					{
						desc.Texture2DArray.MipSlice = mip_index;
						desc.Texture2DArray.ArraySize = layer_count;
						desc.Texture2DArray.FirstArraySlice = layer_index;
					}
					m_context->GetDevice()->CreateDepthStencilView(m_resource.Get(), &desc, handle.CPUPtr);
				}
				else
				{
					FAIL_ASSERT();
				}
				return handle;
			}
		}
	}
}

#endif /// if defined(IS_DX12_ENABLED)