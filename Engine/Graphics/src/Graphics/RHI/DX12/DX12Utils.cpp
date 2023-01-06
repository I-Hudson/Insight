#if defined(IS_DX12_ENABLED)

#include "Graphics/RHI/DX12/DX12Utils.h"

#include "Platforms/Platform.h"

DXGI_FORMAT PixelFormatToDXFormat[static_cast<int>(PixelFormat::MAX)] =
{
    DXGI_FORMAT_UNKNOWN,                ///0

    DXGI_FORMAT_R32G32B32A32_TYPELESS,  ///1
    DXGI_FORMAT_R32G32B32A32_FLOAT,     ///2
    DXGI_FORMAT_R32G32B32A32_UINT,      ///3
    DXGI_FORMAT_R32G32B32A32_SINT,      ///4

    DXGI_FORMAT_R32G32B32_TYPELESS,     ///5
    DXGI_FORMAT_R32G32B32_FLOAT,
    DXGI_FORMAT_R32G32B32_UINT,
    DXGI_FORMAT_R32G32B32_SINT,

    DXGI_FORMAT_R16G16B16A16_TYPELESS,
    DXGI_FORMAT_R16G16B16A16_FLOAT,
    DXGI_FORMAT_R16G16B16A16_UNORM,
    DXGI_FORMAT_R16G16B16A16_UINT,
    DXGI_FORMAT_R16G16B16A16_SNORM,
    DXGI_FORMAT_R16G16B16A16_SINT,

    DXGI_FORMAT_R32G32_TYPELESS,
    DXGI_FORMAT_R32G32_FLOAT,
    DXGI_FORMAT_R32G32_UINT,
    DXGI_FORMAT_R32G32_SINT,

    DXGI_FORMAT_R32G8X24_TYPELESS, /// TODO: R32G8X24_Typeless
    DXGI_FORMAT_D32_FLOAT_S8X24_UINT,
    DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS,  /// TODO: R32_Float_X8X24_Typeless
    DXGI_FORMAT_X32_TYPELESS_G8X24_UINT, /// TODO: X32_Typeless_G8X24_UInt

    DXGI_FORMAT_R10G10B10A2_TYPELESS,
    DXGI_FORMAT_R10G10B10A2_UNORM,
    DXGI_FORMAT_R10G10B10A2_UINT,

    DXGI_FORMAT_R11G11B10_FLOAT,

    DXGI_FORMAT_R8G8B8A8_TYPELESS,
    DXGI_FORMAT_R8G8B8A8_UNORM,
    DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
    DXGI_FORMAT_R8G8B8A8_UINT,
    DXGI_FORMAT_R8G8B8A8_SNORM,
    DXGI_FORMAT_R8G8B8A8_SINT,
    
    DXGI_FORMAT_R16G16_TYPELESS,
    DXGI_FORMAT_R16G16_FLOAT,
    DXGI_FORMAT_R16G16_UNORM,
    DXGI_FORMAT_R16G16_UINT,
    DXGI_FORMAT_R16G16_SNORM,
    DXGI_FORMAT_R16G16_SINT,
   
    DXGI_FORMAT_R32_TYPELESS,
    DXGI_FORMAT_D32_FLOAT,
    DXGI_FORMAT_R32_FLOAT,
    DXGI_FORMAT_R32_UINT,
    DXGI_FORMAT_R32_SINT,
    
    DXGI_FORMAT_R24G8_TYPELESS,
    DXGI_FORMAT_D24_UNORM_S8_UINT,
    DXGI_FORMAT_R24_UNORM_X8_TYPELESS,
    DXGI_FORMAT_X24_TYPELESS_G8_UINT,
   
    DXGI_FORMAT_R8G8_TYPELESS,
    DXGI_FORMAT_R8G8_UNORM,
    DXGI_FORMAT_R8G8_UINT,
    DXGI_FORMAT_R8G8_SNORM,
    DXGI_FORMAT_R8G8_SINT,
    
    DXGI_FORMAT_R16_TYPELESS,
    DXGI_FORMAT_R16_FLOAT,
    DXGI_FORMAT_D16_UNORM,
    DXGI_FORMAT_R16_UNORM,
    DXGI_FORMAT_R16_UINT,
    DXGI_FORMAT_R16_SNORM,
    DXGI_FORMAT_R16_SINT,
   
    DXGI_FORMAT_R8_TYPELESS,
    DXGI_FORMAT_R8_UNORM,
    DXGI_FORMAT_R8_UINT,
    DXGI_FORMAT_R8_SNORM,
    DXGI_FORMAT_R8_SINT,

    DXGI_FORMAT_A8_UNORM,

    DXGI_FORMAT_R1_UNORM,

    DXGI_FORMAT_R9G9B9E5_SHAREDEXP,

    DXGI_FORMAT_R8G8_B8G8_UNORM,

    DXGI_FORMAT_G8R8_G8B8_UNORM,

    DXGI_FORMAT_BC1_TYPELESS,
    DXGI_FORMAT_BC1_UNORM,
    DXGI_FORMAT_BC1_UNORM_SRGB,

    DXGI_FORMAT_BC2_TYPELESS,
    DXGI_FORMAT_BC2_UNORM,
    DXGI_FORMAT_BC2_UNORM_SRGB,

    DXGI_FORMAT_BC3_TYPELESS,
    DXGI_FORMAT_BC3_UNORM,
    DXGI_FORMAT_BC3_UNORM_SRGB,

    DXGI_FORMAT_BC4_TYPELESS,
    DXGI_FORMAT_BC4_UNORM,
    DXGI_FORMAT_BC4_SNORM,

    DXGI_FORMAT_BC5_TYPELESS,
    DXGI_FORMAT_BC5_UNORM,
    DXGI_FORMAT_BC5_SNORM,

    DXGI_FORMAT_B5G6R5_UNORM,
    DXGI_FORMAT_B5G5R5A1_UNORM,
    DXGI_FORMAT_B8G8R8A8_UNORM,
    DXGI_FORMAT_B8G8R8X8_UNORM,

    DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM,

    DXGI_FORMAT_B8G8R8A8_TYPELESS,
    DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
    DXGI_FORMAT_B8G8R8X8_TYPELESS,
    DXGI_FORMAT_B8G8R8X8_UNORM_SRGB,
   
    DXGI_FORMAT_BC6H_TYPELESS,
    DXGI_FORMAT_BC6H_UF16,
    DXGI_FORMAT_BC6H_SF16,
    
    DXGI_FORMAT_BC7_TYPELESS,
    DXGI_FORMAT_BC7_UNORM,
    DXGI_FORMAT_BC7_UNORM_SRGB,
};

