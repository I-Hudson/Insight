#include "Graphics/RHI/DX12/RHI_Texture_DX12.h"
#include "Graphics/RHI/DX12/DX12Utils.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::DX12
		{
			void RHI_Texture_DX12::Create(RenderContext* context, TextureType textureType, int width, int height, int channels)
			{
				m_context = dynamic_cast<RenderContext_DX12*>(context);
				m_width = width;
				m_height = height;
				m_channels = channels;
				m_type = textureType;

				CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(
					PixelFormatToDX12(m_format),
					m_width,
					m_height,
					1,
					0,
					1,
					0,
					D3D12_RESOURCE_FLAG_NONE,
					D3D12_TEXTURE_LAYOUT_UNKNOWN,
					0);

				// Create the texture.
				ThrowIfFailed(m_context->GetDevice()->CreateCommittedResource(
					&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
					D3D12_HEAP_FLAG_NONE,
					&resourceDesc,
					D3D12_RESOURCE_STATE_COPY_DEST,
					nullptr,
					IID_PPV_ARGS(&m_resource)));
			}

			void RHI_Texture_DX12::Upload(void* data, int sizeInBytes)
			{

			}

			std::vector<Byte> RHI_Texture_DX12::Download(void* data, int sizeInBytes)
			{
				return std::vector<Byte>();
			}

			void RHI_Texture_DX12::Release()
			{
				if (m_resource)
				{
					m_resource->Release();
					m_resource.Reset();
				}
			}

			bool RHI_Texture_DX12::ValidResouce()
			{
				return m_resource;
			}

			void RHI_Texture_DX12::SetName(std::wstring name)
			{
				if (m_resource)
				{
					m_resource->SetName(name.c_str());
				}
			}
		}
	}
}