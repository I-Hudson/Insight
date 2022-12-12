#include "Graphics/PipelineStateObject.h"

#include "Core/Profiler.h"

namespace Insight
{
	namespace Graphics
	{
		u64 PipelineStateObject::GetHash() const
		{
			u64 hash = 0;

			{
				IS_PROFILE_SCOPE("shader");
				HashCombine(hash, Shader);
			}
			{
				IS_PROFILE_SCOPE("Queue");
				HashCombine(hash, Queue);
			}
			{
				IS_PROFILE_SCOPE("Render targets");
				for (const RHI_Texture* rt : RenderTargets)
				{
					HashCombine(hash, rt);
				}
			}
			{
				IS_PROFILE_SCOPE("Depth texture");
				HashCombine(hash, DepthStencil);
			}
			{
				IS_PROFILE_SCOPE("Draw options");
				HashCombine(hash, PrimitiveTopologyType);
				HashCombine(hash, PolygonMode);
				HashCombine(hash, CullMode);
				HashCombine(hash, FrontFace);
			}
			{
				IS_PROFILE_SCOPE("Depth");
				HashCombine(hash, DepthTest);
				HashCombine(hash, DepthWrite);
				HashCombine(hash, DepthCompareOp);
				HashCombine(hash, DepthBaisEnabled);
				HashCombine(hash, DepthClampEnabled);
			}

			{
				IS_PROFILE_SCOPE("Blend");
				HashCombine(hash, BlendEnable);
				HashCombine(hash, ColourWriteMask);
				HashCombine(hash, SrcColourBlendFactor);
				HashCombine(hash, DstColourBlendFactor);
				HashCombine(hash, ColourBlendOp);
				HashCombine(hash, SrcAplhaBlendFactor);
				HashCombine(hash, DstAplhaBlendFactor);
				HashCombine(hash, AplhaBlendOp);
			}

			{
				IS_PROFILE_SCOPE("swapchain");
				HashCombine(hash, Swapchain);
			}
			return hash;
		}
	}
}