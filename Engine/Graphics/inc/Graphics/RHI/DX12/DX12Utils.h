#pragma once

#if defined(IS_DX12_ENABLED)

#include "Graphics/Enums.h"
#include "Graphics/PixelFormat.h"

#include <d3dx12.h>
#include <stdexcept>

#include <array>

extern DXGI_FORMAT PixelFormatToDXFormat[static_cast<int>(PixelFormat::MAX)];

inline std::string HrToString(HRESULT hr)
{
	char s_str[64] = {};
	sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
	return std::string(s_str);
}

class HrException : public std::runtime_error
{
public:
	HrException(HRESULT hr) : std::runtime_error(HrToString(hr)), m_hr(hr) {}
	HRESULT Error() const { return m_hr; }
private:
	const HRESULT m_hr;
};

inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		throw HrException(hr);
	}
}

namespace Insight
{
	namespace Graphics
	{
		DXGI_FORMAT PixelFormatToDX12(PixelFormat format);
		D3D12_COMMAND_LIST_TYPE GPUCommandListTypeToCommandListTypeDX12(GPUCommandListType type);

		D3D12_FILTER FilterToDX12(Filter filter);
		D3D12_TEXTURE_ADDRESS_MODE SamplerAddressModelToDX12(SamplerAddressMode addressMode);
		D3D12_COMPARISON_FUNC CompareOpToDX12(CompareOp op);
		std::array<FLOAT, 4> BorderColourToDX12(BorderColour boarderColour);

		D3D12_PRIMITIVE_TOPOLOGY_TYPE PrimitiveTopologyTypeToDX12(PrimitiveTopologyType type);
		D3D12_PRIMITIVE_TOPOLOGY PrimitiveTopologyToDX12(PrimitiveTopologyType type);
		D3D12_FILL_MODE PolygonModeToDX12(PolygonMode polygonMode);
		D3D12_CULL_MODE CullModeToDX12(CullMode cullMode);
		D3D12_BLEND BlendFactorToDX12(BlendFactor factor);
		D3D12_BLEND_OP BlendOpToFX12(BlendOp op);
		D3D12_DESCRIPTOR_HEAP_TYPE DescriptorTypeToDX12(DescriptorType type);
		D3D12_DESCRIPTOR_RANGE_TYPE DescriptorRangeTypeToDX12(DescriptorType type);
	}
}

#endif /// if defined(IS_DX12_ENABLED)