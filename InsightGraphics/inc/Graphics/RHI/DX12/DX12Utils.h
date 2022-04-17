#pragma once

#include "Graphics/Enums.h"
#include "Graphics/PixelFormat.h"
#include <d3d12.h>
#include <stdexcept>

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

		D3D_PRIMITIVE_TOPOLOGY PrimitiveTopologyTypeToDX12(PrimitiveTopologyType type);
		D3D12_FILL_MODE PolygonModeToDX12(PolygonMode polygonMode);
		D3D12_CULL_MODE CullModeToDX12(CullMode cullMode);
		D3D12_BLEND BlendFactorToDX12(BlendFactor factor);
		D3D12_BLEND_OP BlendOpToFX12(BlendOp op);

        struct CD3DX12_RESOURCE_BARRIER : public D3D12_RESOURCE_BARRIER
        {
            CD3DX12_RESOURCE_BARRIER() = default;
            explicit CD3DX12_RESOURCE_BARRIER(const D3D12_RESOURCE_BARRIER& o) noexcept :
                D3D12_RESOURCE_BARRIER(o)
            {}
            static inline CD3DX12_RESOURCE_BARRIER Transition(
                _In_ ID3D12Resource* pResource,
                D3D12_RESOURCE_STATES stateBefore,
                D3D12_RESOURCE_STATES stateAfter,
                UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
                D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE) noexcept
            {
                CD3DX12_RESOURCE_BARRIER result = {};
                D3D12_RESOURCE_BARRIER& barrier = result;
                result.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                result.Flags = flags;
                barrier.Transition.pResource = pResource;
                barrier.Transition.StateBefore = stateBefore;
                barrier.Transition.StateAfter = stateAfter;
                barrier.Transition.Subresource = subresource;
                return result;
            }
            static inline CD3DX12_RESOURCE_BARRIER Aliasing(
                _In_ ID3D12Resource* pResourceBefore,
                _In_ ID3D12Resource* pResourceAfter) noexcept
            {
                CD3DX12_RESOURCE_BARRIER result = {};
                D3D12_RESOURCE_BARRIER& barrier = result;
                result.Type = D3D12_RESOURCE_BARRIER_TYPE_ALIASING;
                barrier.Aliasing.pResourceBefore = pResourceBefore;
                barrier.Aliasing.pResourceAfter = pResourceAfter;
                return result;
            }
            static inline CD3DX12_RESOURCE_BARRIER UAV(
                _In_ ID3D12Resource* pResource) noexcept
            {
                CD3DX12_RESOURCE_BARRIER result = {};
                D3D12_RESOURCE_BARRIER& barrier = result;
                result.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
                barrier.UAV.pResource = pResource;
                return result;
            }
        };

        struct CD3DX12_CPU_DESCRIPTOR_HANDLE : public D3D12_CPU_DESCRIPTOR_HANDLE
        {
            CD3DX12_CPU_DESCRIPTOR_HANDLE() noexcept { ptr = 0; }
            explicit CD3DX12_CPU_DESCRIPTOR_HANDLE(const D3D12_CPU_DESCRIPTOR_HANDLE& o) noexcept :
                D3D12_CPU_DESCRIPTOR_HANDLE(o)
            {}
            CD3DX12_CPU_DESCRIPTOR_HANDLE(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& other, INT offsetScaledByIncrementSize) noexcept
            {
                InitOffsetted(other, offsetScaledByIncrementSize);
            }
            CD3DX12_CPU_DESCRIPTOR_HANDLE(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& other, INT offsetInDescriptors, UINT descriptorIncrementSize) noexcept
            {
                InitOffsetted(other, offsetInDescriptors, descriptorIncrementSize);
            }
            CD3DX12_CPU_DESCRIPTOR_HANDLE& Offset(INT offsetInDescriptors, UINT descriptorIncrementSize) noexcept
            {
                ptr = SIZE_T(INT64(ptr) + INT64(offsetInDescriptors) * INT64(descriptorIncrementSize));
                return *this;
            }
            CD3DX12_CPU_DESCRIPTOR_HANDLE& Offset(INT offsetScaledByIncrementSize) noexcept
            {
                ptr = SIZE_T(INT64(ptr) + INT64(offsetScaledByIncrementSize));
                return *this;
            }
            bool operator==(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& other) const noexcept
            {
                return (ptr == other.ptr);
            }
            bool operator!=(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& other) const noexcept
            {
                return (ptr != other.ptr);
            }
            CD3DX12_CPU_DESCRIPTOR_HANDLE& operator=(const D3D12_CPU_DESCRIPTOR_HANDLE& other) noexcept
            {
                ptr = other.ptr;
                return *this;
            }

            inline void InitOffsetted(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& base, INT offsetScaledByIncrementSize) noexcept
            {
                InitOffsetted(*this, base, offsetScaledByIncrementSize);
            }

            inline void InitOffsetted(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& base, INT offsetInDescriptors, UINT descriptorIncrementSize) noexcept
            {
                InitOffsetted(*this, base, offsetInDescriptors, descriptorIncrementSize);
            }

            static inline void InitOffsetted(_Out_ D3D12_CPU_DESCRIPTOR_HANDLE& handle, _In_ const D3D12_CPU_DESCRIPTOR_HANDLE& base, INT offsetScaledByIncrementSize) noexcept
            {
                handle.ptr = SIZE_T(INT64(base.ptr) + INT64(offsetScaledByIncrementSize));
            }

            static inline void InitOffsetted(_Out_ D3D12_CPU_DESCRIPTOR_HANDLE& handle, _In_ const D3D12_CPU_DESCRIPTOR_HANDLE& base, INT offsetInDescriptors, UINT descriptorIncrementSize) noexcept
            {
                handle.ptr = SIZE_T(INT64(base.ptr) + INT64(offsetInDescriptors) * INT64(descriptorIncrementSize));
            }
        };
	}
}