namespace Insight
{
	namespace Graphics
	{
#define COMPARE_AND_SET_BIT(value, bitToCompare, bitToSet, result) if (value & static_cast<u32>(bitToCompare)) { result |= bitToSet; }
#define COMPARE_AND_SET(value, bitToCompare, bitToSet, result) if (value & static_cast<u32>(bitToCompare)) { result = bitToSet; }

		DXGI_FORMAT PixelFormatToDX12(PixelFormat format)
		{
			return PixelFormatToDXFormat[(int)format];
		}

		D3D12_COMMAND_LIST_TYPE GPUCommandListTypeToCommandListTypeDX12(GPUCommandListType type)
		{
			switch (type)
			{
			case GPUCommandListType::Default: return D3D12_COMMAND_LIST_TYPE_DIRECT;
			case GPUCommandListType::Transient: return D3D12_COMMAND_LIST_TYPE_DIRECT;
			case GPUCommandListType::Compute: return D3D12_COMMAND_LIST_TYPE_COMPUTE;
			case GPUCommandListType::Reset: return D3D12_COMMAND_LIST_TYPE_DIRECT;
			default:
				break;
			}
            FAIL_ASSERT();
			return D3D12_COMMAND_LIST_TYPE_DIRECT;
		}

        DXGI_FORMAT IndexTypeToDX12(IndexType type)
        {
            switch (type)
            {
            case Insight::Graphics::IndexType::Uint16: return DXGI_FORMAT_R16_UINT;
            case Insight::Graphics::IndexType::Uint32: return DXGI_FORMAT_R32_UINT;
            default:
                break;
            }
            FAIL_ASSERT();
            return DXGI_FORMAT_R16_UINT;
        }

