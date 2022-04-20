#pragma once

#include "Graphics/RHI/RHI_Descriptor.h"
#include "Graphics/RHI/DX12/RHI_PhysicalDevice_DX12.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::DX12
		{
			class RenderContext_DX12;

			class RHI_DescriptorLayout_DX12 : public RHI_DescriptorLayout
			{
			public:
				ID3D12RootSignature* GetRootSignature() { return m_layout.Get(); }

				// RHI_DescriptorLayout
				virtual void Release() override;

			protected:
				virtual void Create(RenderContext* context, int set, std::vector<Descriptor> descriptors) override;

			private:
				RenderContext_DX12* m_context{ nullptr };
				ComPtr<ID3D12RootSignature> m_layout{ nullptr };
			};

			class RHI_Descriptor_DX12 : public RHI_Descriptor
			{
			public:

				// RHI_DescriptorLayout
				virtual void Release() override;

			private:
				RenderContext_DX12* m_context{ nullptr };
			};
		}
	}
}