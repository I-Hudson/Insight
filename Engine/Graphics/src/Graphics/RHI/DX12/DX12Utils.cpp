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
   
    DXGI_FORMAT_D32_FLOAT,
    DXGI_FORMAT_R32_TYPELESS,
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
		DXGI_FORMAT PixelFormatToDX12(PixelFormat format)
		{
			return PixelFormatToDXFormat[(int)format];
		}

		D3D12_COMMAND_LIST_TYPE GPUCommandListTypeToCommandListTypeDX12(GPUCommandListType type)
		{
			switch (type)
			{
			case Insight::Graphics::GPUCommandListType::Default: return D3D12_COMMAND_LIST_TYPE_DIRECT;
			case Insight::Graphics::GPUCommandListType::Transient: return D3D12_COMMAND_LIST_TYPE_DIRECT;
			case Insight::Graphics::GPUCommandListType::Compute: return D3D12_COMMAND_LIST_TYPE_COMPUTE;
			case Insight::Graphics::GPUCommandListType::Reset: return D3D12_COMMAND_LIST_TYPE_DIRECT;
			default:
				break;
			}
			return D3D12_COMMAND_LIST_TYPE_DIRECT;
		}

        D3D12_FILTER FilterToDX12(Filter filter)
        {
            switch (filter)
            {
            case Insight::Graphics::Filter::Nearest:    return D3D12_FILTER::D3D12_FILTER_MIN_MAG_MIP_POINT;
            case Insight::Graphics::Filter::Linear:     return D3D12_FILTER::D3D12_FILTER_MIN_MAG_MIP_LINEAR;
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
            case Insight::Graphics::SamplerAddressMode::Repeat:             return D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            case Insight::Graphics::SamplerAddressMode::MirroredRepeat:     return D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
            case Insight::Graphics::SamplerAddressMode::ClampToEdge:        return D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            case Insight::Graphics::SamplerAddressMode::ClampToBoarder:     return D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            case Insight::Graphics::SamplerAddressMode::MirrorClampToEdge:  return D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
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
            case Insight::Graphics::CompareOp::Never:          return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_NEVER;
            case Insight::Graphics::CompareOp::Less:           return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_LESS;
            case Insight::Graphics::CompareOp::Equal:          return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_EQUAL;
            case Insight::Graphics::CompareOp::LessOrEqual:    return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_LESS_EQUAL;
            case Insight::Graphics::CompareOp::Greater:        return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_GREATER;
            case Insight::Graphics::CompareOp::NotEqual:       return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_NOT_EQUAL;
            case Insight::Graphics::CompareOp::GreaterOrEqual: return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_GREATER_EQUAL;
            case Insight::Graphics::CompareOp::Always:         return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_ALWAYS;
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
            case Insight::Graphics::BorderColour::FloatTransparentBlack: return { 0.0f, 0.0f, 0.0f, 0.0f };
            case Insight::Graphics::BorderColour::IntTransparentBlack:   return { 0.0f, 0.0f, 0.0f, 0.0f };
            case Insight::Graphics::BorderColour::FloatOpaqueBlack:      return { 0.0f, 0.0f, 0.0f, 1.0f };
            case Insight::Graphics::BorderColour::IntOpaqueBlack:        return { 0.0f, 0.0f, 0.0f, 1.0f };
            case Insight::Graphics::BorderColour::FloatOpaqueWhite:      return { 1.0f, 1.0f, 1.0f, 1.0f };
            case Insight::Graphics::BorderColour::IntOpaqueWhite:        return { 1.0f, 1.0f, 1.0f, 1.0f };
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
            case Insight::Graphics::PrimitiveTopologyType::PointList: return D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
            case Insight::Graphics::PrimitiveTopologyType::LineList: return D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
            case Insight::Graphics::PrimitiveTopologyType::LineStrip: return D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
            case Insight::Graphics::PrimitiveTopologyType::TriangleList: return D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            case Insight::Graphics::PrimitiveTopologyType::TriangleStrip: return D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            ///case Insight::Graphics::PrimitiveTopologyType::TriangleFan: return D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ;
            case Insight::Graphics::PrimitiveTopologyType::LineListWithAdjacency: return D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
            case Insight::Graphics::PrimitiveTopologyType::LineStripWithAdjacency: return D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
            case Insight::Graphics::PrimitiveTopologyType::TriangleListWithAdjacency: return D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            case Insight::Graphics::PrimitiveTopologyType::TriangleStripWithAdjacency: return D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            case Insight::Graphics::PrimitiveTopologyType::PatchList: return D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
            default:
                break;
            }
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
            return D3D12_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        }

        D3D12_FILL_MODE PolygonModeToDX12(PolygonMode polygonMode)
        {
            switch (polygonMode)
            {
            case Insight::Graphics::PolygonMode::Fill: return D3D12_FILL_MODE::D3D12_FILL_MODE_SOLID;
            case Insight::Graphics::PolygonMode::Line: return D3D12_FILL_MODE::D3D12_FILL_MODE_SOLID;
            case Insight::Graphics::PolygonMode::Point: {}
            }
            return D3D12_FILL_MODE::D3D12_FILL_MODE_SOLID;
        }

        D3D12_CULL_MODE CullModeToDX12(CullMode cullMode)
        {
            switch (cullMode)
            {
            case Insight::Graphics::CullMode::None: return D3D12_CULL_MODE::D3D12_CULL_MODE_NONE;
            case Insight::Graphics::CullMode::Front: return D3D12_CULL_MODE::D3D12_CULL_MODE_FRONT;
            case Insight::Graphics::CullMode::Back: return D3D12_CULL_MODE::D3D12_CULL_MODE_BACK;
            case Insight::Graphics::CullMode::FrontAndBack: {}
            }
            return D3D12_CULL_MODE::D3D12_CULL_MODE_NONE;
        }

        D3D12_BLEND BlendFactorToDX12(BlendFactor factor)
        {
            return D3D12_BLEND();
        }

        D3D12_BLEND_OP BlendOpToFX12(BlendOp op)
        {
            return D3D12_BLEND_OP();
        }

        D3D12_DESCRIPTOR_HEAP_TYPE DescriptorTypeToDX12(DescriptorType type)
        {
            switch (type)
            {
            case Insight::Graphics::DescriptorType::Sampler:                    return D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
            case Insight::Graphics::DescriptorType::Combined_Image_Sampler:     return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            case Insight::Graphics::DescriptorType::Sampled_Image:              return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            case Insight::Graphics::DescriptorType::Storage_Image:              return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            case Insight::Graphics::DescriptorType::Uniform_Texel_Buffer:       return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            case Insight::Graphics::DescriptorType::Storage_Texel_Buffer:       return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            case Insight::Graphics::DescriptorType::Unifom_Buffer:              return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            case Insight::Graphics::DescriptorType::Storage_Buffer:             return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            case Insight::Graphics::DescriptorType::Uniform_Buffer_Dynamic:     return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            case Insight::Graphics::DescriptorType::Storage_Buffer_Dyanmic:     return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            case Insight::Graphics::DescriptorType::Input_Attachment:           return D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            case Insight::Graphics::DescriptorType::Unknown:                    return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            default:
                break;
            }
            return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        }

        D3D12_DESCRIPTOR_RANGE_TYPE DescriptorRangeTypeToDX12(DescriptorType type)
        {
            switch (type)
            {
            case Insight::Graphics::DescriptorType::Sampler:                return D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
            case Insight::Graphics::DescriptorType::Combined_Image_Sampler: return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            case Insight::Graphics::DescriptorType::Sampled_Image:          return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            case Insight::Graphics::DescriptorType::Storage_Image:          return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            case Insight::Graphics::DescriptorType::Uniform_Texel_Buffer:   return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
            case Insight::Graphics::DescriptorType::Storage_Texel_Buffer:   return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
            case Insight::Graphics::DescriptorType::Unifom_Buffer:          return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
            case Insight::Graphics::DescriptorType::Storage_Buffer:         return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
            case Insight::Graphics::DescriptorType::Uniform_Buffer_Dynamic: return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
            case Insight::Graphics::DescriptorType::Storage_Buffer_Dyanmic: return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
            case Insight::Graphics::DescriptorType::Input_Attachment:       return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            case Insight::Graphics::DescriptorType::Unknown: break;
            default:
                break;
            }
            return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
        }
	}
}

#endif /// if defined(IS_DX12_ENABLED)