        D3D12_BARRIER_SYNC PipelineStageFlagsToDX12(PipelineStageFlags flags)
        {
            D3D12_BARRIER_SYNC result = D3D12_BARRIER_SYNC_NONE;
            COMPARE_AND_SET(flags, PipelineStageFlagBits::TopOfPipe,                        D3D12_BARRIER_SYNC_ALL, result);
            COMPARE_AND_SET(flags, PipelineStageFlagBits::DrawIndirect,                     D3D12_BARRIER_SYNC_ALL, result);
            COMPARE_AND_SET(flags, PipelineStageFlagBits::VertexInput,                      D3D12_BARRIER_SYNC_VERTEX_SHADING, result);
            COMPARE_AND_SET(flags, PipelineStageFlagBits::VertexShader,                     D3D12_BARRIER_SYNC_VERTEX_SHADING, result);
            COMPARE_AND_SET(flags, PipelineStageFlagBits::TessesllationControlShader,       D3D12_BARRIER_SYNC_VERTEX_SHADING, result);
            COMPARE_AND_SET(flags, PipelineStageFlagBits::TessesllationEvaluationShader,    D3D12_BARRIER_SYNC_VERTEX_SHADING, result);
            COMPARE_AND_SET(flags, PipelineStageFlagBits::GeometryShader,                   D3D12_BARRIER_SYNC_VERTEX_SHADING, result);
            COMPARE_AND_SET(flags, PipelineStageFlagBits::FragmentShader,                   D3D12_BARRIER_SYNC_PIXEL_SHADING, result);
            COMPARE_AND_SET(flags, PipelineStageFlagBits::EarlyFramgmentShader,             D3D12_BARRIER_SYNC_DEPTH_STENCIL, result);
            COMPARE_AND_SET(flags, PipelineStageFlagBits::LateFramgmentShader,              D3D12_BARRIER_SYNC_DEPTH_STENCIL, result);
            COMPARE_AND_SET(flags, PipelineStageFlagBits::ColourAttachmentOutput,           D3D12_BARRIER_SYNC_RENDER_TARGET, result);
            COMPARE_AND_SET(flags, PipelineStageFlagBits::ComputeShader,                    D3D12_BARRIER_SYNC_COMPUTE_SHADING, result);
            COMPARE_AND_SET(flags, PipelineStageFlagBits::Transfer,                         D3D12_BARRIER_SYNC_COPY, result);
            COMPARE_AND_SET(flags, PipelineStageFlagBits::BottomOfPipe,                     D3D12_BARRIER_SYNC_ALL, result);
            COMPARE_AND_SET(flags, PipelineStageFlagBits::Host,                             D3D12_BARRIER_SYNC_RESOLVE, result);
            COMPARE_AND_SET(flags, PipelineStageFlagBits::AllGraphics,                      D3D12_BARRIER_SYNC_ALL_SHADING, result);
            COMPARE_AND_SET(flags, PipelineStageFlagBits::AllCommands,                      D3D12_BARRIER_SYNC_ALL, result);
            return result;
        }

        D3D12_BARRIER_ACCESS AccessFlagsToDX12(AccessFlags flags)
        {
            D3D12_BARRIER_ACCESS result = D3D12_BARRIER_ACCESS_NO_ACCESS;
            COMPARE_AND_SET(flags, AccessFlagBits::IndirectCommandRead,             D3D12_BARRIER_ACCESS_INDIRECT_ARGUMENT, result);
            COMPARE_AND_SET(flags, AccessFlagBits::IndexRead,                       D3D12_BARRIER_ACCESS_INDEX_BUFFER, result);
            COMPARE_AND_SET(flags, AccessFlagBits::VertexAttributeRead,             D3D12_BARRIER_ACCESS_VERTEX_BUFFER, result);
            COMPARE_AND_SET(flags, AccessFlagBits::UniformRead,                     D3D12_BARRIER_ACCESS_CONSTANT_BUFFER, result);
            COMPARE_AND_SET(flags, AccessFlagBits::InputAttachmentRead,             D3D12_BARRIER_ACCESS_SHADER_RESOURCE, result);
            COMPARE_AND_SET(flags, AccessFlagBits::ShaderRead,                      D3D12_BARRIER_ACCESS_SHADER_RESOURCE, result);
            COMPARE_AND_SET(flags, AccessFlagBits::ShaderWrite,                     D3D12_BARRIER_ACCESS_UNORDERED_ACCESS, result);
            COMPARE_AND_SET(flags, AccessFlagBits::ColorAttachmentRead,             D3D12_BARRIER_ACCESS_RENDER_TARGET, result);
            COMPARE_AND_SET(flags, AccessFlagBits::ColorAttachmentWrite,            D3D12_BARRIER_ACCESS_RENDER_TARGET, result);
            COMPARE_AND_SET(flags, AccessFlagBits::DepthStencilAttachmentRead,      D3D12_BARRIER_ACCESS_DEPTH_STENCIL_READ, result);
            COMPARE_AND_SET(flags, AccessFlagBits::DepthStencilAttachmentWrite,     D3D12_BARRIER_ACCESS_DEPTH_STENCIL_WRITE, result);
            COMPARE_AND_SET(flags, AccessFlagBits::TransferRead,                    D3D12_BARRIER_ACCESS_COPY_SOURCE, result);
            COMPARE_AND_SET(flags, AccessFlagBits::TransferWrite,                   D3D12_BARRIER_ACCESS_COPY_DEST, result);
            COMPARE_AND_SET(flags, AccessFlagBits::HostRead,                        D3D12_BARRIER_ACCESS_COMMON, result);
            COMPARE_AND_SET(flags, AccessFlagBits::HostWrite,                       D3D12_BARRIER_ACCESS_COMMON, result);
            COMPARE_AND_SET(flags, AccessFlagBits::MemoryRead,                      D3D12_BARRIER_ACCESS_COMMON, result);
            COMPARE_AND_SET(flags, AccessFlagBits::MemoryWrite,                     D3D12_BARRIER_ACCESS_COMMON, result);
            COMPARE_AND_SET(flags, AccessFlagBits::None,                            D3D12_BARRIER_ACCESS_NO_ACCESS, result);
            return result;
        }

