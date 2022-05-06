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
		D3D12_DESCRIPTOR_HEAP_TYPE DescriptorTypeToDX12(DescriptorType type);

		struct SceneConstantBuffer
		{
			float offset[4];
			float padding[60]; // Padding so the constant buffer is 256-byte aligned.
		};
		static_assert((sizeof(SceneConstantBuffer) % 256) == 0, "Constant Buffer size must be 256-byte aligned");


		//------------------------------------------------------------------------------------------------
		inline constexpr UINT D3D12CalcSubresource(UINT MipSlice, UINT ArraySlice, UINT PlaneSlice, UINT MipLevels, UINT ArraySize) noexcept
		{
			return MipSlice + ArraySlice * MipLevels + PlaneSlice * MipLevels * ArraySize;
		}

		//------------------------------------------------------------------------------------------------
		template <typename T, typename U, typename V>
		inline void D3D12DecomposeSubresource(UINT Subresource, UINT MipLevels, UINT ArraySize, _Out_ T& MipSlice, _Out_ U& ArraySlice, _Out_ V& PlaneSlice) noexcept
		{
			MipSlice = static_cast<T>(Subresource % MipLevels);
			ArraySlice = static_cast<U>((Subresource / MipLevels) % ArraySize);
			PlaneSlice = static_cast<V>(Subresource / (MipLevels * ArraySize));
		}

		//------------------------------------------------------------------------------------------------
		inline UINT8 D3D12GetFormatPlaneCount(
			_In_ ID3D12Device* pDevice,
			DXGI_FORMAT Format
		) noexcept
		{
			D3D12_FEATURE_DATA_FORMAT_INFO formatInfo = { Format, 0 };
			if (FAILED(pDevice->CheckFeatureSupport(D3D12_FEATURE_FORMAT_INFO, &formatInfo, sizeof(formatInfo))))
			{
				return 0;
			}
			return formatInfo.PlaneCount;
		}

		//------------------------------------------------------------------------------------------------
		struct CD3DX12_RANGE : public D3D12_RANGE
		{
			CD3DX12_RANGE() = default;
			explicit CD3DX12_RANGE(const D3D12_RANGE& o) noexcept :
				D3D12_RANGE(o)
			{}
			CD3DX12_RANGE(
				SIZE_T begin,
				SIZE_T end) noexcept
			{
				Begin = begin;
				End = end;
			}
		};

		//------------------------------------------------------------------------------------------------
		struct CD3DX12_RESOURCE_DESC : public D3D12_RESOURCE_DESC
		{
			CD3DX12_RESOURCE_DESC() = default;
			explicit CD3DX12_RESOURCE_DESC(const D3D12_RESOURCE_DESC& o) noexcept :
				D3D12_RESOURCE_DESC(o)
			{}
			CD3DX12_RESOURCE_DESC(
				D3D12_RESOURCE_DIMENSION dimension,
				UINT64 alignment,
				UINT64 width,
				UINT height,
				UINT16 depthOrArraySize,
				UINT16 mipLevels,
				DXGI_FORMAT format,
				UINT sampleCount,
				UINT sampleQuality,
				D3D12_TEXTURE_LAYOUT layout,
				D3D12_RESOURCE_FLAGS flags) noexcept
			{
				Dimension = dimension;
				Alignment = alignment;
				Width = width;
				Height = height;
				DepthOrArraySize = depthOrArraySize;
				MipLevels = mipLevels;
				Format = format;
				SampleDesc.Count = sampleCount;
				SampleDesc.Quality = sampleQuality;
				Layout = layout;
				Flags = flags;
			}
			static inline CD3DX12_RESOURCE_DESC Buffer(
				const D3D12_RESOURCE_ALLOCATION_INFO& resAllocInfo,
				D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE) noexcept
			{
				return CD3DX12_RESOURCE_DESC(D3D12_RESOURCE_DIMENSION_BUFFER, resAllocInfo.Alignment, resAllocInfo.SizeInBytes,
					1, 1, 1, DXGI_FORMAT_UNKNOWN, 1, 0, D3D12_TEXTURE_LAYOUT_ROW_MAJOR, flags);
			}
			static inline CD3DX12_RESOURCE_DESC Buffer(
				UINT64 width,
				D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE,
				UINT64 alignment = 0) noexcept
			{
				return CD3DX12_RESOURCE_DESC(D3D12_RESOURCE_DIMENSION_BUFFER, alignment, width, 1, 1, 1,
					DXGI_FORMAT_UNKNOWN, 1, 0, D3D12_TEXTURE_LAYOUT_ROW_MAJOR, flags);
			}
			static inline CD3DX12_RESOURCE_DESC Tex1D(
				DXGI_FORMAT format,
				UINT64 width,
				UINT16 arraySize = 1,
				UINT16 mipLevels = 0,
				D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE,
				D3D12_TEXTURE_LAYOUT layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
				UINT64 alignment = 0) noexcept
			{
				return CD3DX12_RESOURCE_DESC(D3D12_RESOURCE_DIMENSION_TEXTURE1D, alignment, width, 1, arraySize,
					mipLevels, format, 1, 0, layout, flags);
			}
			static inline CD3DX12_RESOURCE_DESC Tex2D(
				DXGI_FORMAT format,
				UINT64 width,
				UINT height,
				UINT16 arraySize = 1,
				UINT16 mipLevels = 0,
				UINT sampleCount = 1,
				UINT sampleQuality = 0,
				D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE,
				D3D12_TEXTURE_LAYOUT layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
				UINT64 alignment = 0) noexcept
			{
				return CD3DX12_RESOURCE_DESC(D3D12_RESOURCE_DIMENSION_TEXTURE2D, alignment, width, height, arraySize,
					mipLevels, format, sampleCount, sampleQuality, layout, flags);
			}
			static inline CD3DX12_RESOURCE_DESC Tex3D(
				DXGI_FORMAT format,
				UINT64 width,
				UINT height,
				UINT16 depth,
				UINT16 mipLevels = 0,
				D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE,
				D3D12_TEXTURE_LAYOUT layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
				UINT64 alignment = 0) noexcept
			{
				return CD3DX12_RESOURCE_DESC(D3D12_RESOURCE_DIMENSION_TEXTURE3D, alignment, width, height, depth,
					mipLevels, format, 1, 0, layout, flags);
			}
			inline UINT16 Depth() const noexcept
			{
				return (Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D ? DepthOrArraySize : 1);
			}
			inline UINT16 ArraySize() const noexcept
			{
				return (Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE3D ? DepthOrArraySize : 1);
			}
			inline UINT8 PlaneCount(_In_ ID3D12Device* pDevice) const noexcept
			{
				return D3D12GetFormatPlaneCount(pDevice, Format);
			}
			inline UINT Subresources(_In_ ID3D12Device* pDevice) const noexcept
			{
				return MipLevels * ArraySize() * PlaneCount(pDevice);
			}
			inline UINT CalcSubresource(UINT MipSlice, UINT ArraySlice, UINT PlaneSlice) noexcept
			{
				return D3D12CalcSubresource(MipSlice, ArraySlice, PlaneSlice, MipLevels, ArraySize());
			}
		};
		inline bool operator==(const D3D12_RESOURCE_DESC& l, const D3D12_RESOURCE_DESC& r) noexcept
		{
			return l.Dimension == r.Dimension &&
				l.Alignment == r.Alignment &&
				l.Width == r.Width &&
				l.Height == r.Height &&
				l.DepthOrArraySize == r.DepthOrArraySize &&
				l.MipLevels == r.MipLevels &&
				l.Format == r.Format &&
				l.SampleDesc.Count == r.SampleDesc.Count &&
				l.SampleDesc.Quality == r.SampleDesc.Quality &&
				l.Layout == r.Layout &&
				l.Flags == r.Flags;
		}
		inline bool operator!=(const D3D12_RESOURCE_DESC& l, const D3D12_RESOURCE_DESC& r) noexcept
		{
			return !(l == r);
		}

		//------------------------------------------------------------------------------------------------
		struct CD3DX12_RESOURCE_DESC1 : public D3D12_RESOURCE_DESC1
		{
			CD3DX12_RESOURCE_DESC1() = default;
			explicit CD3DX12_RESOURCE_DESC1(const D3D12_RESOURCE_DESC1& o) noexcept :
				D3D12_RESOURCE_DESC1(o)
			{}
			CD3DX12_RESOURCE_DESC1(
				D3D12_RESOURCE_DIMENSION dimension,
				UINT64 alignment,
				UINT64 width,
				UINT height,
				UINT16 depthOrArraySize,
				UINT16 mipLevels,
				DXGI_FORMAT format,
				UINT sampleCount,
				UINT sampleQuality,
				D3D12_TEXTURE_LAYOUT layout,
				D3D12_RESOURCE_FLAGS flags,
				UINT samplerFeedbackMipRegionWidth = 0,
				UINT samplerFeedbackMipRegionHeight = 0,
				UINT samplerFeedbackMipRegionDepth = 0) noexcept
			{
				Dimension = dimension;
				Alignment = alignment;
				Width = width;
				Height = height;
				DepthOrArraySize = depthOrArraySize;
				MipLevels = mipLevels;
				Format = format;
				SampleDesc.Count = sampleCount;
				SampleDesc.Quality = sampleQuality;
				Layout = layout;
				Flags = flags;
				SamplerFeedbackMipRegion.Width = samplerFeedbackMipRegionWidth;
				SamplerFeedbackMipRegion.Height = samplerFeedbackMipRegionHeight;
				SamplerFeedbackMipRegion.Depth = samplerFeedbackMipRegionDepth;
			}
			static inline CD3DX12_RESOURCE_DESC1 Buffer(
				const D3D12_RESOURCE_ALLOCATION_INFO& resAllocInfo,
				D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE) noexcept
			{
				return CD3DX12_RESOURCE_DESC1(D3D12_RESOURCE_DIMENSION_BUFFER, resAllocInfo.Alignment, resAllocInfo.SizeInBytes,
					1, 1, 1, DXGI_FORMAT_UNKNOWN, 1, 0, D3D12_TEXTURE_LAYOUT_ROW_MAJOR, flags, 0, 0, 0);
			}
			static inline CD3DX12_RESOURCE_DESC1 Buffer(
				UINT64 width,
				D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE,
				UINT64 alignment = 0) noexcept
			{
				return CD3DX12_RESOURCE_DESC1(D3D12_RESOURCE_DIMENSION_BUFFER, alignment, width, 1, 1, 1,
					DXGI_FORMAT_UNKNOWN, 1, 0, D3D12_TEXTURE_LAYOUT_ROW_MAJOR, flags, 0, 0, 0);
			}
			static inline CD3DX12_RESOURCE_DESC1 Tex1D(
				DXGI_FORMAT format,
				UINT64 width,
				UINT16 arraySize = 1,
				UINT16 mipLevels = 0,
				D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE,
				D3D12_TEXTURE_LAYOUT layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
				UINT64 alignment = 0) noexcept
			{
				return CD3DX12_RESOURCE_DESC1(D3D12_RESOURCE_DIMENSION_TEXTURE1D, alignment, width, 1, arraySize,
					mipLevels, format, 1, 0, layout, flags, 0, 0, 0);
			}
			static inline CD3DX12_RESOURCE_DESC1 Tex2D(
				DXGI_FORMAT format,
				UINT64 width,
				UINT height,
				UINT16 arraySize = 1,
				UINT16 mipLevels = 0,
				UINT sampleCount = 1,
				UINT sampleQuality = 0,
				D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE,
				D3D12_TEXTURE_LAYOUT layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
				UINT64 alignment = 0,
				UINT samplerFeedbackMipRegionWidth = 0,
				UINT samplerFeedbackMipRegionHeight = 0,
				UINT samplerFeedbackMipRegionDepth = 0) noexcept
			{
				return CD3DX12_RESOURCE_DESC1(D3D12_RESOURCE_DIMENSION_TEXTURE2D, alignment, width, height, arraySize,
					mipLevels, format, sampleCount, sampleQuality, layout, flags, samplerFeedbackMipRegionWidth,
					samplerFeedbackMipRegionHeight, samplerFeedbackMipRegionDepth);
			}
			static inline CD3DX12_RESOURCE_DESC1 Tex3D(
				DXGI_FORMAT format,
				UINT64 width,
				UINT height,
				UINT16 depth,
				UINT16 mipLevels = 0,
				D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE,
				D3D12_TEXTURE_LAYOUT layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
				UINT64 alignment = 0) noexcept
			{
				return CD3DX12_RESOURCE_DESC1(D3D12_RESOURCE_DIMENSION_TEXTURE3D, alignment, width, height, depth,
					mipLevels, format, 1, 0, layout, flags, 0, 0, 0);
			}
			inline UINT16 Depth() const noexcept
			{
				return (Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D ? DepthOrArraySize : 1);
			}
			inline UINT16 ArraySize() const noexcept
			{
				return (Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE3D ? DepthOrArraySize : 1);
			}
			inline UINT8 PlaneCount(_In_ ID3D12Device* pDevice) const noexcept
			{
				return D3D12GetFormatPlaneCount(pDevice, Format);
			}
			inline UINT Subresources(_In_ ID3D12Device* pDevice) const noexcept
			{
				return MipLevels * ArraySize() * PlaneCount(pDevice);
			}
			inline UINT CalcSubresource(UINT MipSlice, UINT ArraySlice, UINT PlaneSlice) noexcept
			{
				return D3D12CalcSubresource(MipSlice, ArraySlice, PlaneSlice, MipLevels, ArraySize());
			}
		};
		inline bool operator==(const D3D12_RESOURCE_DESC1& l, const D3D12_RESOURCE_DESC1& r) noexcept
		{
			return l.Dimension == r.Dimension &&
				l.Alignment == r.Alignment &&
				l.Width == r.Width &&
				l.Height == r.Height &&
				l.DepthOrArraySize == r.DepthOrArraySize &&
				l.MipLevels == r.MipLevels &&
				l.Format == r.Format &&
				l.SampleDesc.Count == r.SampleDesc.Count &&
				l.SampleDesc.Quality == r.SampleDesc.Quality &&
				l.Layout == r.Layout &&
				l.Flags == r.Flags &&
				l.SamplerFeedbackMipRegion.Width == r.SamplerFeedbackMipRegion.Width &&
				l.SamplerFeedbackMipRegion.Height == r.SamplerFeedbackMipRegion.Height &&
				l.SamplerFeedbackMipRegion.Depth == r.SamplerFeedbackMipRegion.Depth;
		}
		inline bool operator!=(const D3D12_RESOURCE_DESC1& l, const D3D12_RESOURCE_DESC1& r) noexcept
		{
			return !(l == r);
		}


		//------------------------------------------------------------------------------------------------
		struct CD3DX12_HEAP_PROPERTIES : public D3D12_HEAP_PROPERTIES
		{
			CD3DX12_HEAP_PROPERTIES() = default;
			explicit CD3DX12_HEAP_PROPERTIES(const D3D12_HEAP_PROPERTIES& o) noexcept :
				D3D12_HEAP_PROPERTIES(o)
			{}
			CD3DX12_HEAP_PROPERTIES(
				D3D12_CPU_PAGE_PROPERTY cpuPageProperty,
				D3D12_MEMORY_POOL memoryPoolPreference,
				UINT creationNodeMask = 1,
				UINT nodeMask = 1) noexcept
			{
				Type = D3D12_HEAP_TYPE_CUSTOM;
				CPUPageProperty = cpuPageProperty;
				MemoryPoolPreference = memoryPoolPreference;
				CreationNodeMask = creationNodeMask;
				VisibleNodeMask = nodeMask;
			}
			explicit CD3DX12_HEAP_PROPERTIES(
				D3D12_HEAP_TYPE type,
				UINT creationNodeMask = 1,
				UINT nodeMask = 1) noexcept
			{
				Type = type;
				CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
				MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
				CreationNodeMask = creationNodeMask;
				VisibleNodeMask = nodeMask;
			}
			bool IsCPUAccessible() const noexcept
			{
				return Type == D3D12_HEAP_TYPE_UPLOAD || Type == D3D12_HEAP_TYPE_READBACK || (Type == D3D12_HEAP_TYPE_CUSTOM &&
					(CPUPageProperty == D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE || CPUPageProperty == D3D12_CPU_PAGE_PROPERTY_WRITE_BACK));
			}
		};
		inline bool operator==(const D3D12_HEAP_PROPERTIES& l, const D3D12_HEAP_PROPERTIES& r) noexcept
		{
			return l.Type == r.Type && l.CPUPageProperty == r.CPUPageProperty &&
				l.MemoryPoolPreference == r.MemoryPoolPreference &&
				l.CreationNodeMask == r.CreationNodeMask &&
				l.VisibleNodeMask == r.VisibleNodeMask;
		}
		inline bool operator!=(const D3D12_HEAP_PROPERTIES& l, const D3D12_HEAP_PROPERTIES& r) noexcept
		{
			return !(l == r);
		}


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

		struct CD3DX12_DEFAULT {};

		//------------------------------------------------------------------------------------------------
		struct CD3DX12_ROOT_DESCRIPTOR_TABLE1 : public D3D12_ROOT_DESCRIPTOR_TABLE1
		{
			CD3DX12_ROOT_DESCRIPTOR_TABLE1() = default;
			explicit CD3DX12_ROOT_DESCRIPTOR_TABLE1(const D3D12_ROOT_DESCRIPTOR_TABLE1& o) noexcept :
				D3D12_ROOT_DESCRIPTOR_TABLE1(o)
			{}
			CD3DX12_ROOT_DESCRIPTOR_TABLE1(
				UINT numDescriptorRanges,
				_In_reads_opt_(numDescriptorRanges) const D3D12_DESCRIPTOR_RANGE1* _pDescriptorRanges) noexcept
			{
				Init(numDescriptorRanges, _pDescriptorRanges);
			}

			inline void Init(
				UINT numDescriptorRanges,
				_In_reads_opt_(numDescriptorRanges) const D3D12_DESCRIPTOR_RANGE1* _pDescriptorRanges) noexcept
			{
				Init(*this, numDescriptorRanges, _pDescriptorRanges);
			}

			static inline void Init(
				_Out_ D3D12_ROOT_DESCRIPTOR_TABLE1& rootDescriptorTable,
				UINT numDescriptorRanges,
				_In_reads_opt_(numDescriptorRanges) const D3D12_DESCRIPTOR_RANGE1* _pDescriptorRanges) noexcept
			{
				rootDescriptorTable.NumDescriptorRanges = numDescriptorRanges;
				rootDescriptorTable.pDescriptorRanges = _pDescriptorRanges;
			}
		};

		//------------------------------------------------------------------------------------------------
		struct CD3DX12_ROOT_CONSTANTS : public D3D12_ROOT_CONSTANTS
		{
			CD3DX12_ROOT_CONSTANTS() = default;
			explicit CD3DX12_ROOT_CONSTANTS(const D3D12_ROOT_CONSTANTS& o) noexcept :
				D3D12_ROOT_CONSTANTS(o)
			{}
			CD3DX12_ROOT_CONSTANTS(
				UINT num32BitValues,
				UINT shaderRegister,
				UINT registerSpace = 0) noexcept
			{
				Init(num32BitValues, shaderRegister, registerSpace);
			}

			inline void Init(
				UINT num32BitValues,
				UINT shaderRegister,
				UINT registerSpace = 0) noexcept
			{
				Init(*this, num32BitValues, shaderRegister, registerSpace);
			}

			static inline void Init(
				_Out_ D3D12_ROOT_CONSTANTS& rootConstants,
				UINT num32BitValues,
				UINT shaderRegister,
				UINT registerSpace = 0) noexcept
			{
				rootConstants.Num32BitValues = num32BitValues;
				rootConstants.ShaderRegister = shaderRegister;
				rootConstants.RegisterSpace = registerSpace;
			}
		};

		//------------------------------------------------------------------------------------------------
		struct CD3DX12_ROOT_DESCRIPTOR1 : public D3D12_ROOT_DESCRIPTOR1
		{
			CD3DX12_ROOT_DESCRIPTOR1() = default;
			explicit CD3DX12_ROOT_DESCRIPTOR1(const D3D12_ROOT_DESCRIPTOR1& o) noexcept :
				D3D12_ROOT_DESCRIPTOR1(o)
			{}
			CD3DX12_ROOT_DESCRIPTOR1(
				UINT shaderRegister,
				UINT registerSpace = 0,
				D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE) noexcept
			{
				Init(shaderRegister, registerSpace, flags);
			}

			inline void Init(
				UINT shaderRegister,
				UINT registerSpace = 0,
				D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE) noexcept
			{
				Init(*this, shaderRegister, registerSpace, flags);
			}

			static inline void Init(
				_Out_ D3D12_ROOT_DESCRIPTOR1& table,
				UINT shaderRegister,
				UINT registerSpace = 0,
				D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE) noexcept
			{
				table.ShaderRegister = shaderRegister;
				table.RegisterSpace = registerSpace;
				table.Flags = flags;
			}
		};

		//------------------------------------------------------------------------------------------------
		struct CD3DX12_DESCRIPTOR_RANGE1 : public D3D12_DESCRIPTOR_RANGE1
		{
			CD3DX12_DESCRIPTOR_RANGE1() = default;
			explicit CD3DX12_DESCRIPTOR_RANGE1(const D3D12_DESCRIPTOR_RANGE1& o) noexcept :
				D3D12_DESCRIPTOR_RANGE1(o)
			{}
			CD3DX12_DESCRIPTOR_RANGE1(
				D3D12_DESCRIPTOR_RANGE_TYPE rangeType,
				UINT numDescriptors,
				UINT baseShaderRegister,
				UINT registerSpace = 0,
				D3D12_DESCRIPTOR_RANGE_FLAGS flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE,
				UINT offsetInDescriptorsFromTableStart =
				D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND) noexcept
			{
				Init(rangeType, numDescriptors, baseShaderRegister, registerSpace, flags, offsetInDescriptorsFromTableStart);
			}

			inline void Init(
				D3D12_DESCRIPTOR_RANGE_TYPE rangeType,
				UINT numDescriptors,
				UINT baseShaderRegister,
				UINT registerSpace = 0,
				D3D12_DESCRIPTOR_RANGE_FLAGS flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE,
				UINT offsetInDescriptorsFromTableStart =
				D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND) noexcept
			{
				Init(*this, rangeType, numDescriptors, baseShaderRegister, registerSpace, flags, offsetInDescriptorsFromTableStart);
			}

			static inline void Init(
				_Out_ D3D12_DESCRIPTOR_RANGE1& range,
				D3D12_DESCRIPTOR_RANGE_TYPE rangeType,
				UINT numDescriptors,
				UINT baseShaderRegister,
				UINT registerSpace = 0,
				D3D12_DESCRIPTOR_RANGE_FLAGS flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE,
				UINT offsetInDescriptorsFromTableStart =
				D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND) noexcept
			{
				range.RangeType = rangeType;
				range.NumDescriptors = numDescriptors;
				range.BaseShaderRegister = baseShaderRegister;
				range.RegisterSpace = registerSpace;
				range.Flags = flags;
				range.OffsetInDescriptorsFromTableStart = offsetInDescriptorsFromTableStart;
			}
		};

		//------------------------------------------------------------------------------------------------
		struct CD3DX12_ROOT_PARAMETER1 : public D3D12_ROOT_PARAMETER1
		{
			CD3DX12_ROOT_PARAMETER1() = default;
			explicit CD3DX12_ROOT_PARAMETER1(const D3D12_ROOT_PARAMETER1& o) noexcept :
				D3D12_ROOT_PARAMETER1(o)
			{}

			static inline void InitAsDescriptorTable(
				_Out_ D3D12_ROOT_PARAMETER1& rootParam,
				UINT numDescriptorRanges,
				_In_reads_(numDescriptorRanges) const D3D12_DESCRIPTOR_RANGE1* pDescriptorRanges,
				D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL) noexcept
			{
				rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				rootParam.ShaderVisibility = visibility;
				CD3DX12_ROOT_DESCRIPTOR_TABLE1::Init(rootParam.DescriptorTable, numDescriptorRanges, pDescriptorRanges);
			}

			static inline void InitAsConstants(
				_Out_ D3D12_ROOT_PARAMETER1& rootParam,
				UINT num32BitValues,
				UINT shaderRegister,
				UINT registerSpace = 0,
				D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL) noexcept
			{
				rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
				rootParam.ShaderVisibility = visibility;
				CD3DX12_ROOT_CONSTANTS::Init(rootParam.Constants, num32BitValues, shaderRegister, registerSpace);
			}

			static inline void InitAsConstantBufferView(
				_Out_ D3D12_ROOT_PARAMETER1& rootParam,
				UINT shaderRegister,
				UINT registerSpace = 0,
				D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
				D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL) noexcept
			{
				rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
				rootParam.ShaderVisibility = visibility;
				CD3DX12_ROOT_DESCRIPTOR1::Init(rootParam.Descriptor, shaderRegister, registerSpace, flags);
			}

			static inline void InitAsShaderResourceView(
				_Out_ D3D12_ROOT_PARAMETER1& rootParam,
				UINT shaderRegister,
				UINT registerSpace = 0,
				D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
				D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL) noexcept
			{
				rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
				rootParam.ShaderVisibility = visibility;
				CD3DX12_ROOT_DESCRIPTOR1::Init(rootParam.Descriptor, shaderRegister, registerSpace, flags);
			}

			static inline void InitAsUnorderedAccessView(
				_Out_ D3D12_ROOT_PARAMETER1& rootParam,
				UINT shaderRegister,
				UINT registerSpace = 0,
				D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
				D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL) noexcept
			{
				rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
				rootParam.ShaderVisibility = visibility;
				CD3DX12_ROOT_DESCRIPTOR1::Init(rootParam.Descriptor, shaderRegister, registerSpace, flags);
			}

			inline void InitAsDescriptorTable(
				UINT numDescriptorRanges,
				_In_reads_(numDescriptorRanges) const D3D12_DESCRIPTOR_RANGE1* pDescriptorRanges,
				D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL) noexcept
			{
				InitAsDescriptorTable(*this, numDescriptorRanges, pDescriptorRanges, visibility);
			}

			inline void InitAsConstants(
				UINT num32BitValues,
				UINT shaderRegister,
				UINT registerSpace = 0,
				D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL) noexcept
			{
				InitAsConstants(*this, num32BitValues, shaderRegister, registerSpace, visibility);
			}

			inline void InitAsConstantBufferView(
				UINT shaderRegister,
				UINT registerSpace = 0,
				D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
				D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL) noexcept
			{
				InitAsConstantBufferView(*this, shaderRegister, registerSpace, flags, visibility);
			}

			inline void InitAsShaderResourceView(
				UINT shaderRegister,
				UINT registerSpace = 0,
				D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
				D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL) noexcept
			{
				InitAsShaderResourceView(*this, shaderRegister, registerSpace, flags, visibility);
			}

			inline void InitAsUnorderedAccessView(
				UINT shaderRegister,
				UINT registerSpace = 0,
				D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
				D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL) noexcept
			{
				InitAsUnorderedAccessView(*this, shaderRegister, registerSpace, flags, visibility);
			}
		};

		//------------------------------------------------------------------------------------------------
		struct CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC : public D3D12_VERSIONED_ROOT_SIGNATURE_DESC
		{
			CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC() = default;
			explicit CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC(const D3D12_VERSIONED_ROOT_SIGNATURE_DESC& o) noexcept :
				D3D12_VERSIONED_ROOT_SIGNATURE_DESC(o)
			{}
			explicit CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC(const D3D12_ROOT_SIGNATURE_DESC& o) noexcept
			{
				Version = D3D_ROOT_SIGNATURE_VERSION_1_0;
				Desc_1_0 = o;
			}
			explicit CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC(const D3D12_ROOT_SIGNATURE_DESC1& o) noexcept
			{
				Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
				Desc_1_1 = o;
			}
			CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC(
				UINT numParameters,
				_In_reads_opt_(numParameters) const D3D12_ROOT_PARAMETER* _pParameters,
				UINT numStaticSamplers = 0,
				_In_reads_opt_(numStaticSamplers) const D3D12_STATIC_SAMPLER_DESC* _pStaticSamplers = nullptr,
				D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE) noexcept
			{
				Init_1_0(numParameters, _pParameters, numStaticSamplers, _pStaticSamplers, flags);
			}
			CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC(
				UINT numParameters,
				_In_reads_opt_(numParameters) const D3D12_ROOT_PARAMETER1* _pParameters,
				UINT numStaticSamplers = 0,
				_In_reads_opt_(numStaticSamplers) const D3D12_STATIC_SAMPLER_DESC* _pStaticSamplers = nullptr,
				D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE) noexcept
			{
				Init_1_1(numParameters, _pParameters, numStaticSamplers, _pStaticSamplers, flags);
			}
			CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC(CD3DX12_DEFAULT) noexcept
			{
				Init_1_1(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_NONE);
			}

			inline void Init_1_0(
				UINT numParameters,
				_In_reads_opt_(numParameters) const D3D12_ROOT_PARAMETER* _pParameters,
				UINT numStaticSamplers = 0,
				_In_reads_opt_(numStaticSamplers) const D3D12_STATIC_SAMPLER_DESC* _pStaticSamplers = nullptr,
				D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE) noexcept
			{
				Init_1_0(*this, numParameters, _pParameters, numStaticSamplers, _pStaticSamplers, flags);
			}

			static inline void Init_1_0(
				_Out_ D3D12_VERSIONED_ROOT_SIGNATURE_DESC& desc,
				UINT numParameters,
				_In_reads_opt_(numParameters) const D3D12_ROOT_PARAMETER* _pParameters,
				UINT numStaticSamplers = 0,
				_In_reads_opt_(numStaticSamplers) const D3D12_STATIC_SAMPLER_DESC* _pStaticSamplers = nullptr,
				D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE) noexcept
			{
				desc.Version = D3D_ROOT_SIGNATURE_VERSION_1_0;
				desc.Desc_1_0.NumParameters = numParameters;
				desc.Desc_1_0.pParameters = _pParameters;
				desc.Desc_1_0.NumStaticSamplers = numStaticSamplers;
				desc.Desc_1_0.pStaticSamplers = _pStaticSamplers;
				desc.Desc_1_0.Flags = flags;
			}

			inline void Init_1_1(
				UINT numParameters,
				_In_reads_opt_(numParameters) const D3D12_ROOT_PARAMETER1* _pParameters,
				UINT numStaticSamplers = 0,
				_In_reads_opt_(numStaticSamplers) const D3D12_STATIC_SAMPLER_DESC* _pStaticSamplers = nullptr,
				D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE) noexcept
			{
				Init_1_1(*this, numParameters, _pParameters, numStaticSamplers, _pStaticSamplers, flags);
			}

			static inline void Init_1_1(
				_Out_ D3D12_VERSIONED_ROOT_SIGNATURE_DESC& desc,
				UINT numParameters,
				_In_reads_opt_(numParameters) const D3D12_ROOT_PARAMETER1* _pParameters,
				UINT numStaticSamplers = 0,
				_In_reads_opt_(numStaticSamplers) const D3D12_STATIC_SAMPLER_DESC* _pStaticSamplers = nullptr,
				D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE) noexcept
			{
				desc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
				desc.Desc_1_1.NumParameters = numParameters;
				desc.Desc_1_1.pParameters = _pParameters;
				desc.Desc_1_1.NumStaticSamplers = numStaticSamplers;
				desc.Desc_1_1.pStaticSamplers = _pStaticSamplers;
				desc.Desc_1_1.Flags = flags;
			}
		};

		//------------------------------------------------------------------------------------------------
		struct CD3DX12_ROOT_SIGNATURE_DESC : public D3D12_ROOT_SIGNATURE_DESC
		{
			CD3DX12_ROOT_SIGNATURE_DESC() = default;
			explicit CD3DX12_ROOT_SIGNATURE_DESC(const D3D12_ROOT_SIGNATURE_DESC& o) noexcept :
				D3D12_ROOT_SIGNATURE_DESC(o)
			{}
			CD3DX12_ROOT_SIGNATURE_DESC(
				UINT numParameters,
				_In_reads_opt_(numParameters) const D3D12_ROOT_PARAMETER* _pParameters,
				UINT numStaticSamplers = 0,
				_In_reads_opt_(numStaticSamplers) const D3D12_STATIC_SAMPLER_DESC* _pStaticSamplers = nullptr,
				D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE) noexcept
			{
				Init(numParameters, _pParameters, numStaticSamplers, _pStaticSamplers, flags);
			}
			CD3DX12_ROOT_SIGNATURE_DESC(CD3DX12_DEFAULT) noexcept
			{
				Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_NONE);
			}

			inline void Init(
				UINT numParameters,
				_In_reads_opt_(numParameters) const D3D12_ROOT_PARAMETER* _pParameters,
				UINT numStaticSamplers = 0,
				_In_reads_opt_(numStaticSamplers) const D3D12_STATIC_SAMPLER_DESC* _pStaticSamplers = nullptr,
				D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE) noexcept
			{
				Init(*this, numParameters, _pParameters, numStaticSamplers, _pStaticSamplers, flags);
			}

			static inline void Init(
				_Out_ D3D12_ROOT_SIGNATURE_DESC& desc,
				UINT numParameters,
				_In_reads_opt_(numParameters) const D3D12_ROOT_PARAMETER* _pParameters,
				UINT numStaticSamplers = 0,
				_In_reads_opt_(numStaticSamplers) const D3D12_STATIC_SAMPLER_DESC* _pStaticSamplers = nullptr,
				D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE) noexcept
			{
				desc.NumParameters = numParameters;
				desc.pParameters = _pParameters;
				desc.NumStaticSamplers = numStaticSamplers;
				desc.pStaticSamplers = _pStaticSamplers;
				desc.Flags = flags;
			}
		};

        //------------------------------------------------------------------------------------------------
		// D3D12 exports a new method for serializing root signatures in the Windows 10 Anniversary Update.
		// To help enable root signature 1.1 features when they are available and not require maintaining
		// two code paths for building root signatures, this helper method reconstructs a 1.0 signature when
		// 1.1 is not supported.
        inline HRESULT D3DX12SerializeVersionedRootSignature(
            _In_ const D3D12_VERSIONED_ROOT_SIGNATURE_DESC* pRootSignatureDesc,
            D3D_ROOT_SIGNATURE_VERSION MaxVersion,
            _Outptr_ ID3DBlob** ppBlob,
            _Always_(_Outptr_opt_result_maybenull_) ID3DBlob** ppErrorBlob) noexcept
        {
            if (ppErrorBlob != nullptr)
            {
                *ppErrorBlob = nullptr;
            }

            switch (MaxVersion)
            {
            case D3D_ROOT_SIGNATURE_VERSION_1_0:
                switch (pRootSignatureDesc->Version)
                {
                case D3D_ROOT_SIGNATURE_VERSION_1_0:
                    return D3D12SerializeRootSignature(&pRootSignatureDesc->Desc_1_0, D3D_ROOT_SIGNATURE_VERSION_1, ppBlob, ppErrorBlob);

                case D3D_ROOT_SIGNATURE_VERSION_1_1:
                {
                    HRESULT hr = S_OK;
                    const D3D12_ROOT_SIGNATURE_DESC1& desc_1_1 = pRootSignatureDesc->Desc_1_1;

                    const SIZE_T ParametersSize = sizeof(D3D12_ROOT_PARAMETER) * desc_1_1.NumParameters;
                    void* pParameters = (ParametersSize > 0) ? HeapAlloc(GetProcessHeap(), 0, ParametersSize) : nullptr;
                    if (ParametersSize > 0 && pParameters == nullptr)
                    {
                        hr = E_OUTOFMEMORY;
                    }
                    auto pParameters_1_0 = static_cast<D3D12_ROOT_PARAMETER*>(pParameters);

                    if (SUCCEEDED(hr))
                    {
                        for (UINT n = 0; n < desc_1_1.NumParameters; n++)
                        {
                            __analysis_assume(ParametersSize == sizeof(D3D12_ROOT_PARAMETER) * desc_1_1.NumParameters);
                            pParameters_1_0[n].ParameterType = desc_1_1.pParameters[n].ParameterType;
                            pParameters_1_0[n].ShaderVisibility = desc_1_1.pParameters[n].ShaderVisibility;

                            switch (desc_1_1.pParameters[n].ParameterType)
                            {
                            case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
                                pParameters_1_0[n].Constants.Num32BitValues = desc_1_1.pParameters[n].Constants.Num32BitValues;
                                pParameters_1_0[n].Constants.RegisterSpace = desc_1_1.pParameters[n].Constants.RegisterSpace;
                                pParameters_1_0[n].Constants.ShaderRegister = desc_1_1.pParameters[n].Constants.ShaderRegister;
                                break;

                            case D3D12_ROOT_PARAMETER_TYPE_CBV:
                            case D3D12_ROOT_PARAMETER_TYPE_SRV:
                            case D3D12_ROOT_PARAMETER_TYPE_UAV:
                                pParameters_1_0[n].Descriptor.RegisterSpace = desc_1_1.pParameters[n].Descriptor.RegisterSpace;
                                pParameters_1_0[n].Descriptor.ShaderRegister = desc_1_1.pParameters[n].Descriptor.ShaderRegister;
                                break;

                            case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
                                const D3D12_ROOT_DESCRIPTOR_TABLE1& table_1_1 = desc_1_1.pParameters[n].DescriptorTable;

                                const SIZE_T DescriptorRangesSize = sizeof(D3D12_DESCRIPTOR_RANGE) * table_1_1.NumDescriptorRanges;
                                void* pDescriptorRanges = (DescriptorRangesSize > 0 && SUCCEEDED(hr)) ? HeapAlloc(GetProcessHeap(), 0, DescriptorRangesSize) : nullptr;
                                if (DescriptorRangesSize > 0 && pDescriptorRanges == nullptr)
                                {
                                    hr = E_OUTOFMEMORY;
                                }
                                auto pDescriptorRanges_1_0 = static_cast<D3D12_DESCRIPTOR_RANGE*>(pDescriptorRanges);

                                if (SUCCEEDED(hr))
                                {
                                    for (UINT x = 0; x < table_1_1.NumDescriptorRanges; x++)
                                    {
                                        __analysis_assume(DescriptorRangesSize == sizeof(D3D12_DESCRIPTOR_RANGE) * table_1_1.NumDescriptorRanges);
                                        pDescriptorRanges_1_0[x].BaseShaderRegister = table_1_1.pDescriptorRanges[x].BaseShaderRegister;
                                        pDescriptorRanges_1_0[x].NumDescriptors = table_1_1.pDescriptorRanges[x].NumDescriptors;
                                        pDescriptorRanges_1_0[x].OffsetInDescriptorsFromTableStart = table_1_1.pDescriptorRanges[x].OffsetInDescriptorsFromTableStart;
                                        pDescriptorRanges_1_0[x].RangeType = table_1_1.pDescriptorRanges[x].RangeType;
                                        pDescriptorRanges_1_0[x].RegisterSpace = table_1_1.pDescriptorRanges[x].RegisterSpace;
                                    }
                                }

                                D3D12_ROOT_DESCRIPTOR_TABLE& table_1_0 = pParameters_1_0[n].DescriptorTable;
                                table_1_0.NumDescriptorRanges = table_1_1.NumDescriptorRanges;
                                table_1_0.pDescriptorRanges = pDescriptorRanges_1_0;
                            }
                        }
                    }

                    if (SUCCEEDED(hr))
                    {
                        CD3DX12_ROOT_SIGNATURE_DESC desc_1_0(desc_1_1.NumParameters, pParameters_1_0, desc_1_1.NumStaticSamplers, desc_1_1.pStaticSamplers, desc_1_1.Flags);
                        hr = D3D12SerializeRootSignature(&desc_1_0, D3D_ROOT_SIGNATURE_VERSION_1, ppBlob, ppErrorBlob);
                    }

                    if (pParameters)
                    {
                        for (UINT n = 0; n < desc_1_1.NumParameters; n++)
                        {
                            if (desc_1_1.pParameters[n].ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
                            {
                                auto pDescriptorRanges_1_0 = pParameters_1_0[n].DescriptorTable.pDescriptorRanges;
                                HeapFree(GetProcessHeap(), 0, reinterpret_cast<void*>(const_cast<D3D12_DESCRIPTOR_RANGE*>(pDescriptorRanges_1_0)));
                            }
                        }
                        HeapFree(GetProcessHeap(), 0, pParameters);
                    }
                    return hr;
                }
                }
                break;

            case D3D_ROOT_SIGNATURE_VERSION_1_1:
                return D3D12SerializeVersionedRootSignature(pRootSignatureDesc, ppBlob, ppErrorBlob);
            }

            return E_INVALIDARG;
        }
	}
}