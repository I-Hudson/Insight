#if defined(IS_DX12_ENABLED)

#include "Graphics/RHI/DX12/RHI_Texture_DX12.h"
#include "Graphics/RHI/DX12/RHI_Buffer_DX12.h"
#include "Graphics/RHI/DX12/DX12Utils.h"
#include "Graphics/PixelFormatExtensions.h"

#include "Core/Profiler.h"
#include "Platforms/Platform.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::DX12
		{
			RHI_Texture_DX12::~RHI_Texture_DX12()
			{
				Release();
			}

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

			DescriptorHeapHandle_DX12 RHI_Texture_DX12::GetUAVHandle() const
			{
				return m_allLayerUAVHandle;
			}

			DescriptorHeapHandle_DX12 RHI_Texture_DX12::GetSingleLayerDescriptorHandle(u32 const index) const
			{
				if (index >= 0 && index < static_cast<u32>(m_singleLayerDescriptorHandle.size()))
				{
					return m_singleLayerDescriptorHandle[index];
				}
				return DescriptorHeapHandle_DX12();
			}

			DescriptorHeapHandle_DX12 RHI_Texture_DX12::GetSingleLayerRenderTargetHandle(u32 const index) const
			{
				if (index >= 0 && index < static_cast<u32>(m_singleLayerRenderTargetHandle.size()))
				{
					return m_singleLayerRenderTargetHandle[index];
				}
				return DescriptorHeapHandle_DX12();
			}

			DescriptorHeapHandle_DX12 RHI_Texture_DX12::GetSingleLayerDepthStencilHandle(u32 const index) const
			{
				if (index >= 0 && index < static_cast<u32>(m_singleLayerDepthStencilHandle.size()))
				{
					return m_singleLayerDepthStencilHandle[index];
				}
				return DescriptorHeapHandle_DX12();
			}

			DescriptorHeapHandle_DX12 RHI_Texture_DX12::GetSingleLayerUAVHandle(u32 const index) const
			{
				if (index >= 0 && index < static_cast<u32>(m_singleLayerUAVHandle.size()))
				{
					return m_singleLayerUAVHandle[index];
				}
				return DescriptorHeapHandle_DX12();
			}

			void RHI_Texture_DX12::Create(RenderContext* context, RHI_TextureInfo createInfo)
			{
				IS_PROFILE_FUNCTION();

				m_context = static_cast<RenderContext_DX12*>(context);

				D3D12_CLEAR_VALUE clearColour = {};
				clearColour.Format = PixelFormatToDX12(createInfo.Format);

				if (createInfo.ImageUsage & ImageUsageFlagsBits::DepthStencilAttachment)
				{
					clearColour.DepthStencil.Depth = RenderContext::Instance().IsRenderOptionsEnabled(RenderOptions::ReverseZ) ? 0.0f : 1.0f;
					clearColour.DepthStencil.Stencil = 0;

					if (!m_context->HasExtension(DeviceExtension::FormatTypeCasting))
					{
						createInfo.Format = PixelFormatExtensions::MakeTypeless(createInfo.Format);
					}
				}
				if (createInfo.ImageUsage & ImageUsageFlagsBits::ColourAttachment)
				{
					const Maths::Vector4 textureClearColour = GetClearColour();
					ASSERT(sizeof(clearColour.Color) == sizeof(textureClearColour));
					Platform::MemCopy(clearColour.Color, &textureClearColour[0], sizeof(clearColour.Color));
				}

				CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(
					PixelFormatToDX12(createInfo.Format),
					createInfo.Width,
					createInfo.Height,
					createInfo.Layer_Count,
					createInfo.Mip_Count,
					1,
					0,
					ImageUsageFlagsToDX12(createInfo.ImageUsage),
					D3D12_TEXTURE_LAYOUT_UNKNOWN,
					0);

				bool optimiseClearColourEnabled = createInfo.ImageUsage & ImageUsageFlagsBits::DepthStencilAttachment
					|| createInfo.ImageUsage & ImageUsageFlagsBits::ColourAttachment;

				D3D12MA::ALLOCATION_DESC allocationDesc = {};
				allocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

				/// Create the texture.
				ThrowIfFailed(m_context->GetAllocator()->CreateResource(
					&allocationDesc,
					&resourceDesc,
					ImageLayoutToDX12ResouceState(createInfo.Layout),
					optimiseClearColourEnabled ? &clearColour : nullptr,
					&m_allocation,
					IID_NULL, NULL));
				SetName(m_name);
				ASSERT(m_allocation);

				for (size_t i = 0; i < createInfo.Mip_Count; ++i)
				{
					// Clamp textures to 1x1.
					createInfo.Width = std::max(createInfo.Width, 1);
					createInfo.Height = std::max(createInfo.Height, 1);
					m_infos.push_back(createInfo);
				}

				if (createInfo.ImageUsage & ImageUsageFlagsBits::Sampled)
				{
					m_allLayerDescriptorHandle = CreateSharderResouceView(0, 1, createInfo.Layer_Count, 0, DescriptorHeapTypes::CBV_SRV_UAV, ImageUsageFlagsBits::Sampled);
				}
				if (createInfo.ImageUsage & ImageUsageFlagsBits::DepthStencilAttachment)
				{
					m_allLayerDepthStencilHandle = CreateSharderResouceView(0, 1, createInfo.Layer_Count, 0, DescriptorHeapTypes::DepthStencilView, ImageUsageFlagsBits::DepthStencilAttachment);
				}
				if (createInfo.ImageUsage & ImageUsageFlagsBits::ColourAttachment)
				{
					m_allLayerRenderTargetHandle= CreateSharderResouceView(0, 1, createInfo.Layer_Count, 0, DescriptorHeapTypes::RenderTargetView, ImageUsageFlagsBits::ColourAttachment);
				}
				if (createInfo.ImageUsage & ImageUsageFlagsBits::Storage)
				{
					m_allLayerUAVHandle = CreateSharderResouceView(0, 1, createInfo.Layer_Count, 0, DescriptorHeapTypes::CBV_SRV_UAV, ImageUsageFlagsBits::Storage);
				}


				// Create a image view for each layer. (Use image views when rendering to different layers).
				for (u32 i = 0; i < createInfo.Layer_Count; ++i)
				{
					if (createInfo.ImageUsage & ImageUsageFlagsBits::Sampled)
					{
						m_singleLayerDescriptorHandle.push_back(CreateSharderResouceView(0, 1, 1, i, DescriptorHeapTypes::CBV_SRV_UAV, ImageUsageFlagsBits::Sampled));
					}
					if (createInfo.ImageUsage & ImageUsageFlagsBits::DepthStencilAttachment)
					{
						m_singleLayerDepthStencilHandle.push_back(CreateSharderResouceView(0, 1, 1, i, DescriptorHeapTypes::DepthStencilView, ImageUsageFlagsBits::DepthStencilAttachment));
					}
					if (createInfo.ImageUsage & ImageUsageFlagsBits::ColourAttachment)
					{
						m_singleLayerRenderTargetHandle.push_back(CreateSharderResouceView(0, 1, 1, i, DescriptorHeapTypes::RenderTargetView, ImageUsageFlagsBits::ColourAttachment));
					}
					if (createInfo.ImageUsage & ImageUsageFlagsBits::Storage)
					{
						m_singleLayerUAVHandle.push_back(CreateSharderResouceView(0, 1, 1, i, DescriptorHeapTypes::CBV_SRV_UAV, ImageUsageFlagsBits::Storage));
					}
				}
			}

			void RHI_Texture_DX12::Upload(void* data, int sizeInBytes)
			{
				IS_PROFILE_FUNCTION();

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
				IS_PROFILE_FUNCTION();

				return std::vector<Byte>();
			}

			void RHI_Texture_DX12::Release()
			{
				IS_PROFILE_FUNCTION();

				RHI_Texture::Release();

				if (m_allocation)
				{
					m_allocation->Release();
					m_allocation = nullptr;
				}

				m_infos.clear();
				if (m_allLayerDescriptorHandle.IsValid())
				{
					m_context->GetDescriptorHeap(m_allLayerDescriptorHandle.HeapType).FreeHandle(m_allLayerDescriptorHandle);
				}
				if (m_allLayerRenderTargetHandle.IsValid())
				{
					m_context->GetDescriptorHeap(m_allLayerRenderTargetHandle.HeapType).FreeHandle(m_allLayerRenderTargetHandle);
				}
				if (m_allLayerDepthStencilHandle.IsValid())
				{
					m_context->GetDescriptorHeap(m_allLayerDepthStencilHandle.HeapType).FreeHandle(m_allLayerDepthStencilHandle);
				}
				if (m_allLayerUAVHandle.IsValid())
				{
					m_context->GetDescriptorHeap(m_allLayerUAVHandle.HeapType).FreeHandle(m_allLayerUAVHandle);
				}

				for (size_t i = 0; i < m_singleLayerDescriptorHandle.size(); ++i)
				{
					m_context->GetDescriptorHeap(m_singleLayerDescriptorHandle[i].HeapType).FreeHandle(m_singleLayerDescriptorHandle[i]);
				}
				for (size_t i = 0; i < m_singleLayerRenderTargetHandle.size(); ++i)
				{
					m_context->GetDescriptorHeap(m_singleLayerRenderTargetHandle[i].HeapType).FreeHandle(m_singleLayerRenderTargetHandle[i]);
				}
				for (size_t i = 0; i < m_singleLayerDepthStencilHandle.size(); ++i)
				{
					m_context->GetDescriptorHeap(m_singleLayerDepthStencilHandle[i].HeapType).FreeHandle(m_singleLayerDepthStencilHandle[i]);
				}
				for (size_t i = 0; i < m_singleLayerUAVHandle.size(); ++i)
				{
					m_context->GetDescriptorHeap(m_singleLayerUAVHandle[i].HeapType).FreeHandle(m_singleLayerUAVHandle[i]);
				}
				m_singleLayerDescriptorHandle.clear();
				m_singleLayerRenderTargetHandle.clear();
				m_singleLayerDepthStencilHandle.clear();
				m_singleLayerUAVHandle.clear();
			}

			bool RHI_Texture_DX12::ValidResource()
			{
				return m_allocation && m_allocation->GetResource();
			}

			void RHI_Texture_DX12::SetName(std::string name)
			{
				if (m_allocation && m_allocation->GetResource())
				{
					m_context->SetObjectName(name, m_allocation->GetResource());
					std::wstring wStr = Platform::WStringFromString(name);
					m_allocation->SetName(wStr.c_str());
				}
				m_name = std::move(name);
			}

			DescriptorHeapHandle_DX12 RHI_Texture_DX12::CreateSharderResouceView(u32 mip_index, u32 mip_count, u32 layer_count, u32 layer_index, DescriptorHeapTypes heap, ImageUsageFlagsBits imageUsage)
			{
				DescriptorHeapHandle_DX12 handle = m_context->GetDescriptorHeap(heap).GetNewHandle();

				if (heap == DescriptorHeapTypes::CBV_SRV_UAV)
				{
					PixelFormat viewFormat = PixelFormatExtensions::FindShaderResourceFormat(GetFormat(), false);

					if (imageUsage & ImageUsageFlagsBits::Sampled)
					{
						D3D12_SHADER_RESOURCE_VIEW_DESC shaderResouceViewDesc = {};
						shaderResouceViewDesc.Format = PixelFormatToDX12(viewFormat);
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
						else if (GetType() == TextureType::TexCube)
						{
							if (layer_count == 6)
							{
								shaderResouceViewDesc.TextureCube.MipLevels = mip_count;
								shaderResouceViewDesc.TextureCube.MostDetailedMip = 0;
							}
							else
							{
								shaderResouceViewDesc.Texture2DArray.ArraySize = layer_count;
								shaderResouceViewDesc.Texture2DArray.FirstArraySlice = layer_index;
								shaderResouceViewDesc.Texture2DArray.MipLevels = mip_count;
								shaderResouceViewDesc.Texture2DArray.MostDetailedMip = 0u;
								shaderResouceViewDesc.Texture2DArray.PlaneSlice = 0u;
								shaderResouceViewDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
							}
						}
						m_context->GetDevice()->CreateShaderResourceView(m_allocation->GetResource(), &shaderResouceViewDesc, handle.CPUPtr);
					}
					else if (imageUsage & ImageUsageFlagsBits::Storage)
					{
						D3D12_UNORDERED_ACCESS_VIEW_DESC shaderUAVViewDesc = {};
						shaderUAVViewDesc.Format = PixelFormatToDX12(viewFormat);
						shaderUAVViewDesc.ViewDimension = TextureTypeToDX12UAVDimension(GetType());

						if (GetType() == TextureType::Tex2D)
						{
							shaderUAVViewDesc.Texture2D.MipSlice = 0;
						}
						else if (GetType() == TextureType::Tex2DArray)
						{
							shaderUAVViewDesc.Texture2DArray.MipSlice = 0;
							shaderUAVViewDesc.Texture2DArray.FirstArraySlice = layer_index;
							shaderUAVViewDesc.Texture2DArray.ArraySize = layer_count;
						}
						m_context->GetDevice()->CreateUnorderedAccessView(m_allocation->GetResource(), nullptr, &shaderUAVViewDesc, handle.CPUPtr);
					}

				}
				else if (heap == DescriptorHeapTypes::RenderTargetView)
				{
					m_context->GetDevice()->CreateRenderTargetView(m_allocation->GetResource(), nullptr, handle.CPUPtr);
				}
				else if (heap == DescriptorHeapTypes::DepthStencilView)
				{
					PixelFormat viewFormat = PixelFormatExtensions::FindDepthStencilFormat(GetFormat());

					D3D12_DEPTH_STENCIL_VIEW_DESC desc = {};
					desc.Format = PixelFormatToDX12(viewFormat);
					desc.ViewDimension = TextureTypeToDX12DSVDimension(GetType());
					desc.Flags = D3D12_DSV_FLAG_NONE;

					if (GetType() == TextureType::Tex2D)
					{
						desc.Texture2D.MipSlice = mip_index;
					}
					else if (GetType() == TextureType::Tex2DArray || GetType() == TextureType::TexCube)
					{
						desc.Texture2DArray.MipSlice = mip_index;
						desc.Texture2DArray.ArraySize = layer_count;
						desc.Texture2DArray.FirstArraySlice = layer_index;
					}
					m_context->GetDevice()->CreateDepthStencilView(m_allocation->GetResource(), &desc, handle.CPUPtr);
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