        D3D12_BARRIER_LAYOUT ImageLayoutToDX12(ImageLayout layout)
        {
            switch (layout)
            {
            case ImageLayout::Undefined:                        return D3D12_BARRIER_LAYOUT::D3D12_BARRIER_LAYOUT_UNDEFINED;
            case ImageLayout::General:                          return D3D12_BARRIER_LAYOUT::D3D12_BARRIER_LAYOUT_UNORDERED_ACCESS;
            case ImageLayout::ColourAttachment:                 return D3D12_BARRIER_LAYOUT::D3D12_BARRIER_LAYOUT_RENDER_TARGET;
            case ImageLayout::DepthStencilAttachment:           return D3D12_BARRIER_LAYOUT::D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE;
            case ImageLayout::DepthStencilAttachmentReadOnly:   return D3D12_BARRIER_LAYOUT::D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_READ;
            case ImageLayout::ShaderReadOnly:                   return D3D12_BARRIER_LAYOUT::D3D12_BARRIER_LAYOUT_SHADER_RESOURCE;
            case ImageLayout::TransforSrc:                      return D3D12_BARRIER_LAYOUT::D3D12_BARRIER_LAYOUT_COPY_SOURCE;
            case ImageLayout::TransforDst:                      return D3D12_BARRIER_LAYOUT::D3D12_BARRIER_LAYOUT_COPY_DEST;
            case ImageLayout::Preinitialised:                   return D3D12_BARRIER_LAYOUT::D3D12_BARRIER_LAYOUT_UNDEFINED;
            case ImageLayout::DepthReadOnlyStencilAttacment:    return D3D12_BARRIER_LAYOUT::D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_READ;
            case ImageLayout::DepthAttachmentStencilReadOnly:   return D3D12_BARRIER_LAYOUT::D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_READ;
            case ImageLayout::DepthAttachmentOnly:              return D3D12_BARRIER_LAYOUT::D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE;
            case ImageLayout::DepthReadOnly:                    return D3D12_BARRIER_LAYOUT::D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_READ;
            case ImageLayout::StencilAttacment:                 return D3D12_BARRIER_LAYOUT::D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE;
            case ImageLayout::StencilReadOnly:                  return D3D12_BARRIER_LAYOUT::D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_READ;
            case ImageLayout::PresentSrc:                       return D3D12_BARRIER_LAYOUT::D3D12_BARRIER_LAYOUT_PRESENT;
            case ImageLayout::Count:
            default:
                break;
            }
            FAIL_ASSERT();
            return D3D12_BARRIER_LAYOUT_UNDEFINED;
        }

        D3D12_RESOURCE_STATES ImageLayoutToDX12ResouceState(ImageLayout layout)
        {
            switch (layout)
            {
            case ImageLayout::Undefined:                        return D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;
            case ImageLayout::General:                          return D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;
            case ImageLayout::ColourAttachment:                 return D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET;
            case ImageLayout::DepthStencilAttachment:           return D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_WRITE;
            case ImageLayout::DepthStencilAttachmentReadOnly:   return D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_READ;
            case ImageLayout::ShaderReadOnly:                   return D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
            case ImageLayout::TransforSrc:                      return D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_SOURCE;
            case ImageLayout::TransforDst:                      return D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_DEST;
            case ImageLayout::Preinitialised:                   return D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;
            case ImageLayout::DepthReadOnlyStencilAttacment:    return D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_READ;
            case ImageLayout::DepthAttachmentStencilReadOnly:   return D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_READ;
            case ImageLayout::DepthAttachmentOnly:              return D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_WRITE;
            case ImageLayout::DepthReadOnly:                    return D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_READ;
            case ImageLayout::StencilAttacment:                 return D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_WRITE;
            case ImageLayout::StencilReadOnly:                  return D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_READ;
            case ImageLayout::PresentSrc:                       return D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;
            case ImageLayout::Count:
            default:
                break;
            }
            FAIL_ASSERT();
            return D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;
        }

