#include "Graphics/RHI/DX12/DX12Utils.h"

namespace Insight
{
	namespace Graphics
	{
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
	}
}