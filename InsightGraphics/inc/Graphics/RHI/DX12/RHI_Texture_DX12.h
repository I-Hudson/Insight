#pragma once

#include "Graphics/RHI/RHI_Texture.h"
#include "Graphics/RHI/DX12/RenderContext_DX12.h"
#include "Graphics/RHI/DX12/RHI_PhysicalDevice_DX12.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::DX12
		{
			class RHI_Texture_DX12 : public RHI_Texture
			{
			public:

				ID3D12Resource* GetResouce() const { return m_resource.Get(); }

				// RHI_Texture
				virtual void Create(RenderContext* context, TextureType textureType, int width, int height, int channels) override;
				virtual void Upload(void* data, int sizeInBytes) override;
				virtual std::vector<Byte> Download(void* data, int sizeInBytes) override;

				// RHI_Resouce
				virtual void Release() override;
				virtual bool ValidResouce() override;
				virtual void SetName(std::wstring name) override;

			private:
				ComPtr<ID3D12Resource> m_resource;
				RenderContext_DX12* m_context = nullptr;
			};
		}
	}
}