        D3D12_SRV_DIMENSION TextureTypeToDX12(TextureType type)
        {
            switch (type)
            {
            case Insight::Graphics::TextureType::Unknown:    return D3D12_SRV_DIMENSION::D3D12_SRV_DIMENSION_UNKNOWN;
            case Insight::Graphics::TextureType::Tex1D:      return D3D12_SRV_DIMENSION::D3D12_SRV_DIMENSION_TEXTURE1D;
            case Insight::Graphics::TextureType::Tex2D:      return D3D12_SRV_DIMENSION::D3D12_SRV_DIMENSION_TEXTURE2D;
            case Insight::Graphics::TextureType::Tex3D:      return D3D12_SRV_DIMENSION::D3D12_SRV_DIMENSION_TEXTURE3D;
            case Insight::Graphics::TextureType::TexCube:    return D3D12_SRV_DIMENSION::D3D12_SRV_DIMENSION_TEXTURECUBE;
            case Insight::Graphics::TextureType::Tex2DArray: return D3D12_SRV_DIMENSION::D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
            case Insight::Graphics::TextureType::Tex3DArray: 
            default:
                break;
            }
            FAIL_ASSERT();
            return D3D12_SRV_DIMENSION::D3D12_SRV_DIMENSION_UNKNOWN;
        }

        D3D12_DSV_DIMENSION TextureTypeToDX12DSVDimension(TextureType type)
        {
            switch (type)
            {
            case TextureType::Unknown:    return D3D12_DSV_DIMENSION::D3D12_DSV_DIMENSION_UNKNOWN;
            case TextureType::Tex1D:      return D3D12_DSV_DIMENSION::D3D12_DSV_DIMENSION_TEXTURE1D;
            case TextureType::Tex2D:      return D3D12_DSV_DIMENSION::D3D12_DSV_DIMENSION_TEXTURE2D;
            case TextureType::Tex2DArray: return D3D12_DSV_DIMENSION::D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
            case TextureType::Tex3DArray:
            default:
                break;
            }
            FAIL_ASSERT();
            return D3D12_DSV_DIMENSION::D3D12_DSV_DIMENSION_UNKNOWN;
        }

        CD3DX12_HEAP_PROPERTIES BufferTypeToDX12HeapProperties(BufferType bufferType)
        {
            switch (bufferType)
            {
            case BufferType::Vertex:  return CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT);
            case BufferType::Index:   return CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT);
            case BufferType::Uniform: return CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_UPLOAD);
            case BufferType::Storage: return CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_UPLOAD);
            case BufferType::Raw:     return CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT);
            case BufferType::Staging: return CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_UPLOAD);
            case BufferType::Readback:return CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_READBACK);
            default:
                break;
            }
            FAIL_ASSERT();
            return CD3DX12_HEAP_PROPERTIES();
        }

        D3D12_RESOURCE_STATES BufferTypeToDX12InitialResourceState(BufferType bufferType)
        {
            switch (bufferType)
            {
            case BufferType::Vertex:  return D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
            case BufferType::Index:   return D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_INDEX_BUFFER;
            case BufferType::Uniform: return D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ;
            case BufferType::Storage: return D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ;
            case BufferType::Raw:     return D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;
            case BufferType::Staging: return D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ;
            case BufferType::Readback:return D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_DEST;
            default:
                break;
            }
            FAIL_ASSERT();
            return D3D12_RESOURCE_STATES();
        }

        D3D12_RESOURCE_STATES BufferTypeToDX12ResourceState(BufferType bufferType)
        {
            switch (bufferType)
            {
            case BufferType::Vertex:  return D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
            case BufferType::Index:   return D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_INDEX_BUFFER;
            case BufferType::Uniform: return D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ;
            case BufferType::Storage: return D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ;
            case BufferType::Raw:     return D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;
            case BufferType::Staging: return D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ;
            case BufferType::Readback:return D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_DEST;
            default:
                break;
            }
            FAIL_ASSERT();
            return D3D12_RESOURCE_STATES();
        }

        D3D12_RESOURCE_FLAGS ImageUsageFlagsToDX12(ImageUsageFlags flags)
        {
            D3D12_RESOURCE_FLAGS result = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE;
            COMPARE_AND_SET_BIT(flags, ImageUsageFlagsBits::TransferSrc,            D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE, result);
            COMPARE_AND_SET_BIT(flags, ImageUsageFlagsBits::TransferDst,            D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE, result);
            COMPARE_AND_SET_BIT(flags, ImageUsageFlagsBits::Sampled,                D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE, result);
            COMPARE_AND_SET_BIT(flags, ImageUsageFlagsBits::Storage,                D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, result);
            COMPARE_AND_SET_BIT(flags, ImageUsageFlagsBits::ColourAttachment,       D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, result);
            COMPARE_AND_SET_BIT(flags, ImageUsageFlagsBits::DepthStencilAttachment, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, result);
            COMPARE_AND_SET_BIT(flags, ImageUsageFlagsBits::TransientAttachment,    D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE, result);
            COMPARE_AND_SET_BIT(flags, ImageUsageFlagsBits::InputAttachment,        D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE, result);
            return result;
        }

        D3D12_FILTER FilterToDX12(Filter filter, CompareOp op)
        {
            switch (filter)
            {
            case Filter::Nearest:    return op == CompareOp::Never ? D3D12_FILTER::D3D12_FILTER_MIN_MAG_MIP_POINT : D3D12_FILTER::D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
            case Filter::Linear:     return op == CompareOp::Never ? D3D12_FILTER::D3D12_FILTER_MIN_MAG_MIP_LINEAR : D3D12_FILTER::D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
            case Filter::Point:      return op == CompareOp::Never ? D3D12_FILTER::D3D12_FILTER_MIN_MAG_MIP_POINT : D3D12_FILTER::D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
            default:
                break;
            }
            FAIL_ASSERT();
            return D3D12_FILTER::D3D12_FILTER_MIN_MAG_MIP_POINT;
        }

        D3D12_TEXTURE_ADDRESS_MODE SamplerAddressModelToDX12(SamplerAddressMode addressMode)
        {
            switch (addressMode)
            {
            case SamplerAddressMode::Repeat:             return D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            case SamplerAddressMode::MirroredRepeat:     return D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
            case SamplerAddressMode::ClampToEdge:        return D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            case SamplerAddressMode::ClampToBoarder:     return D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            case SamplerAddressMode::MirrorClampToEdge:  return D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
            default:
                break;
            }
            FAIL_ASSERT();
            return D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        }

        D3D12_COMPARISON_FUNC CompareOpToDX12(CompareOp op)
        {
            switch (op)
            {
            case CompareOp::Never:          return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_NEVER;
            case CompareOp::Less:           return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_LESS;
            case CompareOp::Equal:          return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_EQUAL;
            case CompareOp::LessOrEqual:    return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_LESS_EQUAL;
            case CompareOp::Greater:        return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_GREATER;
            case CompareOp::NotEqual:       return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_NOT_EQUAL;
            case CompareOp::GreaterOrEqual: return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_GREATER_EQUAL;
            case CompareOp::Always:         return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_ALWAYS;
            default:
                break;
            }
            FAIL_ASSERT();
            return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_NEVER;
        }

        std::array<FLOAT, 4> BorderColourToDX12(BorderColour boarderColour)
        {
            switch (boarderColour)
            {
            case BorderColour::FloatTransparentBlack: return { 0.0f, 0.0f, 0.0f, 0.0f };
            case BorderColour::IntTransparentBlack:   return { 0.0f, 0.0f, 0.0f, 0.0f };
            case BorderColour::FloatOpaqueBlack:      return { 0.0f, 0.0f, 0.0f, 1.0f };
            case BorderColour::IntOpaqueBlack:        return { 0.0f, 0.0f, 0.0f, 1.0f };
            case BorderColour::FloatOpaqueWhite:      return { 1.0f, 1.0f, 1.0f, 1.0f };
            case BorderColour::IntOpaqueWhite:        return { 1.0f, 1.0f, 1.0f, 1.0f };
            default:
                break;
            }
            FAIL_ASSERT();
            return { 0.0f, 0.0f, 0.0f, 0.0f };
        }

        D3D12_PRIMITIVE_TOPOLOGY_TYPE PrimitiveTopologyTypeToDX12(PrimitiveTopologyType type)
        {
            switch (type)
            {
            case PrimitiveTopologyType::PointList: return D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
            case PrimitiveTopologyType::LineList: return D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
            case PrimitiveTopologyType::LineStrip: return D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
            case PrimitiveTopologyType::TriangleList: return D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            case PrimitiveTopologyType::TriangleStrip: return D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            ///case PrimitiveTopologyType::TriangleFan: return D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ;
            case PrimitiveTopologyType::LineListWithAdjacency: return D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
            case PrimitiveTopologyType::LineStripWithAdjacency: return D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
            case PrimitiveTopologyType::TriangleListWithAdjacency: return D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            case PrimitiveTopologyType::TriangleStripWithAdjacency: return D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            case PrimitiveTopologyType::PatchList: return D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
            default:
                break;
            }
            FAIL_ASSERT();
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        }

        D3D12_PRIMITIVE_TOPOLOGY PrimitiveTopologyToDX12(PrimitiveTopologyType type)
        {
            switch (type)
            {
            case PrimitiveTopologyType::PointList:                  return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
            case PrimitiveTopologyType::LineList:                   return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
            case PrimitiveTopologyType::LineStrip:                  return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
            case PrimitiveTopologyType::TriangleList:               return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            case PrimitiveTopologyType::TriangleStrip:              return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
            case PrimitiveTopologyType::TriangleFan:                return D3D_PRIMITIVE_TOPOLOGY_TRIANGLEFAN;
            case PrimitiveTopologyType::LineListWithAdjacency:      return D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ;
            case PrimitiveTopologyType::LineStripWithAdjacency:     return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ;
            case PrimitiveTopologyType::TriangleListWithAdjacency:  return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ;
            case PrimitiveTopologyType::TriangleStripWithAdjacency: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ;
            case PrimitiveTopologyType::PatchList:                  return D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST;
            default:
                break;
            }
            FAIL_ASSERT();
            return D3D12_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        }

        D3D12_FILL_MODE PolygonModeToDX12(PolygonMode polygonMode)
        {
            switch (polygonMode)
            {
            case PolygonMode::Fill: return D3D12_FILL_MODE::D3D12_FILL_MODE_SOLID;
            case PolygonMode::Line: return D3D12_FILL_MODE::D3D12_FILL_MODE_SOLID;
            case PolygonMode::Point: {}
            }
            FAIL_ASSERT();
            return D3D12_FILL_MODE::D3D12_FILL_MODE_SOLID;
        }

        D3D12_CULL_MODE CullModeToDX12(CullMode cullMode)
        {
            switch (cullMode)
            {
            case CullMode::None: return D3D12_CULL_MODE::D3D12_CULL_MODE_NONE;
            case CullMode::Front: return D3D12_CULL_MODE::D3D12_CULL_MODE_FRONT;
            case CullMode::Back: return D3D12_CULL_MODE::D3D12_CULL_MODE_BACK;
            case CullMode::FrontAndBack: {}
            }
            FAIL_ASSERT();
            return D3D12_CULL_MODE::D3D12_CULL_MODE_NONE;
        }

        D3D12_BLEND BlendFactorToDX12(BlendFactor factor)
        {
            switch (factor)
            {
            case BlendFactor::Zero:                     return D3D12_BLEND::D3D12_BLEND_ZERO;
            case BlendFactor::One:                      return D3D12_BLEND::D3D12_BLEND_ONE;
            case BlendFactor::SrcColour:                return D3D12_BLEND::D3D12_BLEND_SRC_COLOR;
            case BlendFactor::OneMinusSrcColour:        return D3D12_BLEND::D3D12_BLEND_INV_SRC_COLOR;
            case BlendFactor::DstColour:                return D3D12_BLEND::D3D12_BLEND_DEST_COLOR;
            case BlendFactor::OneMinusDstColour:        return D3D12_BLEND::D3D12_BLEND_INV_DEST_COLOR;
            case BlendFactor::SrcAlpha:                 return D3D12_BLEND::D3D12_BLEND_SRC_ALPHA;
            case BlendFactor::OneMinusSrcAlpha:         return D3D12_BLEND::D3D12_BLEND_INV_SRC_ALPHA;
            case BlendFactor::DstAlpha:                 return D3D12_BLEND::D3D12_BLEND_DEST_ALPHA;
            case BlendFactor::OneMinusDstAlpha:         return D3D12_BLEND::D3D12_BLEND_INV_DEST_ALPHA;
            case BlendFactor::ConstantColour:           return D3D12_BLEND::D3D12_BLEND_BLEND_FACTOR;
            case BlendFactor::OneMinusConstantColour:   return D3D12_BLEND::D3D12_BLEND_BLEND_FACTOR;
            case BlendFactor::ConstantAlpha:            return D3D12_BLEND::D3D12_BLEND_BLEND_FACTOR;
            case BlendFactor::OneMinusConstantAlpha:    return D3D12_BLEND::D3D12_BLEND_BLEND_FACTOR;
            case BlendFactor::SrcAplhaSaturate:         return D3D12_BLEND::D3D12_BLEND_BLEND_FACTOR;
            case BlendFactor::Src1Colour:               return D3D12_BLEND::D3D12_BLEND_SRC1_COLOR;
            case BlendFactor::OneMinusSrc1Colour:       return D3D12_BLEND::D3D12_BLEND_INV_SRC1_COLOR;
            case BlendFactor::Src1Alpha:                return D3D12_BLEND::D3D12_BLEND_SRC1_ALPHA;
            case BlendFactor::OneMinusSrc1Alpha:        return D3D12_BLEND::D3D12_BLEND_INV_SRC1_ALPHA;
            default:
                break;
            }
            FAIL_ASSERT();
            return D3D12_BLEND::D3D12_BLEND_ZERO;
        }

        D3D12_BLEND_OP BlendOpToDX12(BlendOp op)
        {
            switch (op)
            {
            case BlendOp::Add:             return D3D12_BLEND_OP::D3D12_BLEND_OP_ADD;
            case BlendOp::Subtract:        return D3D12_BLEND_OP::D3D12_BLEND_OP_SUBTRACT;
            case BlendOp::ReverseSubtract: return D3D12_BLEND_OP::D3D12_BLEND_OP_REV_SUBTRACT;
            case BlendOp::Min:             return D3D12_BLEND_OP::D3D12_BLEND_OP_MIN;
            case BlendOp::Max:             return D3D12_BLEND_OP::D3D12_BLEND_OP_MAX;
            default:
                break;
            }
            FAIL_ASSERT();
            return D3D12_BLEND_OP::D3D12_BLEND_OP_ADD;
        }

        D3D12_DESCRIPTOR_HEAP_TYPE DescriptorTypeToDX12(DescriptorType type)
        {
            switch (type)
            {
            case DescriptorType::Sampler:                    return D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
            case DescriptorType::Sampled_Image:              return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            case DescriptorType::Storage_Image:              return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            case DescriptorType::Uniform_Texel_Buffer:       return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            case DescriptorType::Storage_Texel_Buffer:       return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            case DescriptorType::Unifom_Buffer:              return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            case DescriptorType::Storage_Buffer:             return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            case DescriptorType::Uniform_Buffer_Dynamic:     return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            case DescriptorType::Storage_Buffer_Dyanmic:     return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            case DescriptorType::Input_Attachment:           return D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            case DescriptorType::Unknown:                    return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            default:
                break;
            }
            FAIL_ASSERT();
            return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        }

        D3D12_DESCRIPTOR_RANGE_TYPE DescriptorRangeTypeToDX12(DescriptorType type)
        {
            switch (type)
            {
            case DescriptorType::Sampler:                return D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
            case DescriptorType::Sampled_Image:          return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            case DescriptorType::Storage_Image:          return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
            case DescriptorType::Uniform_Texel_Buffer:   return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
            case DescriptorType::Storage_Texel_Buffer:   return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
            case DescriptorType::Unifom_Buffer:          return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
            case DescriptorType::Storage_Buffer:         return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
            case DescriptorType::Uniform_Buffer_Dynamic: return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
            case DescriptorType::Storage_Buffer_Dyanmic: return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
            case DescriptorType::Input_Attachment:       return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            case DescriptorType::Unknown: break;
            default:
                break;
            }
            FAIL_ASSERT();
            return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
        }

        D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapTypeToDX12(RHI::DX12::DescriptorHeapTypes type)
        {
            switch (type)
            {
            case Insight::Graphics::RHI::DX12::CBV_SRV_UAV:       return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            case Insight::Graphics::RHI::DX12::Sampler:           return D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
            case Insight::Graphics::RHI::DX12::RenderTargetView:  return D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            case Insight::Graphics::RHI::DX12::DepthStencilView:  return D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
            case Insight::Graphics::RHI::DX12::NumDescriptors:
                break;
            default:
                break;
            }
            FAIL_ASSERT();
            return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        }

        DXGI_SWAP_EFFECT SwapchainPresentModeToDX12(SwapchainPresentModes presentMode)
        {
            switch (presentMode)
            {
            case Insight::Graphics::SwapchainPresentModes::Immediate: return DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
            case Insight::Graphics::SwapchainPresentModes::VSync:     return DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
            case Insight::Graphics::SwapchainPresentModes::Variable:  return DXGI_SWAP_EFFECT_FLIP_DISCARD;
            default:
                break;
            }            
            FAIL_ASSERT();
            return DXGI_SWAP_EFFECT_SEQUENTIAL;
        }
	}
}

#endif /// if defined(IS_DX12_ENABLED)